#include "CvManagerMock.h"
#include "LightsControl.h"
#include <unity.h>
#include <map>

extern std::map<uint8_t, int> analog_write_values;
extern void reset_arduino_mock();

void test_lights_brightness_pwm(void) {
  reset_arduino_mock();
  CvManagerMock cvManager;
  cvManager.setCv(CV_FRONT_LIGHT_F0F, 1);
  cvManager.setCv(CV_REAR_LIGHT_F0R, 2);

  // Default brightness 7 -> 1023 PWM
  cvManager.setCv(CV_LIGHT_BRIGHTNESS, 7);

  LightsControl lights(cvManager, 9, 10);
  lights.setup();

  lights.update(MM2DirectionState_Forward, true);
  TEST_ASSERT_EQUAL(1023, analog_write_values[9]);
  TEST_ASSERT_EQUAL(0, analog_write_values[10]);

  // Brightness 3 -> map(3, 0, 7, 0, 1023) = 438
  cvManager.setCv(CV_LIGHT_BRIGHTNESS, 3);
  lights.update(MM2DirectionState_Backward, true);
  TEST_ASSERT_EQUAL(0, analog_write_values[9]);
  TEST_ASSERT_EQUAL(438, analog_write_values[10]);

  // Brightness 0 -> 0 PWM
  cvManager.setCv(CV_LIGHT_BRIGHTNESS, 0);
  lights.update(MM2DirectionState_Forward, true);
  TEST_ASSERT_EQUAL(0, analog_write_values[9]);
}
