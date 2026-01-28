#include "CvManager.h"
#include "LightsControl.h"
#include "MotorControl.h"
#include "RP2040.h"
#include "mocks/CvManager.h"
#include "mocks/Arduino.h"
#include <unity.h>

// Mock implementation for RP2040 reboot
bool   reboot_called = false;
RP2040 rp2040;
void   RP2040::reboot() { reboot_called = true; }

// Test CV Manager
void test_cv_manager_get_set(void) {
  CvManager cvManager;
  cvManager.setup();
  cvManager.setCv(10, 42);
  TEST_ASSERT_EQUAL(42, cvManager.getCv(10));
}

void test_cv_manager_defaults(void) {
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

void test_cv_manager_special(void) {
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

// Test Motor Control
void test_motor_control_default_parameters(void) {
  CvManagerMock cvManager;
  MotorControl  motor(cvManager, 0, 1, 2, 3);
  // TODO: Add assertions to check the default motor parameters
}

// Test Lights Control
void test_lights_control_default_behavior(void) {
  CvManagerMock cvManager;
  LightsControl lights(cvManager, 0, 1);
  // TODO: Add assertions to check the default lighting behavior
}

// Test CV Programmer
#include "CvProgrammer.h"
#include "mocks/ProtocolHandler.h"

void test_cv_programmer_enter_programming_mode(void) {
  CvManagerMock       cvManager;
  ProtocolHandlerMock protocolHandler;
  CvProgrammer        cvProgrammer(&cvManager, &protocolHandler);

  cvManager.setCv(CV_PROGRAMMING_LOCK, 7);

  // Simulate 4 direction changes
  mock_millis = 100;
  protocolHandler.setLastChangeDirTs(100);
  cvProgrammer.loop();
  mock_millis = 200;
  protocolHandler.setLastChangeDirTs(200);
  cvProgrammer.loop();
  mock_millis = 300;
  protocolHandler.setLastChangeDirTs(300);
  cvProgrammer.loop();
  mock_millis = 400;
  protocolHandler.setLastChangeDirTs(400);
  cvProgrammer.loop();

  // Set CV address
  mock_millis = 500;
  protocolHandler.setTargetSpeed(10);
  protocolHandler.setLastSpeedChangeTs(500);
  cvProgrammer.loop();

  // Set CV value
  mock_millis = 600;
  protocolHandler.setTargetSpeed(42);
  protocolHandler.setLastSpeedChangeTs(600);
  cvProgrammer.loop();

  TEST_ASSERT_EQUAL(42, cvManager.getCv(10));
}

void test_cv_programmer_programming_lock(void) {
  CvManagerMock       cvManager;
  ProtocolHandlerMock protocolHandler;
  CvProgrammer        cvProgrammer(&cvManager, &protocolHandler);

  cvManager.setCv(CV_PROGRAMMING_LOCK, 0);

  // Simulate 4 direction changes
  mock_millis = 100;
  protocolHandler.setLastChangeDirTs(100);
  cvProgrammer.loop();
  mock_millis = 200;
  protocolHandler.setLastChangeDirTs(200);
  cvProgrammer.loop();
  mock_millis = 300;
  protocolHandler.setLastChangeDirTs(300);
  cvProgrammer.loop();
  mock_millis = 400;
  protocolHandler.setLastChangeDirTs(400);
  cvProgrammer.loop();

  // Set CV address
  mock_millis = 500;
  protocolHandler.setTargetSpeed(10);
  protocolHandler.setLastSpeedChangeTs(500);
  cvProgrammer.loop();

  // Set CV value
  mock_millis = 600;
  protocolHandler.setTargetSpeed(42);
  protocolHandler.setLastSpeedChangeTs(600);
  cvProgrammer.loop();

  TEST_ASSERT_EQUAL(0, cvManager.getCv(10));
}

void setUp(void) {
    reboot_called = false;
    mock_millis = 0;
}

void tearDown(void) {
  // clean stuff up here
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_cv_manager_get_set);
  RUN_TEST(test_cv_manager_defaults);
  RUN_TEST(test_cv_manager_special);
  RUN_TEST(test_motor_control_default_parameters);
  RUN_TEST(test_lights_control_default_behavior);
  RUN_TEST(test_cv_programmer_enter_programming_mode);
  RUN_TEST(test_cv_programmer_programming_lock);
  return UNITY_END();
}
