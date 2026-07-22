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

void test_is_bemf_enabled_compile_flags(void) {
    CvManagerMock cvManager;
    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();

    // Check behavior depending on what macros are defined during build
#if defined(FORCE_OPEN_LOOP) || defined(OPEN_LOOP)
    cvManager.setCv(CV_BEMF_CONFIG, 1);
    TEST_ASSERT_FALSE(motor.isBemfEnabled());
    cvManager.setCv(CV_BEMF_CONFIG, 0);
    TEST_ASSERT_FALSE(motor.isBemfEnabled());
#elif defined(FORCE_CLOSED_LOOP) || defined(CLOSED_LOOP)
    cvManager.setCv(CV_BEMF_CONFIG, 1);
    TEST_ASSERT_TRUE(motor.isBemfEnabled());
    cvManager.setCv(CV_BEMF_CONFIG, 0);
    TEST_ASSERT_TRUE(motor.isBemfEnabled());
#else
    // Default fallback case (reading from CV 49)
    cvManager.setCv(CV_BEMF_CONFIG, 1);
    TEST_ASSERT_TRUE(motor.isBemfEnabled());
    cvManager.setCv(CV_BEMF_CONFIG, 0);
    TEST_ASSERT_FALSE(motor.isBemfEnabled());
#endif
}
