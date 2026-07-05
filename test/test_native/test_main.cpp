#include "CvManager.h"
#include "CvManagerMock.h"
#include "CvProgrammer.h"
#include "LightsControl.h"
#include "MotorControl.h"
#include "ProtocolHandler.h"
#include "RP2040.h"
#include <unity.h>

void test_mm_signal_f0_f1_f2(void);
void test_cv_programming_6021(void);
void test_watchdog_shutdown(void);
void test_cv_manager_reset_8(void);

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

  // Teste den Reset-Mechanismus durch Schreiben von 0 auf CV_MANUFACTURER_ID
  // Ändere einen Wert, um den Reset zu verifizieren
  cvManager.setCv(CV_BASE_ADDRESS, 42);
  TEST_ASSERT_EQUAL(42, cvManager.getCv(CV_BASE_ADDRESS));

  reboot_called = false;
  cvManager.setCv(CV_MANUFACTURER_ID, 0);
  // Überprüfe, ob der Neustart ausgelöst wurde
  TEST_ASSERT_TRUE(reboot_called);
  // Überprüfe, ob die Werte zurückgesetzt wurden
  TEST_ASSERT_EQUAL(3, cvManager.getCv(CV_BASE_ADDRESS));
}

// Testet den Reset-Mechanismus durch Schreiben von 8 auf CV_MANUFACTURER_ID.
void test_cv_manager_reset_8(void) {
  CvManager cvManager;
  cvManager.setup();

  // Ändere einen Wert, um den Reset zu verifizieren
  cvManager.setCv(CV_BASE_ADDRESS, 42);
  TEST_ASSERT_EQUAL(42, cvManager.getCv(CV_BASE_ADDRESS));

  reboot_called = false;
  cvManager.setCv(CV_MANUFACTURER_ID, 8);
  // Überprüfe, ob der Neustart ausgelöst wurde
  TEST_ASSERT_TRUE(reboot_called);
  // Überprüfe, ob die Werte zurückgesetzt wurden
  TEST_ASSERT_EQUAL(3, cvManager.getCv(CV_BASE_ADDRESS));
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
  LightsControl lights(cvManager, 0, 1);
  // TODO: Behauptungen hinzufügen, um das Standard-Lichtverhalten zu überprüfen
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
  RUN_TEST(test_cv_manager_reset_8);
  RUN_TEST(test_motor_speed_control);
  RUN_TEST(test_lights_control_default_behavior);
  RUN_TEST(test_mm_signal_f0_f1_f2);
  RUN_TEST(test_cv_programming_6021);
  RUN_TEST(test_watchdog_shutdown);
  return UNITY_END();
}
