#ifndef LIGHTS_CONTROL_H
#define LIGHTS_CONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <MaerklinMotorola.h>

class LightsControl {
public:
    LightsControl(int ledF0f, int ledF0b, int pinIntRed, int pinIntGreen, int pinIntBlue, int neoPin, int neoPwrPin, int numPixels);
    void setup();
    void update(int speedStep, MM2DirectionState direction, bool f0, bool f1, bool isMm2Locked, bool isKickstarting, bool isTimeout);

private:
    void setIntLed(int pin, bool on);
    Adafruit_NeoPixel pixels;
    int _ledF0f;
    int _ledF0b;
    int _pinIntRed;
    int _pinIntGreen;
    int _pinIntBlue;
    int _neoPwrPin;
    unsigned long lastVisUpdate;
};

#endif // LIGHTS_CONTROL_H
