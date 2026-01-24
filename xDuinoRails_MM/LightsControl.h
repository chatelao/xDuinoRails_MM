#ifndef LIGHTS_CONTROL_H
#define LIGHTS_CONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <MaerklinMotorola.h>

class LightsControl {
public:
    LightsControl();
    void setup();
    void update(int speedStep, MM2DirectionState direction, bool f0, bool f1, bool isMm2Locked, bool isKickstarting, bool isTimeout);

private:
    void setIntLed(int pin, bool on);
    Adafruit_NeoPixel pixels;
    unsigned long lastVisUpdate;
};

#endif // LIGHTS_CONTROL_H
