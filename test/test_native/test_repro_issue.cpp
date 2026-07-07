#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>
#include <map>

extern std::map<uint8_t, int> analog_write_values;
extern void advance_millis(unsigned long ms);

void test_repro_kickstart_only(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_BEMF_CONFIG, 0); // Disable BEMF
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MAXIMUM_SPEED, 255);
  cvManager.setCv(CV_MOTOR_TYPE, 0); // Standard DC

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // 1. Trigger kickstart
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);

  // 2. Advance time to end kickstart by timeout
  advance_millis(150);

  // 3. Call update again (as loop() would do)
  motor.setSpeed(7, MM2DirectionState_Forward);

  TEST_ASSERT_FALSE(motor.isKickstarting());
  // Expect PWM for step 7, which is 531
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}

void test_repro_kickstart_only_with_vstart_zero(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_BEMF_CONFIG, 0); // Disable BEMF
  cvManager.setCv(CV_START_VOLTAGE, 0);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MAXIMUM_SPEED, 0);
  cvManager.setCv(CV_MOTOR_TYPE, 0); // Standard DC

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // 1. Trigger kickstart
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);

  // 2. Advance time to end kickstart by timeout
  advance_millis(150);

  // 3. Call update again (as loop() would do)
  motor.setSpeed(7, MM2DirectionState_Forward);

  TEST_ASSERT_FALSE(motor.isKickstarting());
  // vStart=1, vHigh=1023 -> vMid=512. map(7, 1, 7, 1, 512) = 512
  TEST_ASSERT_EQUAL(512, analog_write_values[10]);
}
