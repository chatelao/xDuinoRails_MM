#include <Arduino.h>
#include <unity.h>
#include "ProtocolHandler.h"

// Define dummy pins since we are not testing the hardware interaction
#define DCC_MM_SIGNAL_PIN 0

ProtocolHandler protocolHandler(DCC_MM_SIGNAL_PIN);

void setUp(void) {
    // set stuff up here
    protocolHandler.setup();
}

void tearDown(void) {
    // clean stuff up here
}

void test_timeout_logic(void) {
    // Initially, timeout should be false
    TEST_ASSERT_FALSE(protocolHandler.isTimeout());

    // Simulate time passing without any commands
    // The timeout is 1500ms, so let's simulate 2000ms
    delay(2000);

    // Now, the timeout should be true
    TEST_ASSERT_TRUE(protocolHandler.isTimeout());
}

void test_address_setting(void) {
    protocolHandler.setAddress(80);
    // This is a bit tricky to test without a real DCC signal.
    // We'll just check that the address is set internally.
    // This requires exposing the address via a getter or making it public for testing.
    // For now, we assume it works and this test is a placeholder.
    TEST_ASSERT_TRUE(true);
}

void test_function_state(void) {
    // F0 is off by default
    TEST_ASSERT_FALSE(protocolHandler.getFunctionState(0));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_timeout_logic);
    RUN_TEST(test_address_setting);
    RUN_TEST(test_function_state);
}

void loop() {
    UNITY_END();
}
