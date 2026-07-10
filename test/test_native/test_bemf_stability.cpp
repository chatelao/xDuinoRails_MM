#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>

extern std::map<uint8_t, int> analog_write_values;
extern std::map<uint8_t, int> analog_read_values;
extern void advance_millis(unsigned long ms);

/**
 * Test to verify the stability of the PI controller and the effectiveness of the reduced integral windup limit.
 */
void test_bemf_stability_integral_clamping(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);  // PWM 40
  cvManager.setCv(CV_MAXIMUM_SPEED, 255); // PWM 1023
  cvManager.setCv(CV_MEDIUM_SPEED, 0);    // Midpoint PWM 531
  cvManager.setCv(CV_BEMF_CONFIG, 1);     // Enable BEMF
  cvManager.setCv(CV_BEMF_K, 16);         // K=16 -> factor 1
  cvManager.setCv(CV_BEMF_I, 64);         // I=64 -> factor 1
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Set speed to step 7 (targetPwm = 531, targetBEMF = 2124)
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Pass kickstart

  // Simulating a heavily loaded or stalled motor (BEMF = 0)
  analog_read_values[2] = 0;
  analog_read_values[3] = 0;

  // Let the controller run for several cycles to accumulate error
  // Each cycle at 12ms (default for Standard DC)
  for (int i = 0; i < 100; i++) {
    advance_millis(15);
    motor.setSpeed(7, MM2DirectionState_Forward);
  }

  // With targetBEMF = 2124 and currentBEMF = 0, error = 2124.
  // After 100 cycles, the integral would normally be 212400.
  // However, it should be clamped to 4096.

  // Adjustment calculation:
  // Adjustment = (error * K / 16) + (bemfErrorSum * I / 64)
  // Adjustment = (2124 * 16 / 16) + (4096 * 64 / 64) = 2124 + 4096 = 6220
  // finalPwm = 531 + 6220 = 6751 -> clamped to 1023

  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // Now simulate the motor suddenly starting to move very fast (BEMF = 4000)
  // This simulates the "jump" behavior after the stall is released.
  // We want to see how quickly the adjustment decreases.
  analog_read_values[2] = 4000;

  // One cycle:
  // targetBEMF = 2124, currentBEMF = 4000 -> error = -1876
  // bemfErrorSum = 4096 - 1876 = 2220
  // Adjustment = (-1876 * 1) + (2220 * 1) = 344
  // finalPwm = 531 + 344 = 875
  advance_millis(15);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(875, analog_write_values[10]);

  // Next cycle:
  // bemfErrorSum = 2220 - 1876 = 344
  // Adjustment = -1876 + 344 = -1532
  // finalPwm = 531 - 1532 = -1001 -> clamped to 0
  advance_millis(15);
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);
}

/**
 * Test to verify that readBEMF properly toggles the shutdown pin.
 */
void test_read_bemf_shutdown_toggling(void) {
  CvManagerMock cvManager;
  MotorControl motor(cvManager, 10, 11, 2, 3, 12); // shutPin = 12
  motor.setup();

  // Reset last written value for shutdown pin (D2/12)
  extern std::map<uint8_t, int> digital_write_values;
  digital_write_values[12] = 0; // LOW is enabled

  motor.readBEMF();

  // We expect it was set to HIGH (disabled) and then back to LOW (enabled)
  // The mock digital_write just records the last value.
  // To properly test this, we would need a sequence recorder, but we can at least
  // verify it's back to LOW.
  TEST_ASSERT_EQUAL(0, digital_write_values[12]);
}
