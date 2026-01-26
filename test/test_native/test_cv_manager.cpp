#include <unity.h>
#include "CvManager.h"
#include "RP2040.h"

// Mock implementation for RP2040 reboot
bool reboot_called = false;
RP2040 rp2040;
void RP2040::reboot() {
    reboot_called = true;
}

void test_cv_manager_get_set() {
  CvManager cvManager;
  cvManager.setup();
  cvManager.setCv(10, 42);
  TEST_ASSERT_EQUAL(42, cvManager.getCv(10));
}

void test_cv_manager_defaults() {
  CvManager cvManager;
  cvManager.setup();
  TEST_ASSERT_EQUAL(3, cvManager.getCv(CV_BASE_ADDRESS));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_START_VOLTAGE));
  TEST_ASSERT_EQUAL(5, cvManager.getCv(CV_ACCELERATION));
  TEST_ASSERT_EQUAL(5, cvManager.getCv(CV_BRAKING_TIME));
  TEST_ASSERT_EQUAL(0, cvManager.getCv(CV_MAXIMUM_SPEED));
  TEST_ASSERT_EQUAL(10, cvManager.getCv(CV_VERSION));
  TEST_ASSERT_EQUAL(13, cvManager.getCv(CV_MANUFACTURER_ID));
  TEST_ASSERT_EQUAL(192, cvManager.getCv(CV_LONG_ADDRESS_HIGH));
  TEST_ASSERT_EQUAL(100, cvManager.getCv(CV_LONG_ADDRESS_LOW));
  TEST_ASSERT_EQUAL(6, cvManager.getCv(CV_CONFIGURATION));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_FRONT_LIGHT_F0F));
  TEST_ASSERT_EQUAL(2, cvManager.getCv(CV_REAR_LIGHT_F0R));
  TEST_ASSERT_EQUAL(1, cvManager.getCv(CV_EXT_ID_HIGH));
  TEST_ASSERT_EQUAL(10, cvManager.getCv(CV_EXT_ID_LOW));
}

void test_cv_manager_special() {
  CvManager cvManager;
  cvManager.setup();

  // Test read-only CV
  uint8_t version = cvManager.getCv(CV_VERSION);
  cvManager.setCv(CV_VERSION, version + 1);
  TEST_ASSERT_EQUAL(version, cvManager.getCv(CV_VERSION));

  // Test reset CV
  reboot_called = false;
  cvManager.setCv(CV_MANUFACTURER_ID, 0);
  TEST_ASSERT_TRUE(reboot_called);
}

void setUp(void) {
  reboot_called = false;
}

void tearDown(void) {
  // clean stuff up here
}
