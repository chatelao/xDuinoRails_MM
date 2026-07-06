#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>

extern std::map<uint8_t, int> analog_write_values;
extern void advance_millis(unsigned long ms);

void test_motor_pwm_mapping_detailed(void) {
  CvManagerMock cvManager;
  // Use default values as set in CvManager::initCv()
  // CV_START_VOLTAGE = 10
  // CV_MAXIMUM_SPEED = 0 (defaults to 1023 PWM)
  // CV_MEDIUM_SPEED = 0 (defaults to midpoint)
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MAXIMUM_SPEED, 0);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MOTOR_TYPE, 0);
  cvManager.setCv(CV_BEMF_CONFIG, 0); // Disable BEMF

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Helper to get PWM after kickstart
  auto get_pwm = [&](int step) {
    motor.setSpeed(step, MM2DirectionState_Forward);
    advance_millis(150); // Ensure kickstart is over
    motor.setSpeed(step, MM2DirectionState_Forward);
    return analog_write_values[10];
  };

  TEST_ASSERT_EQUAL(0, get_pwm(0));
  TEST_ASSERT_EQUAL(40, get_pwm(1));
  TEST_ASSERT_EQUAL(121, get_pwm(2));
  TEST_ASSERT_EQUAL(203, get_pwm(3));
  TEST_ASSERT_EQUAL(285, get_pwm(4));
  TEST_ASSERT_EQUAL(367, get_pwm(5));
  TEST_ASSERT_EQUAL(449, get_pwm(6));
  TEST_ASSERT_EQUAL(531, get_pwm(7));
  TEST_ASSERT_EQUAL(601, get_pwm(8));
  TEST_ASSERT_EQUAL(671, get_pwm(9));
  TEST_ASSERT_EQUAL(741, get_pwm(10));
  TEST_ASSERT_EQUAL(812, get_pwm(11));
  TEST_ASSERT_EQUAL(882, get_pwm(12));
  TEST_ASSERT_EQUAL(952, get_pwm(13));
  TEST_ASSERT_EQUAL(1023, get_pwm(14));
}

void test_motor_pwm_mapping_new_defaults(void) {
  CvManagerMock cvManager;
  // Use new default values:
  // CV_START_VOLTAGE = 85 (Vstart = 341)
  // CV_MAXIMUM_SPEED = 140 (Vhigh = 561)
  // CV_MEDIUM_SPEED = 105 (Vmid = 421)
  cvManager.setCv(CV_START_VOLTAGE, 85);
  cvManager.setCv(CV_MAXIMUM_SPEED, 140);
  cvManager.setCv(CV_MEDIUM_SPEED, 105);
  cvManager.setCv(CV_MOTOR_TYPE, 0);
  cvManager.setCv(CV_BEMF_CONFIG, 0); // Disable BEMF

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Helper to get PWM after kickstart
  auto get_pwm = [&](int step) {
    motor.setSpeed(step, MM2DirectionState_Forward);
    advance_millis(150); // Ensure kickstart is over
    motor.setSpeed(step, MM2DirectionState_Forward);
    return analog_write_values[10];
  };

  TEST_ASSERT_EQUAL(0, get_pwm(0));
  TEST_ASSERT_EQUAL(341, get_pwm(1));
  TEST_ASSERT_EQUAL(354, get_pwm(2));
  TEST_ASSERT_EQUAL(367, get_pwm(3));
  TEST_ASSERT_EQUAL(381, get_pwm(4));
  TEST_ASSERT_EQUAL(394, get_pwm(5));
  TEST_ASSERT_EQUAL(407, get_pwm(6));
  TEST_ASSERT_EQUAL(421, get_pwm(7));
  TEST_ASSERT_EQUAL(441, get_pwm(8));
  TEST_ASSERT_EQUAL(461, get_pwm(9));
  TEST_ASSERT_EQUAL(481, get_pwm(10));
  TEST_ASSERT_EQUAL(501, get_pwm(11));
  TEST_ASSERT_EQUAL(521, get_pwm(12));
  TEST_ASSERT_EQUAL(541, get_pwm(13));
  TEST_ASSERT_EQUAL(561, get_pwm(14));
}

void test_motor_bemf_pi_control(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);  // PWM 40
  cvManager.setCv(CV_MAXIMUM_SPEED, 255); // PWM 1023
  cvManager.setCv(CV_MEDIUM_SPEED, 0);    // Midpoint PWM 531
  cvManager.setCv(CV_BEMF_CONFIG, 1);     // Enable BEMF
  cvManager.setCv(CV_BEMF_K, 32);         // K=32 -> factor 2
  cvManager.setCv(CV_BEMF_I, 64);         // I=64 -> factor 1
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Set speed to step 7 (targetPwm = 531)
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Pass kickstart

  // targetBEMF = 531 * 4 = 2124
  // Simulate currentBEMF = 2000 (Load detected, engine slow)
  // error = 2124 - 2000 = 124
  // bemfErrorSum = 124
  // adjustment = (124 * 32 / 16) + (124 * 64 / 64) = 248 + 124 = 372
  analog_read_values[2] = 2000;
  analog_read_values[3] = 0;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // finalPwm = 531 + 372 = 903
  TEST_ASSERT_EQUAL(903, analog_write_values[10]);

  // Second update with same BEMF
  // error = 124
  // bemfErrorSum = 124 + 124 = 248
  // adjustment = (124 * 32 / 16) + (248 * 64 / 64) = 248 + 248 = 496
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // finalPwm = 531 + 496 = 1027 (clamped to 1023)
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // Now simulate overspeed (e.g. going downhill)
  // currentBEMF = 2500
  // targetBEMF = 2124
  // error = 2124 - 2500 = -376
  // bemfErrorSum = 248 - 376 = -128
  // adjustment = (-376 * 32 / 16) + (-128 * 64 / 64) = -752 - 128 = -880
  analog_read_values[2] = 2500;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // finalPwm = 531 - 880 = -349 (clamped to 0)
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);
}
