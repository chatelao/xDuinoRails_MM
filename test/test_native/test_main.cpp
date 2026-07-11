#include "CvManager.h"
#include "CvManagerMock.h"
#include "CvProgrammer.h"
#include "LightsControl.h"
#include "Logger.h"
#include "MotorControl.h"
#include "ProtocolHandler.h"
#include "RP2040.h"
#include "SerialConsole.h"
#include <unity.h>

void test_mm_signal_f0_f1_f2(void);
void test_cv_programming_6021(void);
void test_watchdog_shutdown(void);
void test_cv_manager_reset_8(void);
void test_motor_speed_curve(void);
void test_logging(void);
void test_logger_categories(void);
void test_pwm_freq_logging(void);
void test_cv_motor_type_reboot(void);
void test_serial_console(void);
void test_serial_console_short_forms(void);
void test_serial_console_cv_readout(void);
void test_cv_manager_print_all(void);
void test_motor_kickstart_bemf_disabled(void);
void test_motor_kickstart_bemf_enabled(void);
void test_motor_pwm_mapping_detailed(void);
void test_motor_pwm_mapping_new_defaults(void);
void test_debug_leds_heartbeat(void);
void test_motor_bemf_pi_control(void);
void test_serial_console_logging_toggle(void);
void test_serial_console_help(void);
void test_repro_watchdog_stop_no_kickstart(void);
void test_repro_start_backward_kickstart_wrong_dir(void);
void test_repro_bemf_disabled_leftover_adjustment(void);
void test_repro_direction_change_kickstart(void);
void test_repro_kickstart_only(void);
void test_repro_kickstart_only_with_vstart_zero(void);
void test_cv49_zero_only_kickstart_works(void);
void test_cv49_zero_with_direction_change(void);
void test_high_speed_logging(void);
void test_pwm_frequency_defaults(void);
void test_pwm_frequency_override(void);
void test_bemf_collector_gap_glitch(void);
void test_bemf_collector_gap_persistent_failure(void);
void test_bemf_collector_gap_with_integral_filtered(void);
void test_bemf_stability_integral_clamping(void);
void test_read_bemf_shutdown_toggling(void);
void test_read_bemf_internal_median(void);

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
  TEST_ASSERT_EQUAL(0, cvManager.getCv(CV_MEDIUM_SPEED));
  TEST_ASSERT_EQUAL(10, cvManager.getCv(CV_VERSION));
  TEST_ASSERT_EQUAL(13, cvManager.getCv(CV_MANUFACTURER_ID));
  TEST_ASSERT_EQUAL(192, cvManager.getCv(CV_LONG_ADDRESS_HIGH));
  TEST_ASSERT_EQUAL(100, cvManager.getCv(CV_LONG_ADDRESS_LOW));
  TEST_ASSERT_EQUAL(6, cvManager.getCv(CV_CONFIGURATION));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_FRONT_LIGHT_F0F));
  TEST_ASSERT_EQUAL(2, cvManager.getCv(CV_REAR_LIGHT_F0R));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_EXT_ID_HIGH));
  TEST_ASSERT_EQUAL(10, cvManager.getCv(CV_EXT_ID_LOW));
  TEST_ASSERT_EQUAL(16, cvManager.getCv(CV_BEMF_K));
  TEST_ASSERT_EQUAL(12, cvManager.getCv(CV_BEMF_I));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_DEBUG_ENABLE));
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
  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
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
  // Mit Korrektur wird hier direkt Kickstart ausgelöst und danach PWM gesetzt
  motor.setSpeed(1, MM2DirectionState_Backward);
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
  motor.setSpeed(14, MM2DirectionState_Backward);
  TEST_ASSERT_EQUAL(LOW, digital_write_values[10]);
  TEST_ASSERT_EQUAL(1023, analog_write_values[11]);
}

// Test Lights Control
// Testet das Standardverhalten der Lichtsteuerung.
void test_lights_control_default_behavior(void) {
  CvManagerMock cvManager;
  LightsControl lights(cvManager, 0, 1, 13);
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

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
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
  // New default CV 5 = 0 -> PWM 1023
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
  // 500, 14, 0) = 13. PWM für 13 is map(13, 7, 14, 531, 1023) = 952
  int expectedPwm = 952;
  TEST_ASSERT_INT_WITHIN(100, expectedPwm, analog_write_values[10]);

  // 250ms nach Watchdog (750ms total) -> Halbe Geschwindigkeit
  advance_millis(249);
  simulate_loop();
  // Step 7 -> Vmid = 531
  expectedPwm = 531;
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

  // With kickstart triggered after stop(), PWM will be KICK_PWM (800) instead of 1023
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);

  // After kickstart timeout, it should reach 1023
  advance_millis(150);
  simulate_loop();
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);
}

void test_pwm_freq_logging(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);

  // Test Standard DC (Type 0)
  cvManager.setCv(CV_MOTOR_TYPE, 0);
  Serial.clearLog();
  {
    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();
    bool found = false;
    for (const auto &line : Serial.logLines) {
      if (line.find("Motor: Type=Standard DC, PWM Freq=20000 Hz") !=
          std::string::npos) {
        found = true;
        break;
      }
    }
    TEST_ASSERT_TRUE(found);
  }

  // Test Faulhaber (Type 1)
  cvManager.setCv(CV_MOTOR_TYPE, 1);
  Serial.clearLog();
  {
    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();
    bool found = false;
    for (const auto &line : Serial.logLines) {
      if (line.find("Motor: Type=Faulhaber, PWM Freq=400 Hz") !=
          std::string::npos) {
        found = true;
        break;
      }
    }
    TEST_ASSERT_TRUE(found);
  }

  // Test Maxon (Type 2)
  cvManager.setCv(CV_MOTOR_TYPE, 2);
  Serial.clearLog();
  {
    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();
    bool found = false;
    for (const auto &line : Serial.logLines) {
      if (line.find("Motor: Type=Maxon, PWM Freq=20000 Hz") !=
          std::string::npos) {
        found = true;
        break;
      }
    }
    TEST_ASSERT_TRUE(found);
  }
}

void test_cv_motor_type_reboot(void) {
  CvManager cvManager;
  cvManager.setup();

  reboot_called = false;
  cvManager.setCv(CV_MOTOR_TYPE, 1);
  TEST_ASSERT_TRUE(reboot_called);
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_MOTOR_TYPE));
}

void test_logging(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);
  Serial.clearLog();

  ProtocolHandler protocol(0);
  protocol.setAddress(1);

  // Test state change logging
  protocol.mm.SetData(1, 5, true, false, false, MM2DirectionState_Forward, 0,
                      false);
  protocol.loop();

  bool foundStateLog = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("State: Addr=1, Speed=5") != std::string::npos) {
      foundStateLog = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundStateLog);
  Serial.clearLog();

  // Test redundant packet (no log)
  protocol.mm.SetData(1, 5, true, false, false, MM2DirectionState_Forward, 0,
                      false);
  protocol.loop();
  TEST_ASSERT_EQUAL(0, Serial.logLines.size());

  // Test signal lost
  advance_millis(600); // Default timeout is 500ms
  protocol.loop();
  bool foundSignalLost = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("MM Signal lost") != std::string::npos) {
      foundSignalLost = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundSignalLost);
  Serial.clearLog();

  // Test signal recovered
  protocol.mm.SetData(1, 5, true, false, false, MM2DirectionState_Forward, 0,
                      false);
  protocol.loop();
  bool foundSignalRecovered = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("MM Signal recovered") != std::string::npos) {
      foundSignalRecovered = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundSignalRecovered);
  Serial.clearLog();

  // Test Motor kickstart logging
  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();
  motor.setSpeed(1, MM2DirectionState_Forward);

  bool foundKickstartStarted = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("Motor: Kickstart started") != std::string::npos) {
      foundKickstartStarted = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundKickstartStarted);
  Serial.clearLog();

  // Test Motor PWM logging
  motor.setSpeed(7, MM2DirectionState_Forward);
  bool foundPwmLog = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("Motor: Step 7 -> PWM") != std::string::npos) {
      foundPwmLog = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundPwmLog);
  Serial.clearLog();

  // Test redundant call (no log)
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(0, Serial.logLines.size());

  // Test direction change log
  motor.setSpeed(7, MM2DirectionState_Backward);
  foundPwmLog = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("Motor: Step 7 -> PWM") != std::string::npos &&
        line.find("Dir Backward") != std::string::npos) {
      foundPwmLog = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundPwmLog);
}

void test_serial_console(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  SerialConsole console(&cvManager, &protocol);

  // Test CV command
  Serial.pushInput("cv 1 44\n");
  console.loop();
  TEST_ASSERT_EQUAL(44, cvManager.getCv(1));

  // Test Speed command
  Serial.pushInput("s 12\n");
  console.loop();
  TEST_ASSERT_EQUAL(12, protocol.getTargetSpeed());

  // Test Direction command
  Serial.pushInput("d f\n");
  console.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());

  Serial.pushInput("d b\n");
  console.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocol.getTargetDirection());

  // Test Function command
  Serial.pushInput("f 1\n");
  console.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(1));

  Serial.pushInput("f 0\n");
  console.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(1));
}

void test_serial_console_short_forms(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  SerialConsole console(&cvManager, &protocol);

  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);

  // Test "cv8 8"
  Serial.pushInput("cv8 8\n");
  console.loop();
  TEST_ASSERT_EQUAL(8, cvManager.getCv(8));

  // Test "s10"
  Serial.pushInput("s10\n");
  console.loop();
  TEST_ASSERT_EQUAL(10, protocol.getTargetSpeed());

  // Test "df"
  Serial.pushInput("df\n");
  console.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());

  // Test "db"
  Serial.pushInput("db\n");
  console.loop();
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocol.getTargetDirection());

  // Test "f1"
  Serial.pushInput("f1\n");
  console.loop();
  TEST_ASSERT_TRUE(protocol.getFunctionState(1));

  // Test "f0"
  Serial.pushInput("f0\n");
  console.loop();
  TEST_ASSERT_FALSE(protocol.getFunctionState(1));

  // Test "lh" (High speed toggle)
  bool initialHighSpeed = logger.isHighSpeedEnabled();
  Serial.pushInput("lh\n");
  console.loop();
  TEST_ASSERT_EQUAL(!initialHighSpeed, logger.isHighSpeedEnabled());
  // Toggle back to restore state for other tests
  Serial.pushInput("lh\n");
  console.loop();
}

void test_serial_console_cv_readout(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  SerialConsole console(&cvManager, &protocol);

  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);
  Serial.clearLog();

  // Test "cv" command for readout
  Serial.pushInput("cv\n");
  console.loop();

  bool foundHeader = false;
  bool foundAddress = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("--- Current CV Settings ---") != std::string::npos)
      foundHeader = true;
    if (line.find("CV 1 (Address): 3") != std::string::npos)
      foundAddress = true;
  }
  TEST_ASSERT_TRUE(foundHeader);
  TEST_ASSERT_TRUE(foundAddress);
}

void test_cv_manager_print_all(void) {
  CvManager cvManager;
  cvManager.setup();
  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);
  Serial.clearLog();

  cvManager.printAllCvs();

  bool foundHeader     = false;
  bool foundAddress    = false;
  bool foundVersion    = false;
  bool foundFooter     = false;
  bool foundLongAddr   = false;
  bool foundExtId      = false;

  for (const auto &line : Serial.logLines) {
    if (line.find("--- Current CV Settings ---") != std::string::npos)
      foundHeader = true;
    if (line.find("CV 1 (Address): 3") != std::string::npos)
      foundAddress = true;
    if (line.find("CV 7 (Version): 10") != std::string::npos)
      foundVersion = true;
    if (line.find("CV 17/18 (Long Addr): 49252") != std::string::npos) // (192 << 8) | 100 = 49152 + 100 = 49252
      foundLongAddr = true;
    if (line.find("CV 107/108 (Ext ID): 266") != std::string::npos) // (1 << 8) | 10 = 256 + 10 = 266
      foundExtId = true;
    if (line.find("---------------------------") != std::string::npos)
      foundFooter = true;
  }

  TEST_ASSERT_TRUE(foundHeader);
  TEST_ASSERT_TRUE(foundAddress);
  TEST_ASSERT_TRUE(foundVersion);
  TEST_ASSERT_TRUE(foundLongAddr);
  TEST_ASSERT_TRUE(foundExtId);
  TEST_ASSERT_TRUE(foundFooter);
}

void test_motor_kickstart_bemf_disabled(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);

  // BEMF is disabled by default, but we set it explicitly here too
  cvManager.setCv(CV_BEMF_CONFIG, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  Serial.clearLog();

  // Start kickstart
  motor.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());

  // Simulate BEMF detected
  analog_read_values[2] = 500; // BemfA
  analog_read_values[3] = 0;   // BemfB -> diff = 500 > threshold (100)

  // Update motor - should NOT end kickstart because BEMF is disabled
  advance_millis(20);
  motor.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());

  bool foundBemfLog = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("Motor: Kickstart ended (BEMF)") != std::string::npos) {
      foundBemfLog = true;
      break;
    }
  }
  TEST_ASSERT_FALSE(foundBemfLog);

  // Wait for timeout (default 100ms for type 0)
  advance_millis(100);
  motor.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_FALSE(motor.isKickstarting());

  bool foundTimeoutLog = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("Motor: Kickstart ended (timeout)") != std::string::npos) {
      foundTimeoutLog = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundTimeoutLog);
}

void test_motor_kickstart_bemf_enabled(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);

  // Enable BEMF
  cvManager.setCv(CV_BEMF_CONFIG, 1);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  Serial.clearLog();

  // Start kickstart
  motor.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());

  // Simulate BEMF detected
  analog_read_values[2] = 500; // BemfA
  analog_read_values[3] = 0;   // BemfB -> diff = 500 > threshold (100)

  // Update motor - should end kickstart because BEMF is enabled
  advance_millis(20);
  motor.setSpeed(1, MM2DirectionState_Forward);
  TEST_ASSERT_FALSE(motor.isKickstarting());

  bool foundBemfLog = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("Motor: Kickstart ended (BEMF)") != std::string::npos) {
      foundBemfLog = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundBemfLog);
}

void test_motor_speed_curve(void) {
  CvManagerMock cvManager;

  // Case 1: Standard linear curve (Vmid = 0 -> default midpoint)
  // Vstart = 50 (PWM ~200), Vhigh = 200 (PWM ~803), Vmid = 0 (default ~501)
  cvManager.setCv(CV_START_VOLTAGE, 50);
  cvManager.setCv(CV_MAXIMUM_SPEED, 200);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  MotorControl motor1(cvManager, 10, 11, 2, 3, 12);
  motor1.setup();

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

void test_serial_console_logging_toggle(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  SerialConsole console(&cvManager, &protocol);

  // Initial state (enabled by default)
  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);
  TEST_ASSERT_TRUE(logger.isLoggingEnabled());

  // Toggle OFF
  Serial.pushInput("L\n");
  console.loop();
  TEST_ASSERT_FALSE(logger.isLoggingEnabled());
  TEST_ASSERT_EQUAL(0, cvManager.getCv(CV_DEBUG_ENABLE));

  // Toggle ON (using lowercase 'l')
  Serial.pushInput("l\n");
  console.loop();
  TEST_ASSERT_TRUE(logger.isLoggingEnabled());
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_DEBUG_ENABLE));
}

void test_serial_console_help(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  SerialConsole console(&cvManager, &protocol);

  Serial.clearLog();

  // Test "h" command
  Serial.pushInput("h\n");
  console.loop();

  bool foundHelpHeader = false;
  bool foundCvCmd      = false;
  bool foundSpeedCmd   = false;
  bool foundHelpCmd    = false;

  for (const auto &line : Serial.logLines) {
    if (line.find("--- xDuinoRails CLI Help ---") != std::string::npos)
      foundHelpHeader = true;
    if (line.find("cv <num> <val> : Set CV value") != std::string::npos ||
        line.find("cv <num> <val> : Set CV value (short form: cv<num> <val>)") != std::string::npos)
      foundCvCmd = true;
    if (line.find("s <speed>      : Set speed") != std::string::npos)
      foundSpeedCmd = true;
    if (line.find("h/?            : Show this help") != std::string::npos)
      foundHelpCmd = true;
  }
  TEST_ASSERT_TRUE(foundHelpHeader);
  TEST_ASSERT_TRUE(foundCvCmd);
  TEST_ASSERT_TRUE(foundSpeedCmd);
  TEST_ASSERT_TRUE(foundHelpCmd);

  Serial.clearLog();

  // Test "?" command
  Serial.pushInput("?\n");
  console.loop();

  foundHelpHeader = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("--- xDuinoRails CLI Help ---") != std::string::npos) {
      foundHelpHeader = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundHelpHeader);
}

void test_high_speed_logging(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  SerialConsole console(&cvManager, &protocol);

  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  cvManager.setCv(CV_BEMF_CONFIG, 1); // Enable BEMF
  logger.begin(&cvManager);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Initially OFF
  TEST_ASSERT_FALSE(logger.isHighSpeedEnabled());

  // Toggle ON
  Serial.clearLog();
  Serial.pushInput("l h\n");
  console.loop();
  TEST_ASSERT_TRUE(logger.isHighSpeedEnabled());

  // Set speed to trigger control loop
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Past kickstart
  motor.setSpeed(7, MM2DirectionState_Forward); // Update

  // Should see CSV output and header
  bool foundHeader = false;
  bool foundCsv = false;
  for (const auto &line : Serial.logLines) {
    if (line.find("CSV | timestamp | targetPwm | rawBEMF | filteredBEMF | error | integral | adjustment | finalPwm | K | I") != std::string::npos) {
      foundHeader = true;
    }
    if (line.find("CSV | ") == 0) {
      foundCsv = true;
    }
  }
  TEST_ASSERT_TRUE(foundHeader);
  TEST_ASSERT_TRUE(foundCsv);

  // Toggle OFF
  Serial.pushInput("l h\n");
  console.loop();
  TEST_ASSERT_FALSE(logger.isHighSpeedEnabled());
}

void test_cv_programming_6021(void) {
  CvManagerMock   cvManager;
  ProtocolHandler protocol(0);
  protocol.setAddress(1);
  CvProgrammer programmer(&cvManager, &protocol);

  // Set CV 15 to 7 to enable programming
  cvManager.setCv(CV_PROGRAMMING_LOCK, 7);

  // Set first signal time to non-zero
  advance_millis(1000);

  // 4 direction changes to enter programming mode
  for (int i = 0; i < 4; i++) {
    advance_millis(300);
    // Send packet with changeDir = true
    protocol.mm.SetData(1, 0, false, true, false, MM2DirectionState_Unavailable,
                        0, false);
    protocol.loop();
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
  RUN_TEST(test_logging);
  RUN_TEST(test_logger_categories);
  RUN_TEST(test_pwm_freq_logging);
  RUN_TEST(test_cv_motor_type_reboot);
  RUN_TEST(test_serial_console);
  RUN_TEST(test_serial_console_short_forms);
  RUN_TEST(test_serial_console_cv_readout);
  RUN_TEST(test_cv_manager_print_all);
  RUN_TEST(test_motor_kickstart_bemf_disabled);
  RUN_TEST(test_motor_kickstart_bemf_enabled);
  RUN_TEST(test_motor_pwm_mapping_detailed);
  RUN_TEST(test_motor_pwm_mapping_new_defaults);
  RUN_TEST(test_debug_leds_heartbeat);
  RUN_TEST(test_motor_bemf_pi_control);
  RUN_TEST(test_serial_console_logging_toggle);
  RUN_TEST(test_serial_console_help);
  RUN_TEST(test_repro_watchdog_stop_no_kickstart);
  RUN_TEST(test_repro_start_backward_kickstart_wrong_dir);
  RUN_TEST(test_repro_bemf_disabled_leftover_adjustment);
  RUN_TEST(test_repro_direction_change_kickstart);
  RUN_TEST(test_repro_kickstart_only);
  RUN_TEST(test_repro_kickstart_only_with_vstart_zero);
  RUN_TEST(test_cv49_zero_only_kickstart_works);
  RUN_TEST(test_cv49_zero_with_direction_change);
  RUN_TEST(test_high_speed_logging);
  RUN_TEST(test_pwm_frequency_defaults);
  RUN_TEST(test_pwm_frequency_override);
  RUN_TEST(test_bemf_collector_gap_glitch);
  RUN_TEST(test_bemf_collector_gap_persistent_failure);
  RUN_TEST(test_bemf_collector_gap_with_integral_filtered);
  RUN_TEST(test_bemf_stability_integral_clamping);
  RUN_TEST(test_read_bemf_shutdown_toggling);
  RUN_TEST(test_read_bemf_internal_median);
  return UNITY_END();
}
