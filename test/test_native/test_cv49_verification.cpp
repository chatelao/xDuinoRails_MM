#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>

extern std::map<uint8_t, int> analog_write_values;
extern void advance_millis(unsigned long ms);

void test_cv49_zero_only_kickstart_works(void) {
  CvManagerMock cvManager;
  // Factory reset defaults (simulated)
  cvManager.setCv(CV_START_VOLTAGE, 1);
  cvManager.setCv(CV_MEDIUM_SPEED, 127);
  cvManager.setCv(CV_MAXIMUM_SPEED, 255);
  cvManager.setCv(CV_MOTOR_TYPE, 0); // Standard DC
  cvManager.setCv(CV_BEMF_CONFIG, 0); // Disable BEMF as per issue report

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Reset mocks
  analog_write_values.clear();

  // Set speed to step 7 (targetPwm = 509 approx. with CV 2=1, 5=255, 6=127)
  // Step 1: PWM 4, Step 7: PWM 509, Step 14: PWM 1023
  motor.setSpeed(7, MM2DirectionState_Forward);

  // Should be kickstarting
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]); // KICK_PWM for Standard DC

  // Advance time past kickstart timeout (100ms)
  advance_millis(150);

  // Update motor speed again (to trigger the transition check)
  motor.setSpeed(7, MM2DirectionState_Forward);

  // Kickstart should be over
  TEST_ASSERT_FALSE(motor.isKickstarting());

  // Hardware should now be driven with targetPwm
  // vStart = map(1, 0, 255, 0, 1023) = 4
  // vMid = map(127, 0, 255, 0, 1023) = 509
  // Step 7 -> 509
  TEST_ASSERT_EQUAL(509, analog_write_values[10]);
}

void test_cv49_zero_detailed_simulation(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MAXIMUM_SPEED, 0);
  cvManager.setCv(CV_MOTOR_TYPE, 0); // Standard DC
  cvManager.setCv(CV_BEMF_CONFIG, 0); // Disable BEMF

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Reset mocks
  analog_write_values.clear();

  // 1. Initial call - Step 7 (targetPwm = 531)
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);

  // 2. Subsequent call within kickstart time
  advance_millis(50);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);

  // 3. Call that triggers timeout
  advance_millis(60); // Total 110ms > 100ms
  motor.setSpeed(7, MM2DirectionState_Forward);

  TEST_ASSERT_FALSE(motor.isKickstarting());
  // It should now have transitioned to targetPwm (531)
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // 4. Subsequent calls after kickstart
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_FALSE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}

void test_cv49_zero_with_stop_start(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MAXIMUM_SPEED, 0);
  cvManager.setCv(CV_BEMF_CONFIG, 0);
  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Start
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // Stop
  motor.setSpeed(0, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);
  TEST_ASSERT_FALSE(motor.isKickstarting());

  // Start again
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[10]);
}
