#include "CvManagerMock.h"
#include "MotorControl.h"
#include "Logger.h"
#include "SerialConsole.h"
#include <unity.h>

extern std::map<uint8_t, int> analog_read_values;
extern void advance_millis(unsigned long ms);

void test_bemf_logging_toggle(void) {
    CvManagerMock cvManager;
    ProtocolHandler protocol(0);
    SerialConsole console(&cvManager, &protocol);

    cvManager.setCv(CV_DEBUG_ENABLE, 1);
    logger.begin(&cvManager);

    TEST_ASSERT_TRUE(logger.isBemfLoggingEnabled());

    Serial.pushInput("l b\n");
    console.loop();
    TEST_ASSERT_FALSE(logger.isBemfLoggingEnabled());

    Serial.pushInput("L B\n");
    console.loop();
    TEST_ASSERT_TRUE(logger.isBemfLoggingEnabled());
}

void test_bemf_logging_accumulation(void) {
    CvManagerMock cvManager;
    logger.begin(&cvManager);

    // BEMF logging is ON by default now

    MotorControl motor(cvManager, 10, 11, 2, 3);
    motor.setup();

    // Set speed to trigger some PWM
    motor.setSpeed(7, MM2DirectionState_Forward);

    // Mock BEMF readings
    analog_read_values[2] = 400;
    analog_read_values[3] = 0;

    Serial.clearLog();

    // Step through 1 second
    for(int i=0; i<10; i++) {
        advance_millis(100);
        motor.setSpeed(7, MM2DirectionState_Forward);
    }

    bool foundLog = false;
    for (const auto &line : Serial.logLines) {
        if (line.find("BEMF: Avg") != std::string::npos) {
            foundLog = true;
            // Check if values are present (not zero)
            TEST_ASSERT_TRUE(line.find("Avg 400") != std::string::npos || line.find("Avg 0") == std::string::npos);
            break;
        }
    }
    TEST_ASSERT_TRUE(foundLog);
}
