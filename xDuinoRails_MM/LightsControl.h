#ifndef LIGHTS_CONTROL_H
#define LIGHTS_CONTROL_H

#include <Arduino.h>
#include <MaerklinMotorola.h>
#include "ILightsHardware.h"

class LightsControl {
public:
    LightsControl(int f0fPin, int f0bPin, ILightsHardware* hardware);
    void setup();
    void update(MM2DirectionState direction, bool f0);

private:
    int f0fPin_priv;
    int f0bPin_priv;
    ILightsHardware* hardware_priv;
};

#endif // LIGHTS_CONTROL_H
