#ifndef LIGHTS_CONTROL_H
#define LIGHTS_CONTROL_H

#include <Arduino.h>
#include <MaerklinMotorola.h>
#include "CvManager.h"

class LightsControl {
public:
    LightsControl(CvManager& cvManager, int f0fPin, int f0bPin);
    void setup();
    void update(MM2DirectionState direction, bool f0);

private:
    CvManager& cvManager;
    int f0fPin_priv;
    int f0bPin_priv;
};

#endif // LIGHTS_CONTROL_H
