#include <unity.h>
#include "CvManager.h"
#include "ProtocolHandler.h"
#include "EEPROM.h"
#include "RP2040.h"
#include "MaerklinMotorola.h"

// Define dummy pins since we are not testing the hardware interaction
#define DCC_MM_SIGNAL_PIN 0
#define ADDRESS 1

// Define the global protocol object required by the ISR in ProtocolHandler.cpp
ProtocolHandler protocol(DCC_MM_SIGNAL_PIN);

// Mock implementation for RP2040 reboot
bool reboot_called = false;
RP2040 rp2040;
void RP2040::reboot() {
    reboot_called = true;
}

// --- CvManager Tests ---
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

void test_cv_manager_magic_byte() {
  const uint8_t EEPROM_MAGIC_BYTE = 0xAF;
  const int EEPROM_MAGIC_BYTE_ADDR = 0;
  EEPROM.put(EEPROM_MAGIC_BYTE_ADDR, EEPROM_MAGIC_BYTE);
  EEPROM.put(CV_BASE_ADDRESS, 123);
  CvManager cvManager;
  cvManager.setup();
  TEST_ASSERT_EQUAL(123, cvManager.getCv(CV_BASE_ADDRESS));
}

// --- ProtocolHandler Tests ---
void test_initial_state(void) {
    TEST_ASSERT_EQUAL(0, protocol.getTargetSpeed());
    TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocol.getTargetDirection());
    TEST_ASSERT_FALSE(protocol.getFunctionState(0));
    TEST_ASSERT_FALSE(protocol.getFunctionState(1));
    TEST_ASSERT_FALSE(protocol.getFunctionState(2));
}

void test_speed_change(void) {
    MaerklinMotorola::mockData.Address = ADDRESS;
    MaerklinMotorola::mockData.Speed = 10;
    protocol.loop();
    TEST_ASSERT_EQUAL(10, protocol.getTargetSpeed());
}

void test_direction_change_rising_edge(void) {
    ProtocolHandler localProtocol(DCC_MM_SIGNAL_PIN);
    localProtocol.setAddress(ADDRESS);
    localProtocol.setup();
    localProtocol.mm.SetData(&MaerklinMotorola::mockData);

    MaerklinMotorola::mockData.Address = ADDRESS;

    // 1. Initial state
    MaerklinMotorola::mockData.ChangeDir = false;
    localProtocol.loop();
    TEST_ASSERT_EQUAL(MM2DirectionState_Forward, localProtocol.getTargetDirection());

    // 2. Rising edge
    MaerklinMotorola::mockData.ChangeDir = true;
    localProtocol.loop();
    TEST_ASSERT_EQUAL(MM2DirectionState_Backward, localProtocol.getTargetDirection());
}

void test_direction_change_debouncing(void) {
    ProtocolHandler localProtocol(DCC_MM_SIGNAL_PIN);
    localProtocol.setAddress(ADDRESS);
    localProtocol.setup();
    localProtocol.mm.SetData(&MaerklinMotorola::mockData);

    // 1. First direction change
    MaerklinMotorola::mockData.Address = ADDRESS;
    MaerklinMotorola::mockData.ChangeDir = false;
    localProtocol.loop();
    MaerklinMotorola::mockData.ChangeDir = true;
    mock_millis += 100; // Not enough time for debounce
    localProtocol.loop();
    TEST_ASSERT_EQUAL(MM2DirectionState_Forward, localProtocol.getTargetDirection()); // Should not have changed

    // 2. Second direction change, after debounce time
    MaerklinMotorola::mockData.ChangeDir = false;
    localProtocol.loop();
    MaerklinMotorola::mockData.ChangeDir = true;
    mock_millis += 300; // Enough time for debounce
    localProtocol.loop();
    TEST_ASSERT_EQUAL(MM2DirectionState_Backward, localProtocol.getTargetDirection()); // Should have changed
}

void test_function_change(void) {
    MaerklinMotorola::mockData.Address = ADDRESS;
    MaerklinMotorola::mockData.Function = true;
    protocol.loop();
    TEST_ASSERT_TRUE(protocol.getFunctionState(0));
}

void test_mm2_function_change(void) {
    MaerklinMotorola::mockData.Address = ADDRESS;
    MaerklinMotorola::mockData.IsMM2 = true;
    MaerklinMotorola::mockData.MM2FunctionIndex = 1;
    MaerklinMotorola::mockData.IsMM2FunctionOn = true;
    protocol.loop();
    TEST_ASSERT_TRUE(protocol.getFunctionState(1));
}


// --- Test Runner ---
void setUp(void) {
  reboot_called = false;
  protocol.setAddress(ADDRESS);
  protocol.setup();
  // Reset mock data
  MaerklinMotorola::mockData = {0};
  protocol.mm.SetData(&MaerklinMotorola::mockData);
  // Reset EEPROM mock by resizing to 0 and then back to the required size.
  // This is a simple way to clear it. A better mock might have a clear() method.
  EEPROM.begin(0);
  EEPROM.begin(512);
  mock_millis = 0;
}

void tearDown(void) {
  protocol.mm.SetData(nullptr);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    // CV Manager Tests
    RUN_TEST(test_cv_manager_get_set);
    RUN_TEST(test_cv_manager_defaults);
    RUN_TEST(test_cv_manager_special);
    RUN_TEST(test_cv_manager_magic_byte);

    // Protocol Handler Tests
    RUN_TEST(test_initial_state);
    RUN_TEST(test_speed_change);
    RUN_TEST(test_direction_change_rising_edge);
    RUN_TEST(test_direction_change_debouncing);
    RUN_TEST(test_function_change);
    RUN_TEST(test_mm2_function_change);

    return UNITY_END();
}
