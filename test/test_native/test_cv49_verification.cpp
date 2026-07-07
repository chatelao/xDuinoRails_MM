#include "CvManagerMock.h"
#include "MotorControl.h"
#include <unity.h>
#include <map>

extern std::map<uint8_t, int> analog_write_values;
extern void advance_millis(unsigned long ms);

void test_cv49_bemf_disabled_pwm_scaling(void) {
  CvManagerMock cvManager;
  // CV 49 = 0 (BEMF disabled)
  cvManager.setCv(CV_BEMF_CONFIG, 0);

  // Set some CVs for scaling
  // CV 2 = 20 (Vstart -> map(20, 0, 255, 0, 1023) = 80)
  // CV 5 = 200 (Vhigh -> map(200, 0, 255, 0, 1023) = 802)
  // CV 6 = 110 (Vmid -> map(110, 0, 255, 0, 1023) = 441)
  cvManager.setCv(CV_START_VOLTAGE, 20);
  cvManager.setCv(CV_MAXIMUM_SPEED, 200);
  cvManager.setCv(CV_MEDIUM_SPEED, 110);
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  // Helper to get PWM after kickstart
  auto get_pwm = [&](int step) {
    motor.setSpeed(step, MM2DirectionState_Forward);
    advance_millis(150); // Ensure kickstart is over
    motor.setSpeed(step, MM2DirectionState_Forward);
    return analog_write_values[10];
  };

  // Step 0: PWM 0
  TEST_ASSERT_EQUAL(0, get_pwm(0));

  // Step 1: Vstart = 80
  TEST_ASSERT_EQUAL(80, get_pwm(1));

  // Step 7: Vmid = 441
  TEST_ASSERT_EQUAL(441, get_pwm(7));

  // Step 14: Vhigh = 802
  TEST_ASSERT_EQUAL(802, get_pwm(14));

  // Intermediate Step 4: map(4, 1, 7, 80, 441) = 80 + (4-1)*(441-80)/6 = 80 + 3*361/6 = 80 + 180 = 260
  TEST_ASSERT_EQUAL(260, get_pwm(4));

  // Intermediate Step 10: map(10, 7, 14, 441, 802) = 441 + (10-7)*(802-441)/7 = 441 + 3*361/7 = 441 + 154 = 595
  TEST_ASSERT_EQUAL(595, get_pwm(10));
}

void test_cv49_bemf_disabled_default_scaling(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_BEMF_CONFIG, 0); // BEMF disabled

  // Default values
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MAXIMUM_SPEED, 0);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3);
  motor.setup();

  auto get_pwm = [&](int step) {
    motor.setSpeed(step, MM2DirectionState_Forward);
    advance_millis(150);
    motor.setSpeed(step, MM2DirectionState_Forward);
    return analog_write_values[10];
  };

  // vStart = map(10, 0, 255, 0, 1023) = 40
  // vHigh = 0 -> PWM_RANGE = 1023
  // vMid = 0 -> (vStart + vHigh) / 2 = (40 + 1023) / 2 = 531

  TEST_ASSERT_EQUAL(40, get_pwm(1));
  TEST_ASSERT_EQUAL(531, get_pwm(7));
  TEST_ASSERT_EQUAL(1023, get_pwm(14));
}
