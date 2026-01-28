#include "ProtocolHandler.h"
#include <Arduino.h>
#include <unity.h>

// Define dummy pins since we are not testing the hardware interaction
#define DCC_MM_SIGNAL_PIN 0
#define ADDRESS           1

ProtocolHandler protocolHandler(DCC_MM_SIGNAL_PIN);

void setUp(void) {
  // set stuff up here
  protocolHandler.setAddress(ADDRESS);
  protocolHandler.setup();
}

void tearDown(void) {
  // clean stuff up here
}

// Testet die Timeout-Logik des ProtocolHandlers.
void test_timeout_logic(void) {
  // Initial sollte der Timeout-Status 'false' sein.
  TEST_ASSERT_FALSE(protocolHandler.isTimeout());

  // Simuliere das Verstreichen von Zeit ohne eingehende Befehle.
  // Der Timeout ist auf 1500ms eingestellt, wir simulieren 2000ms.
  delay(2000);

  // Nach Ablauf der Zeit sollte der Timeout-Status 'true' sein.
  TEST_ASSERT_TRUE(protocolHandler.isTimeout());
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_timeout_logic);
}

void loop() { UNITY_END(); }
