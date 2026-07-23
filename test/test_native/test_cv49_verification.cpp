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

void test_open_loop_start_voltage_active(void) {
    CvManagerMock cvManager;
    cvManager.setCv(CV_MEDIUM_SPEED, 0);
    cvManager.setCv(CV_MAXIMUM_SPEED, 0);
    cvManager.setCv(CV_BEMF_CONFIG, 0); // Open-loop mode at runtime (or compile-time anyway)
    cvManager.setCv(CV_MOTOR_TYPE, 0);

    // Test with CV_START_VOLTAGE = 20
    cvManager.setCv(CV_START_VOLTAGE, 20);
    {
        MotorControl motor(cvManager, 10, 11, 2, 3, 12);
        motor.setup();

        // Speed step 1 -> trigger kickstart first
        motor.setSpeed(1, MM2DirectionState_Forward);
        TEST_ASSERT_TRUE(motor.isKickstarting());

        // Wait past kickstart timeout
        advance_millis(150);
        motor.setSpeed(1, MM2DirectionState_Forward);
        TEST_ASSERT_FALSE(motor.isKickstarting());

        // Check PWM is vStart (which maps 20 -> 80)
        TEST_ASSERT_EQUAL(80, analog_write_values[10]);
    }

    // Reset mocks for next sub-test
    reset_arduino_mock();

    // Test with CV_START_VOLTAGE = 50
    cvManager.setCv(CV_START_VOLTAGE, 50);
    {
        MotorControl motor(cvManager, 10, 11, 2, 3, 12);
        motor.setup();

        // Speed step 1 -> trigger kickstart first
        motor.setSpeed(1, MM2DirectionState_Forward);
        TEST_ASSERT_TRUE(motor.isKickstarting());

        // Wait past kickstart timeout
        advance_millis(150);
        motor.setSpeed(1, MM2DirectionState_Forward);
        TEST_ASSERT_FALSE(motor.isKickstarting());

        // Check PWM is vStart (which maps 50 -> 200)
        TEST_ASSERT_EQUAL(200, analog_write_values[10]);
    }
}

#if defined(OPEN_LOOP) || defined(FORCE_OPEN_LOOP)
void test_open_loop_kickstart_ignores_high_bemf(void) {
    CvManagerMock cvManager;
    cvManager.setCv(CV_START_VOLTAGE, 10);
    cvManager.setCv(CV_MOTOR_TYPE, 0); // standard DC: kickstart = 100ms
    cvManager.setCv(CV_BEMF_CONFIG, 1); // Enable BEMF via CV

    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();

    // Start kickstart
    motor.setSpeed(7, MM2DirectionState_Forward);
    TEST_ASSERT_TRUE(motor.isKickstarting());

    // Inject high BEMF
    analog_read_values[2] = 500;
    analog_read_values[3] = 0;

    // Call update, kickstart must still be active because compiles as OPEN_LOOP
    advance_millis(20);
    motor.setSpeed(7, MM2DirectionState_Forward);
    TEST_ASSERT_TRUE(motor.isKickstarting());

    // Timeout
    advance_millis(100);
    motor.setSpeed(7, MM2DirectionState_Forward);
    TEST_ASSERT_FALSE(motor.isKickstarting());
}

void test_open_loop_adjustments_always_zero(void) {
    CvManagerMock cvManager;
    cvManager.setCv(CV_START_VOLTAGE, 10);
    cvManager.setCv(CV_MOTOR_TYPE, 0);
    cvManager.setCv(CV_BEMF_CONFIG, 1); // Enable BEMF via CV
    cvManager.setCv(CV_BEMF_K, 64);
    cvManager.setCv(CV_BEMF_I, 64);

    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();

    // Set speed step 7 (targetPwm = 531)
    motor.setSpeed(7, MM2DirectionState_Forward);
    advance_millis(150); // Pass kickstart

    // Inject massive BEMF error (e.g. stalled motor)
    analog_read_values[2] = 0;
    analog_read_values[3] = 0;

    // Under open loop, this error shouldn't be processed and adjustment must be 0
    advance_millis(20);
    motor.setSpeed(7, MM2DirectionState_Forward);

    // It should be exactly 531 (no PI control adjustment added)
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);
}

void test_open_loop_cv_ignored_in_loop_type(void) {
    CvManagerMock cvManager;
    MotorControl motor(cvManager, 10, 11, 2, 3, 12);
    motor.setup();

    // Try setting BEMF_CONFIG to 1, should still return false
    cvManager.setCv(CV_BEMF_CONFIG, 1);
    TEST_ASSERT_FALSE(motor.isBemfEnabled());

    // Try setting BEMF_CONFIG to 0, should still return false
    cvManager.setCv(CV_BEMF_CONFIG, 0);
    TEST_ASSERT_FALSE(motor.isBemfEnabled());
}
#endif
