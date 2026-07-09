#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>

extern std::map<uint8_t, int> analog_write_values;
extern std::map<uint8_t, int> analog_read_values;
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

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
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
  // CV_START_VOLTAGE = 10 (Vstart = 40)
  // CV_MAXIMUM_SPEED = 0 (Vhigh = 1023)
  // CV_MEDIUM_SPEED = 0 (Vmid = 531)
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MAXIMUM_SPEED, 0);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MOTOR_TYPE, 0);
  cvManager.setCv(CV_BEMF_CONFIG, 0); // Disable BEMF

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
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

void test_motor_bemf_pi_control(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);  // PWM 40
  cvManager.setCv(CV_MAXIMUM_SPEED, 255); // PWM 1023
  cvManager.setCv(CV_MEDIUM_SPEED, 0);    // Midpoint PWM 531
  cvManager.setCv(CV_BEMF_CONFIG, 1);     // Enable BEMF
  cvManager.setCv(CV_BEMF_K, 32);         // K=32 -> factor 2
  cvManager.setCv(CV_BEMF_I, 64);         // I=64 -> factor 1
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Set speed to step 7 (targetPwm = 531)
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Pass kickstart

  // Warm up filter (3 samples)
  analog_read_values[2] = 2000;
  analog_read_values[3] = 0;
  for (int i = 0; i < 3; i++) {
    advance_millis(20);
    motor.setSpeed(7, MM2DirectionState_Forward);
  }

  // targetBEMF = 531 * 4 = 2124
  // filteredBEMF = 2000
  // error = 2124 - 2000 = 124
  // bemfErrorSum = 124 (after warmup) + 124 + 124 + 124 = 496
  // Wait, bemfErrorSum accumulates every update.
  // Let's reset the sum for predictable testing
  // Actually we can't reset it from here. Let's calculate carefully.

  // Warmup step 1: bemfErrorSum = 124. adj = (124*2) + (124*1) = 372. finalPwm = 531+372=903.
  // Warmup step 2: bemfErrorSum = 248. adj = 248 + 248 = 496. finalPwm = 531+496=1027->1023.
  // Warmup step 3: bemfErrorSum = 372. adj = 248 + 372 = 620. finalPwm = 1023.

  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // Now simulate overspeed (e.g. going downhill)
  analog_read_values[2] = 2500;
  // Step 1: history {2000, 2000, 2500} -> median 2000. Error remains 124. bemfErrorSum = 496. adj = 248 + 496 = 744. finalPwm = 1023.
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // Step 2: history {2000, 2500, 2500} -> median 2500.
  // targetBEMF = 2124, filteredBEMF = 2500 -> error = -376
  // bemfErrorSum = 496 - 376 = 120
  // adjustment = (-376 * 2) + (120 * 1) = -752 + 120 = -632
  // finalPwm = 531 - 632 = -101 -> clamped to 0
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);
}
