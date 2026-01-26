#include <Arduino.h>
#include <unity.h>
#include "ProtocolHandler.h"

// Define dummy pins since we are not testing the hardware interaction
#define DCC_MM_SIGNAL_PIN 0
#define ADDRESS 1

ProtocolHandler protocolHandler(DCC_MM_SIGNAL_PIN);

void setUp(void) {
    // set stuff up here
    protocolHandler.setup();
    protocolHandler.setAddress(ADDRESS);
}

void tearDown(void) {
    // clean stuff up here
}

void test_initial_state(void) {
    TEST_ASSERT_EQUAL(1, protocolHandler.getAddress());
}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state);
    return UNITY_END();
}
