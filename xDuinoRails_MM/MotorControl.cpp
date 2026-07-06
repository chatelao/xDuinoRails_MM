#include "MotorControl.h"
#include "Logger.h"

const int PWM_RANGE = 1023;

MotorControl::MotorControl(CvManager &cvManager, int pinA, int pinB, int bemfA,
                           int bemfB)
    : cvManager(cvManager) {
  pinA_priv           = pinA;
  pinB_priv           = pinB;
  bemfA_priv          = bemfA;
  bemfB_priv          = bemfB;
  targetPwm           = 0;
  currDirection       = MM2DirectionState_Forward;
  targetDirection     = MM2DirectionState_Forward;
  isKickstarting_priv = false;
  kickstartBegin      = 0;
  lastBemfMeasure     = 0;
  lastSpeed           = 0;
  previousPwm         = 0;
  bemfErrorSum        = 0;
  lastAdjustment      = 0;
}

void MotorControl::setup() {
  pinMode(pinA_priv, OUTPUT);
  pinMode(pinB_priv, OUTPUT);
  pinMode(bemfA_priv, INPUT);
  pinMode(bemfB_priv, INPUT);

  int         motorType = cvManager.getCv(CV_MOTOR_TYPE);
  const char *typeName  = "Standard DC";

  switch (motorType) {
  case 1: // Faulhaber
    PWM_FREQ        = 400;
    KICK_PWM        = 1023;
    KICK_MAX_TIME   = 150;
    BEMF_THRESHOLD  = 120;
    BEMF_SAMPLE_INT = 15;
    typeName        = "Faulhaber";
    break;
  case 2: // Maxon
    PWM_FREQ        = 20000;
    KICK_PWM        = 600;
    KICK_MAX_TIME   = 80;
    BEMF_THRESHOLD  = 80;
    BEMF_SAMPLE_INT = 10;
    typeName        = "Maxon";
    break;
  default: // Standard DC
    PWM_FREQ        = 20000;
    KICK_PWM        = 800;
    KICK_MAX_TIME   = 100;
    BEMF_THRESHOLD  = 100;
    BEMF_SAMPLE_INT = 12;
    typeName        = "Standard DC";
    break;
  }

  logger.printf("Motor: Type=%s, PWM Freq=%d Hz\n", typeName, PWM_FREQ);

#ifdef ARDUINO_ARCH_RP2040
  analogWriteFreq(PWM_FREQ);
  analogWriteRange(PWM_RANGE);
#elif defined(ARDUINO_ARCH_ESP32)
  analogWriteFrequency(pinA_priv, PWM_FREQ);
  analogWriteFrequency(pinB_priv, PWM_FREQ);
  analogWriteResolution(pinA_priv, 10); // 10 bits = 1023
  analogWriteResolution(pinB_priv, 10); // 10 bits = 1023
#endif

  writeMotorHardware(0, MM2DirectionState_Forward);
}

const int PWM_MAX = 1023;

void MotorControl::setSpeed(int step, MM2DirectionState dir) {
  
  //
  // Special case: Handle stopping
  //
  if (step == 0) {
    if (lastSpeed != 0 || dir != targetDirection) {
      logger.printf("Motor: Step 0 -> PWM 0, Dir %s\n",
                    dir == MM2DirectionState_Forward ? "Forward" : "Backward");
      lastSpeed = 0;
    }
    update(0, dir);
    return;
  }

  //
  // Regular case: Handling movements
  //
  // Map CVs (0-255) to PWM (0-1023)
  int vStart = map(cvManager.getCv(CV_START_VOLTAGE), 0, 255, 0, PWM_MAX);
  int vMid   = map(cvManager.getCv(CV_MEDIUM_SPEED ), 0, 255, 0, PWM_MAX);
  int vHigh  = map(cvManager.getCv(CV_MAXIMUM_SPEED), 0, 255, 0, PWM_MAX);

  // If CVs are 0, then set defaults (1 / PWM_MAX)
  if (vStart == 0) vStart = 1;
  if (vMid   == 0) vMid = (vStart + vHigh) / 2;
  if (vHigh  == 0) vHigh = PWM_MAX;

  // Clamp vStart to vHigh to avoid negative slopes
  if (vStart > vHigh)
    vStart = vHigh;

  // Clamp vMid between vStart and vHigh
  if (vStart > vMid  ) vMid = vStart; // Clamp to >= vStart
  if (vMid   > vHigh ) vMid = vHigh;  // Clamp to <= vHigh
  
  int pwm;
  if (step <= 7) {
    pwm = map(step, 1, 7, vStart, vMid);
  } else {
    pwm = map(step, 7, 14, vMid, vHigh);
  }

  if (step != lastSpeed || dir != targetDirection) {
    logger.printf("Motor: Step %d -> PWM %d, Dir %s\n", step, pwm,
                  dir == MM2DirectionState_Forward ? "Forward" : "Backward");
    lastSpeed = step;
  }

  update(pwm, dir);
}

void MotorControl::update(int pwm, MM2DirectionState dir) {
  targetPwm         = pwm;
  targetDirection   = dir;
  unsigned long now = millis();

  // Bei Stillstand Richtung sofort übernehmen
  if (targetPwm == 0) {
    currDirection = targetDirection;
    bemfErrorSum   = 0;
    lastAdjustment = 0;
  }

  if (previousPwm == 0 && targetPwm > 0 && KICK_MAX_TIME > 0) {
    isKickstarting_priv = true;
    logger.println("Motor: Kickstart started");
    kickstartBegin  = now;
    lastBemfMeasure = 0;
  }
  if (targetPwm == 0) {
    isKickstarting_priv = false;
  }

  if (isKickstarting_priv) {
    if (now - kickstartBegin >= KICK_MAX_TIME) {
      isKickstarting_priv = false;
      logger.println("Motor: Kickstart ended (timeout)");
    } else {
      bool bemfEnabled = (cvManager.getCv(CV_BEMF_CONFIG) & 0x01);
      if (bemfEnabled && (now - lastBemfMeasure > BEMF_SAMPLE_INT)) {
        int currentBEMF = readBEMF();
        lastBemfMeasure = now;
        if (currentBEMF > BEMF_THRESHOLD) {
          isKickstarting_priv = false;
          logger.println("Motor: Kickstart ended (BEMF)");
        }
      }
      if (isKickstarting_priv) {
        writeMotorHardware(KICK_PWM, currDirection);
      }
    }
  }

  if (!isKickstarting_priv) {
    if (currDirection != targetDirection) {
      writeMotorHardware(0, currDirection);
      currDirection  = targetDirection;
      bemfErrorSum   = 0;
      lastAdjustment = 0;
    } else {
      int finalPwm = targetPwm;

      bool bemfEnabled = (cvManager.getCv(CV_BEMF_CONFIG) & 0x01);
      if (bemfEnabled && targetPwm > 0) {
        if (now - lastBemfMeasure > BEMF_SAMPLE_INT) {
          int currentBEMF = readBEMF();
          lastBemfMeasure = now;

          // PI-Regler
          // Ziel-BEMF: Wir nehmen an, dass BEMF proportional zu PWM ist.
          // PWM 0-1023 -> BEMF 0-4095 (12-bit ADC).
          // Ein Ziel-BEMF von targetPwm * 4 ist eine grobe Annäherung.
          int targetBEMF = targetPwm * 4;
          int error      = targetBEMF - currentBEMF;

          uint8_t K = cvManager.getCv(CV_BEMF_K);
          uint8_t I = cvManager.getCv(CV_BEMF_I);

          bemfErrorSum += error;
          // Begrenzung des Integrals, um Windup zu verhindern
          if (bemfErrorSum > 10000)
            bemfErrorSum = 10000;
          if (bemfErrorSum < -10000)
            bemfErrorSum = -10000;

          lastAdjustment = (error * K / 16) + (bemfErrorSum * I / 64);
        }
        finalPwm += lastAdjustment;
      }

      writeMotorHardware(finalPwm, currDirection);
    }
  }
  previousPwm = targetPwm;
}

void MotorControl::stop() {
  targetPwm = 0;
  writeMotorHardware(0, currDirection);
}

bool MotorControl::isKickstarting() { return isKickstarting_priv; }

MM2DirectionState MotorControl::getCurrentDirection() { return currDirection; }

void MotorControl::writeMotorHardware(int pwm, MM2DirectionState dir) {
  if (pwm > PWM_RANGE)
    pwm = PWM_RANGE;
  if (pwm < 0)
    pwm = 0;

  if (dir == MM2DirectionState_Forward) {
    digitalWrite(pinB_priv, LOW);
    analogWrite(pinA_priv, pwm);
  } else {
    digitalWrite(pinA_priv, LOW);
    analogWrite(pinB_priv, pwm);
  }
}

int MotorControl::readBEMF() {
  digitalWrite(pinA_priv, LOW);
  digitalWrite(pinB_priv, LOW);
  delayMicroseconds(500);
  int valA = analogRead(bemfA_priv);
  int valB = analogRead(bemfB_priv);
  return (valA > valB) ? (valA - valB) : (valB - valA);
}
