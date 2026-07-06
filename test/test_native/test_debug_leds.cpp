#include "DebugLeds.h"
#include <unity.h>

void test_debug_leds_heartbeat(void) {
  // neoPin, neoPwrPin, numPixels, redPin, greenPin, bluePin
  DebugLeds debugLeds(10, 11, 1, 12, 13, 14);
  debugLeds.setup();

  // Test Case 1: At 0ms (should be darkened)
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_EQUAL(0, debugLeds.getPixelColor(0));

  // Test Case 2: At 50ms (still within [0, 100) ms window, should be darkened)
  // Need to advance_millis because update has a 50ms throttle
  advance_millis(60);
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_EQUAL(0, debugLeds.getPixelColor(0));

  // Test Case 3: At 120ms (outside [0, 100) ms window, speed 0 -> breathing blue)
  advance_millis(60); // Total 120ms
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  // (120 / 20) % 255 = 6. 6 < 127 so breath = 6. Color = (0, 0, 12)
  TEST_ASSERT_EQUAL(Adafruit_NeoPixel::Color(0, 0, 12), debugLeds.getPixelColor(0));

  // Test Case 4: At 1000ms (next second heartbeat starts)
  advance_millis(880); // Total 1000ms
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_EQUAL(0, debugLeds.getPixelColor(0));

  // Test Case 5: At 1050ms (still heartbeat)
  advance_millis(50); // Total 1050ms
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_EQUAL(0, debugLeds.getPixelColor(0));

  // Test Case 6: At 1100ms (heartbeat ended)
  advance_millis(50); // Total 1100ms
  debugLeds.update(0, MM2DirectionState_Forward, false, false, false, false);
  TEST_ASSERT_NOT_EQUAL(0, debugLeds.getPixelColor(0));
}
