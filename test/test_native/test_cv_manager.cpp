#include <Arduino.h>
#include <unity.h>
#include "CvManager.h"
#include "mocks/EepromMock.h"

EepromMock eepromMock;
CvManager cvManager(eepromMock);

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_cv_manager_get_set(void) {
    cvManager.setCv(1, 123);
    TEST_ASSERT_EQUAL(123, cvManager.getCv(1));
}

void test_cv_manager_init(void) {
    cvManager.setup(512);
    TEST_ASSERT_EQUAL(24, cvManager.getCv(1));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_cv_manager_get_set);
    RUN_TEST(test_cv_manager_init);
    return UNITY_END();
}
