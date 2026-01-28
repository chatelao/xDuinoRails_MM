#include "CvManager.h"
#include "LightsControl.h"
#include "MotorControl.h"
#include "ProtocolHandler.h"
#include "RP2040.h"
#include "mocks/CvManager.h"
#include <unity.h>

void test_mm_signal_f0_f1_f2(void);

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

// Test Protocol Handler
#include "ProtocolHandler.h"

void test_protocol_handler_timeout_logic(void) {
  // Reset millis
  mock_millis_value = 0;

  // Setup protocol handler
  ProtocolHandler protocolHandler(0);
  protocolHandler.setAddress(1);
  protocolHandler.setup();

  // Initially, timeout should be false
  TEST_ASSERT_FALSE(protocolHandler.isTimeout());

  // Simulate time passing without any commands
  delay(2000);

  // Now, the timeout should be true
  TEST_ASSERT_TRUE(protocolHandler.isTimeout());
}

void test_protocol_handler_mm1_direction_change(void) {
  // Reset millis
  mock_millis_value = 0;

  ProtocolHandler protocolHandler(0);
  protocolHandler.setAddress(1);
  protocolHandler.setup();

  // Initial direction should be forward
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocolHandler.getTargetDirection());

  // Simulate a direction change command
  MaerklinMotorolaData data;
  data.IsMagnet = false;
  data.Address = 1;
  data.IsMM2 = false;
  data.ChangeDir = true;
  data.Speed = 0;
  data.Function = false;

  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Direction should now be backward
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocolHandler.getTargetDirection());

  // Reset ChangeDir to simulate the signal going low again
  data.ChangeDir = false;
  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Advance time
  delay(300);

  // Simulate another direction change command
  data.ChangeDir = true;
  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Direction should now be forward again
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocolHandler.getTargetDirection());
}

void test_protocol_handler_mm2_direction_change(void) {
  // Reset millis
  mock_millis_value = 0;

  ProtocolHandler protocolHandler(0);
  protocolHandler.setAddress(1);
  protocolHandler.setup();

  // Initial direction is forward
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocolHandler.getTargetDirection());

  // --- First MM2 packet: Enter MM2 mode ---
  MaerklinMotorolaData data;
  data.IsMagnet = false;
  data.Address = 1;
  data.IsMM2 = true;
  data.MM2Direction = MM2DirectionState_Unavailable; // No direction change yet
  data.MM2FunctionIndex = 0;
  data.Speed = 5;
  data.ChangeDir = false;
  data.Function = false;

  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Direction should not have changed yet, but we should be in MM2 locked mode
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocolHandler.getTargetDirection());
  TEST_ASSERT_TRUE(protocolHandler.isMm2Locked());

  // --- Second MM2 packet: Change direction ---
  delay(100);
  data.MM2Direction = MM2DirectionState_Backward;
  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Direction should now be backward
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocolHandler.getTargetDirection());

  // --- Third MM2 packet: Change direction back ---
  delay(100);
  data.MM2Direction = MM2DirectionState_Forward;
  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Direction should be forward again
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocolHandler.getTargetDirection());
}

void test_protocol_handler_mm1_mm2_interaction(void) {
  // Reset millis
  mock_millis_value = 0;

  ProtocolHandler protocolHandler(0);
  protocolHandler.setAddress(1);
  protocolHandler.setup();

  // --- First MM2 packet: Enter MM2 mode ---
  MaerklinMotorolaData data;
  data.IsMagnet = false;
  data.Address = 1;
  data.IsMM2 = true;
  data.MM2Direction = MM2DirectionState_Unavailable;
  data.MM2FunctionIndex = 0;
  data.Speed = 5;
  data.ChangeDir = false;
  data.Function = false;

  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // We should be in MM2 locked mode
  TEST_ASSERT_TRUE(protocolHandler.isMm2Locked());
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocolHandler.getTargetDirection());

  // --- Second, an MM1 direction change packet ---
  delay(100);
  data.IsMM2 = false;
  data.ChangeDir = true;
  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Direction should NOT have changed because we are in MM2 locked mode
  TEST_ASSERT_EQUAL(MM2DirectionState_Forward, protocolHandler.getTargetDirection());

  // --- Wait for MM2 lock to time out ---
  delay(5000);
  TEST_ASSERT_FALSE(protocolHandler.isMm2Locked());

  // --- Third, the same MM1 direction change packet again ---
  // Reset ChangeDir to simulate the signal going low again
  data.ChangeDir = false;
  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();
  delay(300); // Debounce time
  data.ChangeDir = true;
  protocolHandler.mm.SetData(&data);
  protocolHandler.loop();

  // Direction SHOULD now change
  TEST_ASSERT_EQUAL(MM2DirectionState_Backward, protocolHandler.getTargetDirection());
}

void setUp(void) { reboot_called = false; }

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
  RUN_TEST(test_protocol_handler_timeout_logic);
  RUN_TEST(test_protocol_handler_mm1_direction_change);
  RUN_TEST(test_protocol_handler_mm2_direction_change);
  RUN_TEST(test_protocol_handler_mm1_mm2_interaction);
  return UNITY_END();
}
