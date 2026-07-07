#ifndef LIGHTS_CONTROL_H
#define LIGHTS_CONTROL_H

#include "CvManager.h"
#include <Arduino.h>
#include <MaerklinMotorola.h>

class LightsControl {
public:
  LightsControl(CvManager &cvManager, int f0fPin, int f0bPin, int shutPin);
  void setup();
  void update(MM2DirectionState direction, bool f0);

private:
  CvManager &cvManager;
  int        f0fPin_priv;
  int        f0bPin_priv;
  int        shutPin_priv;

  MM2DirectionState lastDirection;
  bool              lastF0f;
  bool              lastF0r;
};

#endif // LIGHTS_CONTROL_H
