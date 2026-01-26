#include <Arduino.h>
#include <unity.h>
#include "MotorControl.h"
#include "IMotorHardware.h"

class MockMotorHardware : public IMotorHardware {
public:
    int lastPwm;
    MM2DirectionState lastDir;
    int writeMotorCount;
    int bemfValue;

    MockMotorHardware() : lastPwm(0), lastDir(MM2DirectionState_Forward), writeMotorCount(0), bemfValue(0) {}

    void writeMotor(int pwm, MM2DirectionState dir) override {
        lastPwm = pwm;
        lastDir = dir;
        writeMotorCount++;
    }

    int readBemf() override {
        return bemfValue;
    }
};

const int MOTOR_TYPE = 1; // HLA

MockMotorHardware mockHardware;
MotorControl motor(MOTOR_TYPE, &mockHardware);

void setUp(void) {
    mockHardware = MockMotorHardware();
    motor.setup();
}

void tearDown(void) {
}

void test_motor_stop(void) {
    motor.stop();
    TEST_ASSERT_EQUAL(0, mockHardware.lastPwm);
    TEST_ASSERT_EQUAL(1, mockHardware.writeMotorCount);
}

void test_motor_update(void) {
    motor.update(500, MM2DirectionState_Forward);
    TEST_ASSERT_EQUAL(500, mockHardware.lastPwm);
    TEST_ASSERT_EQUAL(MM2DirectionState_Forward, mockHardware.lastDir);
    TEST_ASSERT_EQUAL(1, mockHardware.writeMotorCount);
}

void test_motor_kickstart(void) {
    motor.update(500, MM2DirectionState_Forward);
    TEST_ASSERT_TRUE(motor.isKickstarting());
    TEST_ASSERT_EQUAL(1023, mockHardware.lastPwm);
    TEST_ASSERT_EQUAL(1, mockHardware.writeMotorCount);

    delay(200); // Wait for kickstart to finish
    motor.update(500, MM2DirectionState_Forward);
    TEST_ASSERT_FALSE(motor.isKickstarting());
    TEST_ASSERT_EQUAL(500, mockHardware.lastPwm);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_motor_stop);
    RUN_TEST(test_motor_update);
    RUN_TEST(test_motor_kickstart);
}

void loop() {
    UNITY_END();
}
