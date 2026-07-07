#include "CvManagerMock.h"
#include "MotorControl.h"
#include "RP2040.h"
#include "Logger.h"
#include <unity.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

extern std::map<uint8_t, int> analog_write_values;
extern void advance_millis(unsigned long ms);
extern void reset_arduino_mock();

void test_cv49_zero_only_kickstart_works(void) {
    reset_arduino_mock();
    CvManagerMock cvManager;
    cvManager.setCv(CV_DEBUG_ENABLE, 1);
    logger.begin(&cvManager);

    // User sequence: cv 8 = 8
    cvManager.setCv(CV_MANUFACTURER_ID, 8);
    // In real hardware this reboots and resets all CVs to defaults.
    // In our mock, we should at least ensure CV 49 is at its default (1).
    cvManager.setCv(CV_BEMF_CONFIG, 1);
    cvManager.setCv(CV_START_VOLTAGE, 10);
    cvManager.setCv(CV_MAXIMUM_SPEED, 0); // Default 0 -> PWM 1023
    cvManager.setCv(CV_MEDIUM_SPEED, 0);  // Default 0 -> Midpoint

    // Then: cv 49 = 0
    cvManager.setCv(CV_BEMF_CONFIG, 0);

    MotorControl motor(cvManager, 10, 11, 2, 3);
    motor.setup(); // KICK_MAX_TIME = 100, KICK_PWM = 800

    Serial.clearLog();

    // Step 7, Forward. PWM should be around 511-512
    int targetStep = 7;
    motor.setSpeed(targetStep, MM2DirectionState_Forward);

    // Should be kickstarting
    TEST_ASSERT_TRUE(motor.isKickstarting());
    TEST_ASSERT_EQUAL(800, analog_write_values[10]);

    // Advance 50ms - still kickstarting
    advance_millis(50);
    motor.setSpeed(targetStep, MM2DirectionState_Forward);
    TEST_ASSERT_TRUE(motor.isKickstarting());
    TEST_ASSERT_EQUAL(800, analog_write_values[10]);

    // Advance another 60ms (total 110ms) - kickstart should have timed out
    // Simulate multiple calls
    for (int i = 0; i < 6; i++) {
        advance_millis(10);
        motor.setSpeed(targetStep, MM2DirectionState_Forward);
    }
    TEST_ASSERT_FALSE(motor.isKickstarting());

    // Now it should be the normal PWM
    // CV 2=10 -> 40 PWM
    // CV 5=0 -> 1023 PWM
    // CV 6=0 -> 531 PWM (roughly)
    // Step 7 is vMid -> 531
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);

    // Call it again to make sure it stays there
    advance_millis(10);
    motor.setSpeed(targetStep, MM2DirectionState_Forward);
    TEST_ASSERT_EQUAL(531, analog_write_values[10]);

    // Print all log lines to see if it "pretends"
    for (const auto& line : Serial.logLines) {
        std::cout << "LOG: " << line << std::endl;
    }
}
