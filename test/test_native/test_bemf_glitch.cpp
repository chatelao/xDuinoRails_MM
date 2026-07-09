#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>
#include <map>
#include <vector>

extern std::map<uint8_t, int> analog_write_values;
extern std::map<uint8_t, int> analog_read_values;
extern void advance_millis(unsigned long ms);

void test_bemf_collector_gap_glitch(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);  // PWM 40
  cvManager.setCv(CV_MAXIMUM_SPEED, 255); // PWM 1023
  cvManager.setCv(CV_MEDIUM_SPEED, 0);    // Midpoint PWM 531
  cvManager.setCv(CV_BEMF_CONFIG, 1);     // Enable BEMF
  cvManager.setCv(CV_BEMF_K, 16);         // K=16 -> factor 1
  cvManager.setCv(CV_BEMF_I, 0);          // Disable I for pure glitch analysis
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Set speed to step 7 (targetPwm = 531)
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Pass kickstart

  // Warm up filter (3 samples)
  analog_read_values[2] = 2124;
  analog_read_values[3] = 0;
  for (int i = 0; i < 3; i++) {
    advance_millis(20);
    motor.setSpeed(7, MM2DirectionState_Forward);
  }

  // No adjustment expected now that filter is warm
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // Collector gap glitch: BEMF drops to 0 for ONE sample
  analog_read_values[2] = 0;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // Median of {2124, 2124, 0} is 2124.
  // The glitch should be IGNORED.
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // Recovery: BEMF returns to normal
  analog_read_values[2] = 2124;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}

void test_bemf_collector_gap_persistent_failure(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MAXIMUM_SPEED, 255);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_BEMF_CONFIG, 1);
  cvManager.setCv(CV_BEMF_K, 16); // K=1
  cvManager.setCv(CV_BEMF_I, 0);
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150);

  // Warm up
  analog_read_values[2] = 2124;
  for (int i = 0; i < 3; i++) {
    advance_millis(20);
    motor.setSpeed(7, MM2DirectionState_Forward);
  }

  // Persistent drop to 0 (more than 1 sample)
  analog_read_values[2] = 0;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward); // History: {2124, 2124, 0} -> Median 2124
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward); // History: {2124, 0, 0} -> Median 0
  // Now it should react
  // targetBEMF = 2124, filteredBEMF = 0 -> adjustment = 2124
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);
}

void test_bemf_collector_gap_with_integral_filtered(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MAXIMUM_SPEED, 255);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_BEMF_CONFIG, 1);
  cvManager.setCv(CV_BEMF_K, 16); // K=1
  cvManager.setCv(CV_BEMF_I, 16); // I=16 -> factor 0.25
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150);

  // Warm up
  analog_read_values[2] = 2124;
  for (int i = 0; i < 3; i++) {
    advance_millis(20);
    motor.setSpeed(7, MM2DirectionState_Forward);
  }

  // Stable
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // Single sample glitch
  analog_read_values[2] = 0;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // Filtered BEMF should still be 2124. No integral error should be added.
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // Recovery
  analog_read_values[2] = 2124;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}
