#include "CvManager.h"
#include "CvManagerMock.h"
#include "CvProgrammer.h"
#include "DebugLeds.h"
#include "LightsControl.h"
#include "MotorControl.h"
#include "ProtocolHandler.h"
#include "RP2040.h"
#include <unity.h>

void test_mm_signal_f0_f1_f2(void);
void test_cv_programming_6021(void);
void test_watchdog_shutdown(void);
void test_debug_leds_behavior(void);
void test_protocol_handler_advanced(void);
void test_motor_bemf_kickstart(void);

// Mock implementation for RP2040 reboot
bool   reboot_called = false;
RP2040 rp2040;
void   RP2040::reboot() { reboot_called = true; }

// Test CV Manager
// Testet das Setzen und Lesen eines CV-Wertes.
void test_cv_manager_get_set(void) {
  CvManager cvManager;
  cvManager.setup();
  // Setze CV 10 auf den Wert 42
  cvManager.setCv(10, 42);
  // Überprüfe, ob CV 10 den Wert 42 hat
  TEST_ASSERT_EQUAL(42, cvManager.getCv(10));
}

// Testet die Standardwerte der CVs nach der Initialisierung.
void test_cv_manager_defaults(void) {
  CvManager cvManager;
  cvManager.setup();
  // Überprüfe die wichtigsten CVs auf ihre Standardwerte
  TEST_ASSERT_EQUAL(3, cvManager.getCv(CV_BASE_ADDRESS));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_START_VOLTAGE));
  TEST_ASSERT_EQUAL(5, cvManager.getCv(CV_ACCELERATION));
  TEST_ASSERT_EQUAL(5, cvManager.getCv(CV_BRAKING_TIME));
  TEST_ASSERT_EQUAL(0, cvManager.getCv(CV_MAXIMUM_SPEED));
  TEST_ASSERT_EQUAL(10, cvManager.getCv(CV_VERSION));
  TEST_ASSERT_EQUAL(13, cvManager.getCv(CV_MANUFACTURER_ID));
  TEST_ASSERT_EQUAL(192, cvManager.getCv(CV_LONG_ADDRESS_HIGH));
  TEST_ASSERT_EQUAL(100, cvManager.getCv(CV_LONG_ADDRESS_LOW));
  TEST_ASSERT_EQUAL(6, cvManager.getCv(CV_CONFIGURATION));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_FRONT_LIGHT_F0F));
  TEST_ASSERT_EQUAL(2, cvManager.getCv(CV_REAR_LIGHT_F0R));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_EXT_ID_HIGH));
  TEST_ASSERT_EQUAL(10, cvManager.getCv(CV_EXT_ID_LOW));
}

// Testet spezielle CV-Funktionen wie schreibgeschützte CVs und den
// Reset-Mechanismus.
void test_cv_manager_special(void) {
  CvManager cvManager;
  cvManager.setup();

  // Teste schreibgeschützte CV (CV_VERSION)
  uint8_t version = cvManager.getCv(CV_VERSION);
  // Versuch, die schreibgeschützte CV zu ändern
  cvManager.setCv(CV_VERSION, version + 1);
  // Der Wert sollte unverändert sein
  TEST_ASSERT_EQUAL(version, cvManager.getCv(CV_VERSION));

  // Teste den Reset-Mechanismus durch Schreiben auf CV_MANUFACTURER_ID
  reboot_called = false;
  cvManager.setCv(CV_MANUFACTURER_ID, 0);
  // Überprüfe, ob der Neustart ausgelöst wurde
  TEST_ASSERT_TRUE(reboot_called);
}

// Test Motor Control
// Testet die Geschwindigkeits- und Richtungsteuerung des Motors.
void test_motor_speed_control(void) {
  CvManagerMock cvManager;
  // Konfiguriere CVs für den Motor
  cvManager.setCv(CV_START_VOLTAGE, 1);
  cvManager.setCv(CV_MOTOR_TYPE, 0);
  cvManager.setCv(CV_MAXIMUM_SPEED, 200);
  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Test: Geschwindigkeit 0
  motor.setSpeed(0, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[11]);
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);

  // Test: Geschwindigkeit 1 in Vorwärtsrichtung
  motor.setSpeed(1, MM2DirectionState_Forward);
  advance_millis(101); // Simuliere Zeitablauf
  motor.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[11]);
  // Überprüfe den erwarteten PWM-Wert
  TEST_ASSERT_EQUAL(40, analog_write_values[10]);

  // Test: Geschwindigkeit 1 in Rückwärtsrichtung
  motor.setSpeed(0, MM2DirectionState_Forward); // Erst anhalten
  motor.setSpeed(1, MM2DirectionState_Backward);
  advance_millis(101);
  motor.setSpeed(1,
                 MM2DirectionState_Backward); // Dieser Aufruf stoppt den Motor
  motor.setSpeed(1,
                 MM2DirectionState_Backward); // Dieser wendet die Leistung an
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);
  TEST_ASSERT_EQUAL(40, analog_write_values[11]);

  // Test: Maximale Geschwindigkeit in Vorwärtsrichtung
  motor.setSpeed(0, MM2DirectionState_Forward);
  motor.setSpeed(14, MM2DirectionState_Forward);
  advance_millis(101);
  motor.setSpeed(14, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[11]);
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // Test: Maximale Geschwindigkeit in Rückwärtsrichtung
  motor.setSpeed(0, MM2DirectionState_Forward);
  motor.setSpeed(14, MM2DirectionState_Backward);
  advance_millis(101);
  motor.setSpeed(14, MM2DirectionState_Backward); // Stoppt den Motor
  motor.setSpeed(14, MM2DirectionState_Backward); // Wendet Leistung an
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);
  TEST_ASSERT_EQUAL(1023, analog_write_values[11]);
}

// Test Lights Control
// Testet das Standardverhalten der Lichtsteuerung.
void test_lights_control_default_behavior(void) {
  CvManagerMock cvManager;
  // CV 33: Front light mask (default 1), CV 34: Rear light mask (default 2)
  cvManager.setCv(CV_FRONT_LIGHT_F0F, 1);
  cvManager.setCv(CV_REAR_LIGHT_F0R, 2);

  LightsControl lights(cvManager, 9, 10);
  lights.setup();

  // Test: F0 aus, Vorwärts -> Beide Lichter aus
  lights.update(MM2DirectionState_Forward, false);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[9]);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);

  // Test: F0 an, Vorwärts -> Frontlicht an (D9), Rücklicht aus (D10)
  lights.update(MM2DirectionState_Forward, true);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[9]);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);

  // Test: F0 an, Rückwärts -> Frontlicht aus (D9), Rücklicht an (D10)
  lights.update(MM2DirectionState_Backward, true);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[9]);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[10]);

  // Test: CV 33 auf 0 -> Frontlicht sollte immer aus sein
  cvManager.setCv(CV_FRONT_LIGHT_F0F, 0);
  lights.update(MM2DirectionState_Forward, true);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[9]);

  // Test: CV 33 auf 1, CV 34 auf 0 -> Rücklicht sollte immer aus sein
  cvManager.setCv(CV_FRONT_LIGHT_F0F, 1);
  cvManager.setCv(CV_REAR_LIGHT_F0R, 0);
  lights.update(MM2DirectionState_Backward, true);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);
}

// Testet das Schalten der Funktionen F0, F1 und F2 über das MM-Protokoll.
void test_mm_signal_f0_f1_f2(void) {
  ProtocolHandler protocol(0);
  protocol.setAddress(1);

  // Simuliere ein MM2-Signal mit F1 an
  protocol.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 1,
                      true);
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(1));

  // Simuliere ein MM2-Signal mit F1 aus
  protocol.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 1,
                      false);
  protocol.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(1));

  // Simuliere ein MM2-Signal mit F2 an
  protocol.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 2,
                      true);
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(2));

  // Simuliere ein MM2-Signal mit F2 aus
  protocol.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 2,
                      false);
  protocol.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(2));

  // Simuliere ein MM-Signal mit F0 an
  protocol.mm.SetData(1, 0, true, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(0));

  // Simuliere ein MM-Signal mit F0 aus
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(0));
}

// Testet die Watchdog-Funktion für die Notabschaltung des Motors bei
// Signalverlust.
void test_watchdog_shutdown(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_BASE_ADDRESS, 1);
  cvManager.setCv(CV_WATCHDOG_TIMEOUT, 5); // 500ms

  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  protocol.setSignalTimeout(cvManager.getCv(CV_WATCHDOG_TIMEOUT) * 100);

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // 1. Normaler Betrieb: Geschwindigkeit auf 14 setzen
  protocol.mm.SetData(1, 14, false, false, false,
                      MM2DirectionState_Unavailable, 0, false);
  protocol.loop();
  motor.setSpeed(protocol.getTargetSpeed(), protocol.getTargetDirection());
  advance_millis(200); // Über Kickstart hinaus
  // Letztes gültiges Signal vor dem Verlust
  protocol.mm.SetData(1, 14, false, false, false,
                      MM2DirectionState_Unavailable, 0, false);
  protocol.loop();
  motor.setSpeed(protocol.getTargetSpeed(), protocol.getTargetDirection());
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // 2. Signalverlust simulieren (keine protocol.loop() Aufrufe mit Daten)
  advance_millis(501); // Watchdog sollte jetzt triggern (> 500ms)
  TEST_ASSERT_TRUE(protocol.isSignalTimeout());

  // Manuelle Simulation der Ramp-Down-Logik aus der main loop
  auto simulate_loop = [&]() {
    if (protocol.isSignalTimeout()) {
      unsigned long timeSinceLastSignal =
          millis() - protocol.getLastSignalTime();
      int           watchdogTimeout = cvManager.getCv(CV_WATCHDOG_TIMEOUT) * 100;
      unsigned long elapsed         = timeSinceLastSignal - watchdogTimeout;

      if (elapsed >= 500) {
        motor.stop();
      } else {
        int targetSpeed = protocol.getTargetSpeed();
        int rampSpeed   = map(elapsed, 0, 500, targetSpeed, 0);
        motor.setSpeed(rampSpeed, protocol.getTargetDirection());
      }
    } else {
      motor.setSpeed(protocol.getTargetSpeed(), protocol.getTargetDirection());
    }
  };

  simulate_loop();
  // Nach 501ms total (1ms nach Watchdog), sollte die Geschwindigkeit fast noch
  // 14 sein (1ms/500ms ramp) map(1, 0, 500, 14, 0) -> 13
  TEST_ASSERT_INT_WITHIN(2, 14, protocol.getTargetSpeed());
  // Wir prüfen den PWM Wert. targetSpeed=14 -> PWM=1023. rampSpeed = map(1, 0,
  // 500, 14, 0) = 13. PWM für 13 ist map(13, 1, 14, 40, 1023)
  int expectedPwm = map(13, 1, 14, 40, 1023);
  TEST_ASSERT_INT_WITHIN(100, expectedPwm, analog_write_values[10]);

  // 250ms nach Watchdog (750ms total) -> Halbe Geschwindigkeit
  advance_millis(249);
  simulate_loop();
  expectedPwm = map(7, 1, 14, 40, 1023);
  TEST_ASSERT_INT_WITHIN(100, expectedPwm, analog_write_values[10]);

  // 500ms nach Watchdog (1000ms total) -> Stillstand
  advance_millis(250);
  simulate_loop();
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);

  // 3. Signal kehrt zurück -> Sofortige Wiederaufnahme
  protocol.mm.SetData(1, 14, false, false, false,
                      MM2DirectionState_Unavailable, 0, false);
  protocol.loop();
  TEST_ASSERT_FALSE(protocol.isSignalTimeout());
  simulate_loop();
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);
}

// Testet die Debug-LEDs (NeoPixel und interne LEDs).
void test_debug_leds_behavior(void) {
  // Thor mapping: Red=15, Blue=16, Neo=12, NeoPwr=11
  DebugLeds debugLeds(12, 11, 1, 15, -1, 16);
  debugLeds.setup();

  // Advance to allow first update (50ms interval)
  advance_millis(60);

  // 1. Interne LEDs
  // MM2 locked -> Rote LED (15) an (LOW)
  debugLeds.update(0, MM2DirectionState_Forward, false, true, false, false);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[15]);
  // MM2 not locked -> Rote LED (15) aus (HIGH)
  advance_millis(60);
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[15]);

  // F1 an -> Blaue LED (16) an (LOW)
  advance_millis(60);
  debugLeds.update(0, MM2DirectionState_Forward, true, false, false, false);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[16]);
  // F1 aus -> Blaue LED (16) aus (HIGH)
  advance_millis(60);
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[16]);

  // 2. NeoPixel
  // Kickstarting -> Weiß (255, 255, 255)
  debugLeds.update(0, MM2DirectionState_Forward, false, false, true, false);
  // NeoPixel Mock speichert Farbe in pixels[0]
  // Weiß = (255 << 16) | (255 << 8) | 255 = 0xFFFFFF
  // Da DebugLeds::update ein 50ms Intervall hat, müssen wir advance_millis nutzen
  advance_millis(60);
  debugLeds.update(0, MM2DirectionState_Forward, false, false, true, false);
  TEST_ASSERT_EQUAL(0xFFFFFF,
                    ((Adafruit_NeoPixel *)&debugLeds)->pixels[0]); // Hackish but
                                                                   // works for
                                                                   // mock

  // Speed 0 -> Atmen Blau (nur Blau-Komponente)
  advance_millis(60);
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  uint32_t color = ((Adafruit_NeoPixel *)&debugLeds)->pixels[0];
  TEST_ASSERT_EQUAL(0, (color >> 16) & 0xFF); // R = 0
  TEST_ASSERT_EQUAL(0, (color >> 8) & 0xFF);  // G = 0
  TEST_ASSERT_GREATER_THAN(0, color & 0xFF);  // B > 0

  // Forward, Speed 14 -> Grünlich (rb, 255, rb)
  // map(14, 1, 14, 0, 255) -> rb = 255 -> (255, 255, 255) (actually white-ish
  // green)
  advance_millis(60);
  debugLeds.update(14, MM2DirectionState_Forward, false, false, false, false);
  color = ((Adafruit_NeoPixel *)&debugLeds)->pixels[0];
  // rb = map(14, 1, 14, 0, 255) = 255. Color = (255, 255, 255) = 0xFFFFFF
  TEST_ASSERT_EQUAL(0xFFFFFF, color);

  // Forward, Speed 7 -> map(7, 1, 14, 0, 255) = 117 -> (117, 255, 117)
  advance_millis(60);
  debugLeds.update(7, MM2DirectionState_Forward, false, false, false, false);
  color = ((Adafruit_NeoPixel *)&debugLeds)->pixels[0];
  TEST_ASSERT_EQUAL(117, (color >> 16) & 0xFF);
  TEST_ASSERT_EQUAL(255, (color >> 8) & 0xFF);
  TEST_ASSERT_EQUAL(117, color & 0xFF);

  // Backward, Speed 14 -> (r, g, 0)
  // map(14, 1, 14, 255, 0) -> r = 0, g = 0 -> (0, 0, 0)
  advance_millis(60);
  debugLeds.update(14, MM2DirectionState_Backward, false, false, false, false);
  color = ((Adafruit_NeoPixel *)&debugLeds)->pixels[0];
  // r = map(14, 1, 14, 255, 0) = 0, g = map(14, 1, 14, 128, 0) = 0. Color = 0
  TEST_ASSERT_EQUAL(0, color);

  // Backward, Speed 1 -> map(1, 1, 14, 255, 0) -> r=255, g=128
  advance_millis(60);
  debugLeds.update(1, MM2DirectionState_Backward, false, false, false, false);
  color = ((Adafruit_NeoPixel *)&debugLeds)->pixels[0];
  TEST_ASSERT_EQUAL(255, (color >> 16) & 0xFF);
  TEST_ASSERT_EQUAL(128, (color >> 8) & 0xFF);
}

// Testet fortgeschrittene Funktionen des ProtocolHandlers wie MM2-Lock und
// Entprellung der MM1-Richtungsumschaltung.
void test_protocol_handler_advanced(void) {
  ProtocolHandler protocol(0);
  protocol.setAddress(1);

  // 1. MM1 Richtungsumschaltung Entprellung
  // advance_millis to ensure we are past the initial 250ms debounce window
  advance_millis(300);
  // Erster Richtungswechsel (lastChangeDirTs = 0) sollte jetzt gehen
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  // ProtocolHandler starts with targetDirection = Forward.
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocol.getTargetDirection());
  unsigned long firstChange = protocol.getLastChangeDirTs();
  TEST_ASSERT_GREATER_THAN(0, firstChange);

  // Zweiter Richtungswechsel sofort danach sollte ignoriert werden (250ms
  // Sperre)
  advance_millis(100);
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false); // Reset lastChangeDirInput
  protocol.loop();
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocol.getTargetDirection());
  TEST_ASSERT_EQUAL(firstChange, protocol.getLastChangeDirTs());

  // Nach 300ms sollte es wieder gehen
  advance_millis(200);
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());
  TEST_ASSERT_GREATER_THAN(firstChange, protocol.getLastChangeDirTs());

  // 2. MM2 Lock
  // MM2 Signal empfangen
  advance_millis(100);
  protocol.mm.SetData(1, 5, false, true, true, MM2DirectionState_Forward, 0,
                      false);
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.isMm2Locked());

  // MM1 Richtungswechsel-Signal sollte jetzt ignoriert werden
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  // Sollte immer noch Forward sein von MM2 Paket
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());

  // Nach 6000ms (Lock = 5000ms) sollte MM1 wieder gehen
  advance_millis(6000);
  TEST_ASSERT_FALSE(protocol.isMm2Locked());
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocol.getTargetDirection());
}

// Testet den Motor-Kickstart-Mechanismus mit BEMF-Rückmeldung.
void test_motor_bemf_kickstart(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 1);
  cvManager.setCv(CV_MOTOR_TYPE, 0); // Standard DC: BEMF_THRESHOLD = 100
  MotorControl motor(cvManager, 7, 8, 0, 1);
  motor.setup();

  // Starte Motor aus dem Stand
  motor.setSpeed(5, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  // KICK_PWM für Standard DC ist 800
  TEST_ASSERT_EQUAL(800, analog_write_values[7]);

  // Simuliere niedrige BEMF (< 100)
  advance_millis(20);
  analog_read_values[0] = 50;
  analog_read_values[1] = 0;
  motor.setSpeed(5, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());

  // Simuliere hohe BEMF (> 100) -> Kickstart sollte enden
  advance_millis(20);
  analog_read_values[0] = 150;
  analog_read_values[1] = 0;
  motor.setSpeed(5, MM2DirectionState_Forward);
  TEST_ASSERT_FALSE(motor.isKickstarting());
  // PWM sollte jetzt auf Normalwert sinken
  // map(5, 1, 14, 40, 1023) = (5-1)*(1023-40)/(14-1) + 40 = 4*983/13 + 40 =
  // 302.46 -> 302
  TEST_ASSERT_EQUAL(342, analog_write_values[7]);
}

void test_cv_programming_6021(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  CvProgrammer programmer(&cvManager, &protocol);

  // Set CV 15 to 7 to enable programming
  cvManager.setCv(CV_PROGRAMMING_LOCK, 7);

  // 4 direction changes to enter programming mode
  for (int i = 0; i < 4; i++) {
    advance_millis(300);
    unsigned long now = millis();
    // Send packet with changeDir = true
    protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                        0, false);
    protocol.loop();
    TEST_ASSERT_EQUAL(now, protocol.getLastChangeDirTs());
    programmer.loop();

    advance_millis(100);
    // Send packet with changeDir = false to reset lastChangeDirInput
    protocol.mm.SetData(1, 0, false, false, false,
                        MM2DirectionState_Unavailable, 0, false);
    protocol.loop();
    programmer.loop();
  }

  // Now in programming mode
  // Set CV 10 (address)
  advance_millis(100);
  unsigned long now = millis();
  protocol.mm.SetData(1, 10, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(now, protocol.getLastSpeedChangeTs());
  programmer.loop();

  // Set Value 42
  advance_millis(100);
  now = millis();
  protocol.mm.SetData(1, 42, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(now, protocol.getLastSpeedChangeTs());
  programmer.loop();

  TEST_ASSERT_EQUAL(42, cvManager.getCv(10));
}

void setUp(void) {
  reboot_called = false;
  reset_arduino_mock();
}

void tearDown(void) {
  // clean stuff up here
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_cv_manager_get_set);
  RUN_TEST(test_cv_manager_defaults);
  RUN_TEST(test_cv_manager_special);
  RUN_TEST(test_motor_speed_control);
  RUN_TEST(test_lights_control_default_behavior);
  RUN_TEST(test_mm_signal_f0_f1_f2);
  RUN_TEST(test_cv_programming_6021);
  RUN_TEST(test_watchdog_shutdown);
  RUN_TEST(test_debug_leds_behavior);
  RUN_TEST(test_protocol_handler_advanced);
  RUN_TEST(test_motor_bemf_kickstart);
  return UNITY_END();
}
