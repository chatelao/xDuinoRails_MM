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
