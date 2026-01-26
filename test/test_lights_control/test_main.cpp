#include <Arduino.h>
#include <unity.h>
#include "LightsControl.h"
#include "ILightsHardware.h"

// Mock implementation of the hardware interface
class MockLightsHardware : public ILightsHardware {
public:
    int lastPin;
    bool lastState;
    int pinSetCount;

    MockLightsHardware() : lastPin(-1), lastState(false), pinSetCount(0) {}

    void setPin(int pin, bool state) override {
        lastPin = pin;
        lastState = state;
        pinSetCount++;
    }
};

const int F0F_PIN = 1;
const int F0B_PIN = 2;

MockLightsHardware mockHardware;
LightsControl lights(F0F_PIN, F0B_PIN, &mockHardware);

void setUp(void) {
    mockHardware = MockLightsHardware();
    lights.setup();
}

void tearDown(void) {
    // clean stuff up here
}

void test_lights_off(void) {
    lights.update(MM2DirectionState_Forward, false);
    TEST_ASSERT_EQUAL(F0B_PIN, mockHardware.lastPin);
    TEST_ASSERT_EQUAL(LOW, mockHardware.lastState);
    TEST_ASSERT_EQUAL(2, mockHardware.pinSetCount);
}

void test_lights_on_forward(void) {
    lights.update(MM2DirectionState_Forward, true);
    TEST_ASSERT_EQUAL(F0B_PIN, mockHardware.lastPin);
    TEST_ASSERT_EQUAL(LOW, mockHardware.lastState);
    TEST_ASSERT_EQUAL(2, mockHardware.pinSetCount);
}

void test_lights_on_backward(void) {
    lights.update(MM2DirectionState_Backward, true);
    TEST_ASSERT_EQUAL(F0B_PIN, mockHardware.lastPin);
    TEST_ASSERT_EQUAL(HIGH, mockHardware.lastState);
    TEST_ASSERT_EQUAL(2, mockHardware.pinSetCount);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_lights_off);
    RUN_TEST(test_lights_on_forward);
    RUN_TEST(test_lights_on_backward);
}

void loop() {
    UNITY_END();
}
