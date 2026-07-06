#include "LightsControl.h"

LightsControl::LightsControl(CvManager &cvManager, int f0fPin, int f0bPin)
    : cvManager(cvManager) {
  f0fPin_priv = f0fPin;
  f0bPin_priv = f0bPin;
}

void LightsControl::setup() {
  pinMode(f0fPin_priv, OUTPUT);
  pinMode(f0bPin_priv, OUTPUT);
}

void LightsControl::update(MM2DirectionState direction, bool f0) {
  uint8_t f0fCv = cvManager.getCv(CV_FRONT_LIGHT_F0F);
  uint8_t f0rCv = cvManager.getCv(CV_REAR_LIGHT_F0R);

  bool f0f = (f0fCv & 0x01) && f0;
  bool f0r = (f0rCv & 0x02) && f0;

  int brightness = cvManager.getCv(CV_LIGHT_BRIGHTNESS); // 0-7
  if (brightness > 7)
    brightness = 7;
  int pwmValue = map(brightness, 0, 7, 0, 1023);

  if (direction == MM2DirectionState_Forward) {
    analogWrite(f0fPin_priv, f0f ? pwmValue : 0);
    analogWrite(f0bPin_priv, 0);
  } else {
    analogWrite(f0fPin_priv, 0);
    analogWrite(f0bPin_priv, f0r ? pwmValue : 0);
  }
}
