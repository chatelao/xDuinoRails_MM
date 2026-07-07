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
    MotorControl motor(cvManager, 10, 11, 2, 3);
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
    MotorControl motor(cvManager, 10, 11, 2, 3);
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
    MotorControl motor(cvManager, 10, 11, 2, 3);
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
  MotorControl motor(cvManager, 10, 11, 2, 3);
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

  MotorControl motor(cvManager, 10, 11, 2, 3);
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
