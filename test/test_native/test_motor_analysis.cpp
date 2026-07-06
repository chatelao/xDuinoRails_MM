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
