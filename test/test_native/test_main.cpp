#include "CvManager.h"
#include "LightsControl.h"
#include "MotorControl.h"
#include "ProtocolHandler.h"
#include "RealTimeProtocolDecoder.h"
#include "RP2040.h"
#include "mocks/CvManager.h"
#include <unity.h>

void test_mm_signal_f0_f1_f2(void);

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

// Testet spezielle CV-Funktionen wie schreibgeschützte CVs und den Reset-Mechanismus.
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
  MotorControl  motor(cvManager, 10, 11, 2, 3);
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
  motor.setSpeed(1, MM2DirectionState_Backward); // Dieser Aufruf stoppt den Motor
  motor.setSpeed(1, MM2DirectionState_Backward); // Dieser wendet die Leistung an
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
  RealTimeProtocolDecoder decoder(0);
  ProtocolHandler         protocol(decoder);
  protocol.setAddress(1);

  // Simuliere ein MM2-Signal mit F1 an
  decoder.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 1,
                     true);
  decoder.loop();
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(1));

  // Simuliere ein MM2-Signal mit F1 aus
  decoder.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 1,
                     false);
  decoder.loop();
  protocol.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(1));

  // Simuliere ein MM2-Signal mit F2 an
  decoder.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 2,
                     true);
  decoder.loop();
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(2));

  // Simuliere ein MM2-Signal mit F2 aus
  decoder.mm.SetData(1, 0, false, true, true, MM2DirectionState_Forward, 2,
                     false);
  decoder.loop();
  protocol.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(2));

  // Simuliere ein MM-Signal mit F0 an
  decoder.mm.SetData(1, 0, true, false, false, MM2DirectionState_Unavailable, 0,
                     false);
  decoder.loop();
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(0));

  // Simuliere ein MM-Signal mit F0 aus
  decoder.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                     0, false);
  decoder.loop();
  protocol.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(0));
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
  return UNITY_END();
}
