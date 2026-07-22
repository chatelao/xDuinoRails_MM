#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include <unity.h>
#include <map>
#include <vector>
#include <string>

extern std::map<uint8_t, int> analog_write_values;
extern std::map<uint8_t, int> digital_write_values;
extern void advance_millis(unsigned long ms);

void test_cv49_zero_only_kickstart_works(void) {
    CvManagerMock cvManager;
    // After CV 8 = 8 (Defaults)
    cvManager.setCv(CV_START_VOLTAGE, 10);
    cvManager.setCv(CV_MEDIUM_SPEED, 0);
    cvManager.setCv(CV_MAXIMUM_SPEED, 0);
    cvManager.setCv(CV_BEMF_CONFIG, 1); // Default is 1
    cvManager.setCv(CV_MOTOR_TYPE, 0);   // Default is 0

    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();

    // Set CV 49 = 0
    cvManager.setCv(CV_BEMF_CONFIG, 0);

    // Set speed step 7
    motor.setSpeed(7, MM2DirectionState_Forward);

    // Iteration 1: Kickstart starts
    TEST_ASSERT_TRUE(motor.isKickstarting());
    TEST_ASSERT_EQUAL(800, analog_write_values[10]);

    // Several loops during kickstart
    for(int i=0; i<5; i++) {
        advance_millis(10);
        motor.setSpeed(7, MM2DirectionState_Forward);
        TEST_ASSERT_TRUE(motor.isKickstarting());
        TEST_ASSERT_EQUAL(800, analog_write_values[10]);
    }

    // Total elapsed: 50ms. Kickstart still active.

    // Advance to 101ms
    advance_millis(51);
    motor.setSpeed(7, MM2DirectionState_Forward);

    // Now kickstart should end via timeout
    TEST_ASSERT_FALSE(motor.isKickstarting());

    // Hardware PWM should now be 531 (vMid)
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);

    // Call again to ensure it stays
    advance_millis(10);
    motor.setSpeed(7, MM2DirectionState_Forward);
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}

void test_cv49_zero_with_direction_change(void) {
    CvManagerMock cvManager;
    cvManager.setCv(CV_START_VOLTAGE, 10);
    cvManager.setCv(CV_MEDIUM_SPEED, 0);
    cvManager.setCv(CV_MAXIMUM_SPEED, 0);
    cvManager.setCv(CV_BEMF_CONFIG, 0);
    cvManager.setCv(CV_MOTOR_TYPE, 0);

    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();

    // Start Forward
    motor.setSpeed(7, MM2DirectionState_Forward);
    advance_millis(101);
    motor.setSpeed(7, MM2DirectionState_Forward);
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);

    // Change to Backward
    motor.setSpeed(7, MM2DirectionState_Backward);
    // Should be at 0 now for direction change safety
    TEST_ASSERT_EQUAL(0, analog_write_values[10]);
    TEST_ASSERT_EQUAL(0, analog_write_values[11]);

    // Next call triggers kickstart Backward
    motor.setSpeed(7, MM2DirectionState_Backward);
    TEST_ASSERT_TRUE(motor.isKickstarting());
    TEST_ASSERT_EQUAL(800, analog_write_values[11]);

    // End kickstart
    advance_millis(101);
    motor.setSpeed(7, MM2DirectionState_Backward);
    TEST_ASSERT_FALSE(motor.isKickstarting());
    TEST_ASSERT_EQUAL(531, analog_write_values[11]);
}

void test_cv49_disabled_by_default_open_loop_no_pid(void) {
    CvManagerMock cvManager;
    // BEMF config is 0 by default now.
    cvManager.setCv(CV_START_VOLTAGE, 10);
    cvManager.setCv(CV_MEDIUM_SPEED, 0);
    cvManager.setCv(CV_MAXIMUM_SPEED, 0);
    cvManager.setCv(CV_MOTOR_TYPE, 0);

    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();

    // Verify CV 49 is indeed 0 by default
    TEST_ASSERT_EQUAL(0, cvManager.getCv(CV_BEMF_CONFIG));

    // Set speed step 7
    motor.setSpeed(7, MM2DirectionState_Forward);

    // Pass the kickstart period (100ms for type 0)
    advance_millis(150);
    motor.setSpeed(7, MM2DirectionState_Forward);

    // Hardware PWM should be 531 (Vmid)
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);

    // Now, even if we inject high/low BEMF values via analog reads,
    // since BEMF is disabled, the control loop should NOT perform BEMF readings
    // and there should be no adjustments.
    extern std::map<uint8_t, std::deque<int>> analog_read_sequences;
    analog_read_sequences[2] = {1000, 1000, 1000};
    analog_read_sequences[3] = {0, 0, 0};

    advance_millis(20);
    motor.setSpeed(7, MM2DirectionState_Forward);

    // The output should still be exactly 531, and analog reads on BEMF pins 2 and 3 should not have been consumed
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);
    // The sequence for Pin 2 should still contain 3 items because no analogRead was called!
    TEST_ASSERT_EQUAL(3, analog_read_sequences[2].size());
}
