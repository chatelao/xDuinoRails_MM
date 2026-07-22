#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>
#include <map>

extern std::map<uint8_t, int> analog_write_values;
extern void advance_millis(unsigned long ms);

void test_repro_watchdog_stop_no_kickstart(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_BEMF_CONFIG, 0);
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // 1. Start moving
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Finish kickstart
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // 2. Watchdog triggers stop()
  motor.stop();
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);

  // 3. Signal recovered, try to move again
  motor.setSpeed(7, MM2DirectionState_Forward);

  // FIX: It SHOULD kickstart because it was physically stopped.
  TEST_ASSERT_TRUE(motor.isKickstarting());
}

void test_repro_start_backward_kickstart_wrong_dir(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_MOTOR_TYPE, 0); // KICK_PWM = 800

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Start moving Backward from standstill
  motor.setSpeed(7, MM2DirectionState_Backward);

  // FIX: It SHOULD kickstart in Backward direction (Pin 11)
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[11]);
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);
}

#if !defined(OPEN_LOOP) && !defined(FORCE_OPEN_LOOP)
void test_repro_bemf_disabled_leftover_adjustment(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MEDIUM_SPEED, 0);
  cvManager.setCv(CV_MAXIMUM_SPEED, 255);
  cvManager.setCv(CV_BEMF_CONFIG, 1);     // Enable BEMF initially
  cvManager.setCv(CV_BEMF_K, 32);
  cvManager.setCv(CV_BEMF_I, 0);
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Set speed to step 7 (targetPwm = 531)
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Pass kickstart

  // targetBEMF = 531 * 4 = 2124
  // Simulate currentBEMF = 2000
  // error = 124
  // adjustment = (124 * 32 / 16) = 248
  analog_read_values[2] = 2000;
  analog_read_values[3] = 0;
  advance_millis(20);
  motor.setSpeed(7, MM2DirectionState_Forward);

  // finalPwm = 531 + 248 = 779
  TEST_ASSERT_EQUAL(779, analog_write_values[10]);

  // Now disable BEMF via CV
  cvManager.setCv(CV_BEMF_CONFIG, 0);

  // Update again
  motor.setSpeed(7, MM2DirectionState_Forward);

  // FIX: adjustment should now be 0, so PWM should be 531
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}
#endif

void test_repro_direction_change_kickstart(void) {
  CvManagerMock cvManager;
  cvManager.setCv(CV_BEMF_CONFIG, 0);
  cvManager.setCv(CV_START_VOLTAGE, 10);
  cvManager.setCv(CV_MOTOR_TYPE, 0);

  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Start moving Forward
  motor.setSpeed(7, MM2DirectionState_Forward);
  advance_millis(150); // Finish kickstart
  motor.setSpeed(7, MM2DirectionState_Forward);
  TEST_ASSERT_EQUAL(531, analog_write_values[10]);

  // Change direction to Backward while moving
  motor.setSpeed(7, MM2DirectionState_Backward);

  // Should stop first
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);
  TEST_ASSERT_EQUAL(0, analog_write_values[11]);

  // Next call should apply power in new direction AND kickstart
  motor.setSpeed(7, MM2DirectionState_Backward);

  // FIX: It SHOULD kickstart here because it just changed direction
  TEST_ASSERT_TRUE(motor.isKickstarting());
  TEST_ASSERT_EQUAL(800, analog_write_values[11]);
}
