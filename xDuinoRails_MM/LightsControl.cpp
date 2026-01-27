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

  if (direction == MM2DirectionState_Forward) {
    digitalWrite(f0fPin_priv, f0f ? HIGH : LOW);
    digitalWrite(f0bPin_priv, LOW);
  } else {
    digitalWrite(f0fPin_priv, LOW);
    digitalWrite(f0bPin_priv, f0r ? HIGH : LOW);
  }
}
