#include <Arduino.h>
#include <unity.h>

void test_simple_assertion(void) { TEST_ASSERT_EQUAL(1, 1); }

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_simple_assertion);
}

void loop() { UNITY_END(); }
