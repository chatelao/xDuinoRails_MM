#include "CvManagerMock.h"
#include "Logger.h"
#include "ProtocolHandler.h"
#include "MotorControl.h"
#include "SerialConsole.h"
#include <unity.h>
#include <string>
#include <algorithm>

void test_logger_categories(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_DEBUG_ENABLE, 1);
  logger.begin(&cvManager);

  ProtocolHandler protocol(0);
  protocol.setAddress(1);

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  SerialConsole console(&cvManager, &protocol);

  // 1. Initial State: All enabled
  Serial.clearLog();
  logger.printf(LogCategory::Protocol, "Protocol Message");
  logger.printf(LogCategory::PWM, "PWM Message");
  logger.printf(LogCategory::CV, "CV Message");
  logger.printf(LogCategory::General, "General Message");

  TEST_ASSERT_EQUAL(4, Serial.logLines.size());

  // 2. Disable Protocol
  Serial.clearLog();
  Serial.pushInput("l p\n");
  console.loop();

  logger.printf(LogCategory::Protocol, "Protocol Hidden");
  logger.printf(LogCategory::PWM, "PWM Visible");

  bool foundProtocol = false;
  bool foundPwm = false;
  for (const auto& line : Serial.logLines) {
      if (line.find("Protocol Hidden") != std::string::npos) foundProtocol = true;
      if (line.find("PWM Visible") != std::string::npos) foundPwm = true;
  }
  TEST_ASSERT_FALSE(foundProtocol);
  TEST_ASSERT_TRUE(foundPwm);

  // 3. Disable PWM
  Serial.clearLog();
  Serial.pushInput("l w\n");
  console.loop();

  logger.printf(LogCategory::PWM, "PWM Hidden");
  logger.printf(LogCategory::CV, "CV Visible");

  bool foundPwmHidden = false;
  bool foundCvVisible = false;
  for (const auto& line : Serial.logLines) {
      if (line.find("PWM Hidden") != std::string::npos) foundPwmHidden = true;
      if (line.find("CV Visible") != std::string::npos) foundCvVisible = true;
  }
  TEST_ASSERT_FALSE(foundPwmHidden);
  TEST_ASSERT_TRUE(foundCvVisible);

  // 4. Disable CV
  Serial.clearLog();
  Serial.pushInput("l c\n");
  console.loop();

  logger.printf(LogCategory::CV, "CV Hidden");
  logger.printf(LogCategory::General, "General Visible");

  bool foundCvHidden = false;
  bool foundGeneralVisible = false;
  for (const auto& line : Serial.logLines) {
      if (line.find("CV Hidden") != std::string::npos) foundCvHidden = true;
      if (line.find("General Visible") != std::string::npos) foundGeneralVisible = true;
  }
  TEST_ASSERT_FALSE(foundCvHidden);
  TEST_ASSERT_TRUE(foundGeneralVisible);

  // 5. Re-enable all
  Serial.pushInput("l p\n");
  Serial.pushInput("l w\n");
  Serial.pushInput("l c\n");
  console.loop();

  Serial.clearLog();
  logger.printf(LogCategory::Protocol, "P");
  logger.printf(LogCategory::PWM, "W");
  logger.printf(LogCategory::CV, "C");
  TEST_ASSERT_EQUAL(3, Serial.logLines.size());

  // 6. Master Toggle
  Serial.pushInput("l\n");
  console.loop();
  TEST_ASSERT_FALSE(logger.isLoggingEnabled());

  Serial.clearLog();
  logger.printf(LogCategory::General, "Should be hidden");
  TEST_ASSERT_EQUAL(0, Serial.logLines.size());
}
