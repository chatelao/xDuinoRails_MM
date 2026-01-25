#ifndef LIGHTS_CONTROL_H
#define LIGHTS_CONTROL_H

#include <Arduino.h>
#include <MaerklinMotorola.h>

class LightsControl {
public:
    LightsControl();
    void setup();
    void update(MM2DirectionState direction, bool f0);

private:
};

#endif // LIGHTS_CONTROL_H
