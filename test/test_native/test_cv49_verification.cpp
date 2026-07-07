#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>
#include <map>

extern std::map<uint8_t, int> analog_write_values;
extern void advance_millis(unsigned long ms);

void test_cv49_zero_only_kickstart_works(void) {
  CvManagerMock cvManager;

  // Factory reset defaults
  cvManager.setCv(CV_START_VOLTAGE, 10); // vStart = 40
  cvManager.setCv(CV_MAXIMUM_SPEED, 0);  // vHigh = 1023
  cvManager.setCv(CV_MEDIUM_SPEED, 0);   // vMid = 531
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  // Disable BEMF
  cvManager.setCv(CV_BEMF_CONFIG, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // 1. Start moving
  motor.setSpeed(7, MM2DirectionState_Forward);

  // Should be kickstarting
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);

  // 2. Loop during kickstart
  advance_millis(50);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);

  // 3. Kickstart ends via timeout
  advance_millis(100); // total 150ms > 100ms
  motor.setSpeed(7, MM2DirectionState_Forward);

  TEST_ASSERT_FALSE(motor.isKickstarting());
  // HERE IS THE CRITICAL PART: PWM should now be 531
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}
