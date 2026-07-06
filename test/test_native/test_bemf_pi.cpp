#include "CvManagerMock.h"
#include "MotorControl.h"
#include <unity.h>
#include <map>

extern std::map<uint8_t, int> analog_write_values;
extern std::map<uint8_t, int> analog_read_values;
extern void advance_millis(unsigned long ms);
extern void reset_arduino_mock();

void test_bemf_pi_compensation(void) {
  reset_arduino_mock();
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MAXIMUM_SPEED, 0); // Vhigh = 1023
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_BEMF_CONFIG, 1); // Enable BEMF
  cvManager.setCv(CV_BEMF_K, 32);     // K = 2
  cvManager.setCv(CV_BEMF_I, 64);     // I = 1
  cvManager.setCv(CV_MOTOR_TYPE, 0);  // BEMF_SAMPLE_INT = 12

  MotorControl motor(cvManager, 7, 8, 0, 1);
  motor.setup();

  // Speed Step 7 -> PWM 531
  // Start motor
  motor.setSpeed(7, MM2DirectionState_Forward);

  // Finish kickstart (timeout 100ms)
  advance_millis(150);
  // Ensure we are past BEMF sample interval too
  advance_millis(20);

  // Update to get into non-kickstarting mode with fresh BEMF sampling
  // Force a measurement that exactly matches target to avoid adjustment
  analog_read_values[0] = 531 * 4;
  analog_read_values[1] = 0;

  motor.setSpeed(7, MM2DirectionState_Forward);

  int basePwm = analog_write_values[7];
  TEST_ASSERT_EQUAL(531, basePwm);

  // Target BEMF = 531 * 4 = 2124

  // Simulate load: current BEMF drops to 1000
  analog_read_values[0] = 1000;
  analog_read_values[1] = 0;

  advance_millis(15); // trigger sample
  motor.setSpeed(7, MM2DirectionState_Forward);

  // error = 2124 - 1000 = 1124
  // bemfErrorSum = 1124
  // adjustment = (1124 * 32) / 16 + (1124 * 64) / 64 = 2248 + 1124 = 3372
  // finalPwm = 531 + 3372 = 3903 -> clamped to 1023

  TEST_ASSERT_EQUAL(1023, analog_write_values[7]);

  // Simulate overspeed: current BEMF rises to 3000
  analog_read_values[0] = 3000;
  advance_millis(15);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // targetBEMF = 2124
  // error = 2124 - 3000 = -876
  // bemfErrorSum = 1124 - 876 = 248
  // adjustment = (-876 * 32) / 16 + (248 * 64) / 64 = -1752 + 248 = -1504
  // finalPwm = 531 - 1504 = -973 -> clamped to 0

  TEST_ASSERT_EQUAL(0, analog_write_values[7]);
}
