#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>
#include <map>
#include <vector>

extern std::map<uint8_t, int> analog_write_values;
extern std::map<uint8_t, int> analog_read_values;
extern void advance_millis(unsigned long ms);

#if !defined(OPEN_LOOP) && !defined(FORCE_OPEN_LOOP)
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

  // No warm up needed anymore
  analog_read_values[2] = 2124;
  analog_read_values[3] = 0;

  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // No adjustment expected
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // Collector gap glitch: BEMF drops to 0 for ONE sample
  analog_read_values[2] = 0;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // Temporal filter is gone, so it SHOULD react immediately.
  // targetBEMF = 2124, currentBEMF = 0 -> adjustment = 2124
  TEST_ASSERT_EQUAL(1023, analog_write_values[10]);

  // Recovery: BEMF returns to normal
  analog_read_values[2] = 2124;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}
#endif
