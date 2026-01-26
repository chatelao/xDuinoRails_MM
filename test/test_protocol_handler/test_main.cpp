#ifndef NATIVE_TESTING
#include <Arduino.h>
#include <unity.h>
#include "ProtocolHandler.h"

// Define dummy pins since we are not testing the hardware interaction
#define DCC_MM_SIGNAL_PIN 0
#define ADDRESS 1

ProtocolHandler protocolHandler(DCC_MM_SIGNAL_PIN);

void setUp(void) {
    // set stuff up here
    protocolHandler.setAddress(ADDRESS);
    protocolHandler.setup();
}

void tearDown(void) {
    // clean stuff up here
}

void test_timeout_logic(void) {
    // Initially, timeout should be false
    TEST_-ASSERT_FALSE(protocolHandler.isTimeout());

    // Simulate time passing without any commands
    // The timeout is 1500ms, so let's simulate 2000ms
    delay(2000);

    // Now, the timeout should be true
    TEST_ASSERT_TRUE(protocolHandler.isTimeout());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_timeout_logic);
}

void loop() {
    UNITY_END();
}
#else
// Dummy function for the native environment
int main(int argc, char **argv) { return 0; }
#endif
