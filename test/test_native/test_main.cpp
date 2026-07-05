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
void test_cv_manager_reset_8(void);
void test_motor_speed_curve(void);
void test_debug_leds(void);
void test_protocol_handler_lock_and_debounce(void);
void test_motor_bemf_kickstart(void);
void test_cv_programmer_timeout_and_toggle(void);

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
  TEST_ASSERT_EQUAL(10, cvManager.getCv(CV_START_VOLTAGE));
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
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MOTOR_TYPE, 0);
  cvManager.setCv(CV_MAXIMUM_SPEED, 255);
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
  // Überprüfe den erwarteten PWM-Wert (Vstart = map(1, 0, 255, 0, 1023) = 4)
  // Aber wir haben jetzt Standard CV_START_VOLTAGE = 10 -> map(10, 0, 255, 0, 1023) = 40
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
  LightsControl lights(cvManager, 10, 11); // Front: 10, Rear: 11
  lights.setup();

  cvManager.setCv(CV_FRONT_LIGHT_F0F, 1); // Bit 0 set
  cvManager.setCv(CV_REAR_LIGHT_F0R, 2);  // Bit 1 set

  // Case 1: F0 off -> Both off
  lights.update(MM2DirectionState_Forward, false);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[11]);

  // Case 2: F0 on, Forward -> Front on, Rear off
  lights.update(MM2DirectionState_Forward, true);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[10]);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[11]);

  // Case 3: F0 on, Backward -> Front off, Rear on
  lights.update(MM2DirectionState_Backward, true);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[11]);

  // Case 4: CVs disable lights
  cvManager.setCv(CV_FRONT_LIGHT_F0F, 0);
  cvManager.setCv(CV_REAR_LIGHT_F0R, 0);
  lights.update(MM2DirectionState_Forward, true);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);
  lights.update(MM2DirectionState_Backward, true);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[11]);
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

void test_motor_speed_curve(void) {
  CvManagerMock cvManager;

  // Case 1: Standard linear curve (Vmid = 0 -> default midpoint)
  // Vstart = 50 (PWM ~200), Vhigh = 200 (PWM ~803), Vmid = 0 (default ~501)
  cvManager.setCv(CV_START_VOLTAGE, 50);
  cvManager.setCv(CV_MAXIMUM_SPEED, 200);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  MotorControl motor1(cvManager, 10, 11, 2, 3);

  motor1.setSpeed(1, MM2DirectionState_Forward);
  advance_millis(101);
  motor1.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_INT_WITHIN(5, 200, analog_write_values[10]);

  motor1.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_INT_WITHIN(5, 501, analog_write_values[10]);

  motor1.setSpeed(14, MM2DirectionState_Forward);
  TEST_ASSERT_INT_WITHIN(5, 803, analog_write_values[10]);

  // Case 2: Custom Vmid (Non-linear)
  // Vstart = 10 (PWM 40), Vmid = 200 (PWM 803), Vhigh = 255 (PWM 1023)
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MEDIUM_SPEED, 200);
  cvManager.setCv(CV_MAXIMUM_SPEED, 255);

  motor1.setSpeed(1, MM2DirectionState_Forward);
  advance_millis(101);
  motor1.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(40, analog_write_values[10]);

  motor1.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_INT_WITHIN(2, 803, analog_write_values[10]);

  motor1.setSpeed(14, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // Check an intermediate step (e.g. step 4, halfway between 1 and 7)
  motor1.setSpeed(4, MM2DirectionState_Forward);
  // map(4, 1, 7, 40, 803) = 40 + (803-40)/2 = 40 + 381 = 421
  TEST_ASSERT_INT_WITHIN(2, 421, analog_write_values[10]);
}

void test_cv_programmer_timeout_and_toggle(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  CvProgrammer programmer(&cvManager, &protocol);

  cvManager.setCv(CV_PROGRAMMING_LOCK, 7);

  // 1. Direction changes too slow -> count reset
  advance_millis(300);
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  programmer.loop();

  advance_millis(2100); // > 2000ms
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  programmer.loop();

  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  programmer.loop();
  // Count should be 1, not 2

  // 2. Complete 4 changes within time -> Toggle ON
  for (int i = 0; i < 3; i++) {
    advance_millis(100);
    protocol.mm.SetData(1, 0, false, false, false,
                        MM2DirectionState_Unavailable, 0, false);
    protocol.loop();
    programmer.loop();

    advance_millis(300);
    protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                        0, false);
    protocol.loop();
    programmer.loop();
  }
  TEST_ASSERT_TRUE(programmer.isInProgrammingMode());

  // 3. Complete another 4 changes -> Toggle OFF
  for (int i = 0; i < 4; i++) {
    advance_millis(100);
    protocol.mm.SetData(1, 0, false, false, false,
                        MM2DirectionState_Unavailable, 0, false);
    protocol.loop();
    programmer.loop();

    advance_millis(300);
    protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                        0, false);
    protocol.loop();
    programmer.loop();
  }
  TEST_ASSERT_FALSE(programmer.isInProgrammingMode());
}

void test_motor_bemf_kickstart(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MOTOR_TYPE, 0); // Standard DC: BEMF_THRESHOLD=100
  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Start motor from 0 -> Kickstart begins
  motor.setSpeed(5, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]); // KICK_PWM for Standard DC

  // Simulate BEMF below threshold
  advance_millis(20); // > BEMF_SAMPLE_INT (12ms)
  analog_read_values[2] = 50;
  analog_read_values[3] = 0; // diff = 50 < 100
  motor.setSpeed(5, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());

  // Simulate BEMF above threshold
  advance_millis(20);
  analog_read_values[2] = 150;
  analog_read_values[3] = 0; // diff = 150 > 100
  motor.setSpeed(5, MM2DirectionState_Forward);
  TEST_ASSERT_FALSE(motor.isKickstarting());

  // Normal PWM should be applied now
  // vStart=40, vHigh=1023, vMid=531
  // step 5 is map(5, 1, 7, 40, 531) = 40 + (531-40)*4/6 = 40 + 491*2/3 = 40 + 327 = 367
  TEST_ASSERT_INT_WITHIN(5, 367, analog_write_values[10]);
}

void test_protocol_handler_lock_and_debounce(void) {
  ProtocolHandler protocol(0);
  protocol.setAddress(1);

  // 1. MM2 Lock-in Logic
  // First packet MUST be MM2 to establish lastMM2Seen > 0
  advance_millis(1000); // Start at T=1000
  protocol.mm.SetData(1, 10, false, false, true, MM2DirectionState_Forward, 0,
                      false); // address=1, speed=10, isMM2=true
  protocol.loop();
  TEST_ASSERT_TRUE(protocol.isMm2Locked());

  // MM1 ChangeDir packet while locked -> should be ignored
  advance_millis(100);
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());

  // 2. MM1 Direction Change Debounce
  // Reset lock by advancing time
  advance_millis(6000); // MM2_LOCK_TIME is 5000
  TEST_ASSERT_FALSE(protocol.isMm2Locked());

  // Initial state: Forward
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());

  // First changeDir=true
  // We need to ensure lastChangeDirInput is false before sending true
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();

  advance_millis(300);
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocol.getTargetDirection());
  unsigned long firstChangeTs = protocol.getLastChangeDirTs();
  TEST_ASSERT_EQUAL(millis(), firstChangeTs);

  // Reset changeDir input in next packet
  advance_millis(10);
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();

  // Second changeDir=true within 250ms -> should be ignored
  advance_millis(100);
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocol.getTargetDirection());
  TEST_ASSERT_EQUAL(firstChangeTs, protocol.getLastChangeDirTs());

  // Reset changeDir input
  protocol.mm.SetData(1, 0, false, false, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();

  // Third changeDir=true after > 250ms -> should be accepted
  advance_millis(200); // 100 + 200 = 300ms total
  protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                      0, false);
  protocol.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());
  TEST_ASSERT_GREATER_THAN(firstChangeTs, protocol.getLastChangeDirTs());
}

void test_debug_leds(void) {
  DebugLeds leds(0, 1, 1, 2, 3, 4); // Neo:0, NeoPwr:1, R:2, G:3, B:4
  leds.setup();

  Adafruit_NeoPixel *neo = Adafruit_NeoPixel::lastInstance;
  TEST_ASSERT_NOT_NULL(neo);

  // 1. Timeout -> Flashing Red
  advance_millis(250); // now = 250
  leds.update(0, MM2DirectionState_Forward, false, true, false, true);
  // (250/250)%2 == 1 -> Red
  TEST_ASSERT_EQUAL(Adafruit_NeoPixel::Color(255, 0, 0), neo->getPixelColor(0));

  advance_millis(250); // now = 500
  leds.update(0, MM2DirectionState_Forward, false, true, false, true);
  // (500/250)%2 == 0 -> 0
  TEST_ASSERT_EQUAL(0, neo->getPixelColor(0));

  // 2. Kickstarting -> White
  advance_millis(60);
  leds.update(0, MM2DirectionState_Forward, false, true, true, false);
  TEST_ASSERT_EQUAL(Adafruit_NeoPixel::Color(255, 255, 255),
                    neo->getPixelColor(0));

  // 3. Speed 0 -> Breathing Blue
  advance_millis(60);
  leds.update(0, MM2DirectionState_Forward, false, true, false, false);
  uint32_t color = neo->getPixelColor(0);
  TEST_ASSERT_EQUAL(0, (color >> 16) & 0xFF); // R
  TEST_ASSERT_EQUAL(0, (color >> 8) & 0xFF);  // G
  TEST_ASSERT_GREATER_THAN(0, color & 0xFF);  // B should be > 0

  // 4. Forward -> Green/White Gradient
  advance_millis(60);
  leds.update(1, MM2DirectionState_Forward, false, true, false, false);
  color = neo->getPixelColor(0);
  // map(1, 1, 14, 0, 255) = 0 -> Color(0, 255, 0)
  TEST_ASSERT_EQUAL(Adafruit_NeoPixel::Color(0, 255, 0), color);

  advance_millis(60);
  leds.update(14, MM2DirectionState_Forward, false, true, false, false);
  color = neo->getPixelColor(0);
  // map(14, 1, 14, 0, 255) = 255 -> Color(255, 255, 255)
  TEST_ASSERT_EQUAL(Adafruit_NeoPixel::Color(255, 255, 255), color);

  // 5. Backward -> Orange/Black Gradient
  advance_millis(60);
  leds.update(1, MM2DirectionState_Backward, false, true, false, false);
  color = neo->getPixelColor(0);
  // map(1, 1, 14, 255, 0) = 255, map(1, 1, 14, 128, 0) = 128 -> Color(255, 128, 0)
  TEST_ASSERT_EQUAL(Adafruit_NeoPixel::Color(255, 128, 0), color);

  advance_millis(60);
  leds.update(14, MM2DirectionState_Backward, false, true, false, false);
  color = neo->getPixelColor(0);
  // map(14, 1, 14, 255, 0) = 0, map(14, 1, 14, 128, 0) = 0 -> Color(0, 0, 0)
  TEST_ASSERT_EQUAL(0, color);

  // 6. Internal LEDs
  // Red Pin (2) -> MM2 Locked. (LOW is ON)
  advance_millis(60);
  leds.update(0, MM2DirectionState_Forward, false, true, false, false);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[2]);
  advance_millis(60);
  leds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[2]);

  // Blue Pin (4) -> F1
  advance_millis(60);
  leds.update(0, MM2DirectionState_Forward, true, true, false, false);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[4]);
  advance_millis(60);
  leds.update(0, MM2DirectionState_Forward, false, true, false, false);
  TEST_ASSERT_EQUAL(HIGH, digital_write_values[4]);
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
  RUN_TEST(test_motor_speed_curve);
  RUN_TEST(test_debug_leds);
  RUN_TEST(test_protocol_handler_lock_and_debounce);
  RUN_TEST(test_motor_bemf_kickstart);
  RUN_TEST(test_cv_programmer_timeout_and_toggle);
  return UNITY_END();
}
