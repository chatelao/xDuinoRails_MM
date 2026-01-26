#include <Arduino.h>
#include <unity.h>
#include "CvManager.h"

CvManager cvManager;

void setUp(void) {
    // set stuff up here
    cvManager.setup();
}

void tearDown(void) {
    // clean stuff up here
}

void test_cv_initial_values(void) {
    TEST_ASSERT_EQUAL(1, cvManager.getCv(1)); // Default address
    TEST_ASSERT_EQUAL(0, cvManager.getCv(29)); // Default configuration
}

void test_cv_read_write(void) {
    cvManager.setCv(1, 80);
    TEST_ASSERT_EQUAL(80, cvManager.getCv(1));

    cvManager.setCv(29, 2);
    TEST_ASSERT_EQUAL(2, cvManager.getCv(29));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_cv_initial_values);
    RUN_TEST(test_cv_read_write);
}

void loop() {
    UNITY_END();
}
