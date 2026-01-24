#include <Arduino.h>
#include <MaerklinMotorola.h>
#include <Adafruit_NeoPixel.h>

// ==========================================
// 1. KONFIGURATION
// ==========================================
#define MOTOR_TYPE 1  // 1=HLA (Gross), 2=Glockenanker (Klein)

// ==========================================
// 2. PIN DEFINITIONEN (Seeed XIAO RP2040)
// ==========================================
#define MOTOR_PIN_A     D7  
#define MOTOR_PIN_B     D8  
#define BEMF_PIN_A      A0  
#define BEMF_PIN_B      A1  
#define DCC_MM_SIGNAL   D2  
#define LED_F0b         D9  
#define LED_F0f         D10 
#define PIN_INT_RED     17
#define PIN_INT_GREEN   16
#define PIN_INT_BLUE    25
#define NEO_PIN         12 
#define NEO_PWR_PIN     11 
#define NUMPIXELS       1

// ==========================================
// 3. LOGIK & PARAMETER
// ==========================================

Adafruit_NeoPixel pixels(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);
MaerklinMotorola mm(DCC_MM_SIGNAL);

const int PWM_RANGE = 1023;
const int PWM_MAX   = 1023; 

#if MOTOR_TYPE == 1
  // HLA
  const int PWM_FREQ        = 400;    
  const int PWM_MIN_MOVING  = 350;    
  const int KICK_PWM        = 1023;   
  const int KICK_MAX_TIME   = 150;    
  const int BEMF_THRESHOLD  = 120;
  const int BEMF_SAMPLE_INT = 15;     // NEU: Nur alle 15ms messen
#else
  // Glockenanker
  const int PWM_FREQ        = 20000;  
  const int PWM_MIN_MOVING  = 80;     
  const int KICK_PWM        = 600;    
  const int KICK_MAX_TIME   = 80;     
  const int BEMF_THRESHOLD  = 80;    
  const int BEMF_SAMPLE_INT = 10;     // NEU: Nur alle 10ms messen
#endif

const int MM_ADDRESS       = 24;
const int MM_TIMEOUT_MS    = 1500;
const int MM2_LOCK_TIME    = 5000; 

unsigned long lastCommandTime = 0;
unsigned long lastMM2Seen     = 0;
unsigned long kickstartBegin  = 0;
unsigned long lastVisUpdate   = 0;
unsigned long lastBemfMeasure = 0; // NEU: Timer für Messung

int targetPwm  = 0;
int lastSpeed  = 0; 
bool isKickstarting = false;

MM2DirectionState currDirection = MM2DirectionState_Forward;
MM2DirectionState targetDirection = MM2DirectionState_Forward;

bool lastChangeDirInput = false; 
unsigned long lastChangeDirTs = 0;

bool stateF1 = false;
bool stateF2 = false;

// ==========================================
// 4. HELPER FUNKTIONEN
// ==========================================

void setIntLed(int pin, bool on) {
  digitalWrite(pin, on ? LOW : HIGH);
}

void isr() { mm.PinChange(); }

void writeMotorHardware(int pwm, MM2DirectionState dir) {
  if (pwm > PWM_RANGE) pwm = PWM_RANGE;
  if (pwm < 0) pwm = 0;
  
  // Safety: Motortreiber Logik
  if (dir == MM2DirectionState_Forward) {
    digitalWrite(MOTOR_PIN_B, LOW); 
    analogWrite(MOTOR_PIN_A, pwm);
  } else {
    digitalWrite(MOTOR_PIN_A, LOW); 
    analogWrite(MOTOR_PIN_B, pwm);
  }
}

int readBEMF() {
  // 1. Motor kurzzeitig freischalten (Coast)
  digitalWrite(MOTOR_PIN_A, LOW);
  digitalWrite(MOTOR_PIN_B, LOW);
  
  // 2. Warten, bis Induktionsspitzen abgeklungen sind
  delayMicroseconds(500); 
  
  // 3. Spannung messen (Generator-Effekt)
  int valA = analogRead(BEMF_PIN_A);
  int valB = analogRead(BEMF_PIN_B);
  
  return abs(valA - valB);
}

int getLinSpeed(int step) {
  if (step == 0) return 0;
  if (step >= 14) return PWM_MAX;
  return map(step, 1, 14, PWM_MIN_MOVING, PWM_MAX);
}

void updateVisualDebug(int speedStep, bool mm2Locked, bool kickstart) {
  if (millis() - lastVisUpdate < 50) return;
  lastVisUpdate = millis();

  setIntLed(PIN_INT_RED, mm2Locked); 
  setIntLed(PIN_INT_BLUE, stateF1);  

  if (kickstart) {
    pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // Weiss bei Kick
  } 
  else if (targetPwm == 0) {
    int val = (millis() / 20) % 255;
    int breath = (val > 127) ? 255 - val : val;
    pixels.setPixelColor(0, pixels.Color(0, 0, breath * 2)); 
  } 
  else {
    int r = map(speedStep, 0, 14, 0, 255);
    int g = map(speedStep, 0, 14, 255, 0);
    pixels.setPixelColor(0, pixels.Color(r, g, 0));
  }
  pixels.show();
}

// ==========================================
// 5. SETUP
// ==========================================
void setup() {
  pinMode(NEO_PWR_PIN, OUTPUT);
  digitalWrite(NEO_PWR_PIN, HIGH); 
  delay(10); 
  pixels.begin();
  pixels.setBrightness(40); 

  pinMode(PIN_INT_RED, OUTPUT);   digitalWrite(PIN_INT_RED, HIGH);
  pinMode(PIN_INT_GREEN, OUTPUT); digitalWrite(PIN_INT_GREEN, HIGH);
  pinMode(PIN_INT_BLUE, OUTPUT);  digitalWrite(PIN_INT_BLUE, HIGH);

  pinMode(BEMF_PIN_A, INPUT);
  pinMode(BEMF_PIN_B, INPUT);
  analogReadResolution(12); // Wichtig für RP2040 (0-4095)

  pinMode(LED_F0f, OUTPUT); pinMode(LED_F0b, OUTPUT);
  pinMode(MOTOR_PIN_A, OUTPUT); pinMode(MOTOR_PIN_B, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(DCC_MM_SIGNAL), isr, CHANGE);
  
  // RP2040 spezifisch (Earle Philhower Core)
  analogWriteFreq(PWM_FREQ);
  analogWriteRange(PWM_RANGE);
  
  writeMotorHardware(0, MM2DirectionState_Forward);
  lastCommandTime = millis(); 
}

// ==========================================
// 6. MAIN LOOP
// ==========================================
void loop() {
  mm.Parse();
  MaerklinMotorolaData* Data = mm.GetData();
  unsigned long now = millis();

  // --- A. Datenverarbeitung ---
  if (Data && !Data->IsMagnet && Data->Address == MM_ADDRESS) {
    lastCommandTime = now;
    setIntLed(PIN_INT_GREEN, true);

    bool mm2Locked = (now - lastMM2Seen < MM2_LOCK_TIME);
    if (Data->IsMM2) lastMM2Seen = now;

    // Richtung
    if (mm2Locked && Data->IsMM2) {
      if (Data->MM2Direction != MM2DirectionState_Unavailable) {
          targetDirection = Data->MM2Direction;
      }
      if (Data->MM2FunctionIndex == 1) stateF1 = Data->IsMM2FunctionOn;
      if (Data->MM2FunctionIndex == 2) stateF2 = Data->IsMM2FunctionOn;
    } 
    else if (!mm2Locked) {
      // MM1 Toggle Logik
      if (Data->ChangeDir && !lastChangeDirInput) {
        if (now - lastChangeDirTs > 250) { 
          targetDirection = (targetDirection == MM2DirectionState_Forward) 
                            ? MM2DirectionState_Backward : MM2DirectionState_Forward;
          lastChangeDirTs = now;
        }
      }
    }
    lastChangeDirInput = Data->ChangeDir; 

    // Bei Stillstand Richtung sofort übernehmen, sonst wartet Logik auf 0
    if (targetPwm == 0) currDirection = targetDirection;

    // Geschwindigkeit
    int displaySpeed = 0; 
    if (Data->IsMM2 && Data->MM2FunctionIndex != 0) {
       displaySpeed = lastSpeed; 
    } else {
       targetPwm = getLinSpeed(Data->Speed);
       displaySpeed = Data->Speed;
       lastSpeed = Data->Speed; 
    }

    updateVisualDebug(displaySpeed, mm2Locked, isKickstarting);

    // Licht
    if (Data->Function) {
      if (currDirection == MM2DirectionState_Forward) {
        digitalWrite(LED_F0f, HIGH); digitalWrite(LED_F0b, LOW);
      } else {
        digitalWrite(LED_F0f, LOW); digitalWrite(LED_F0b, HIGH);
      }
    } else {
      digitalWrite(LED_F0f, LOW); digitalWrite(LED_F0b, LOW);
    }
    
    setIntLed(PIN_INT_GREEN, false);
  } 

  // --- B. Physik (Kickstart + Direct Drive) ---

  // Kickstart Initialisierung
  static int previousPwm = 0;
  
  // Wenn wir von 0 auf Speed gehen -> Kickstart Trigger
  if (previousPwm == 0 && targetPwm > 0 && KICK_MAX_TIME > 0) {
      isKickstarting = true;
      kickstartBegin = now;
      lastBemfMeasure = 0; // Sofort messen erzwingen beim nächsten Loop
  }
  // Wenn Soll auf 0 geht -> Kickstart sofort abbrechen
  if (targetPwm == 0) {
      isKickstarting = false;
  }
  
  if (isKickstarting) {
    // Timeout prüfen
    if (now - kickstartBegin >= KICK_MAX_TIME) {
      isKickstarting = false; // Zeit abgelaufen
    } 
    else {
      // BEMF Messung nur alle X ms (Sampling Rate), damit Motor Zeit hat zu drehen!
      if (now - lastBemfMeasure > BEMF_SAMPLE_INT) {
         int currentBEMF = readBEMF();
         lastBemfMeasure = now;
         
         // Wenn Motor dreht (EMK > Threshold), Kickstart beenden
         if (currentBEMF > BEMF_THRESHOLD) {
            isKickstarting = false;
         }
      }
      
      // Solange Kickstart aktiv ist -> Volle Power
      if (isKickstarting) {
        writeMotorHardware(KICK_PWM, currDirection);
      }
    }
  } 
  
  // Wenn nicht (mehr) im Kickstart -> Normaler PWM Wert
  if (!isKickstarting) {
    // Richtungswechsel-Schutz (einfach)
    if (currDirection != targetDirection) {
        writeMotorHardware(0, currDirection); // Bremsen vor Umschalten
        // Optional: Kurzes Delay hier würde Loop blockieren, besser wäre State Machine
        currDirection = targetDirection; 
    } else {
        writeMotorHardware(targetPwm, currDirection);
    }
  }
  
  previousPwm = targetPwm; 

  // --- C. Failsafe ---
  if (now - lastCommandTime > MM_TIMEOUT_MS) {
    targetPwm = 0;
    writeMotorHardware(0, currDirection); 
    
    if ((now / 250) % 2) pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    else pixels.setPixelColor(0, 0);
    pixels.show();
    setIntLed(PIN_INT_RED, false);
  }
}
