#ifndef DEBUG_LEDS_H
#define DEBUG_LEDS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <MaerklinMotorola.h>

class DebugLeds {
public:
    DebugLeds();
    void setup();
    void update(int speedStep, bool f1, bool isMm2Locked, bool isKickstarting, bool isTimeout);

private:
    void setIntLed(int pin, bool on);
    Adafruit_NeoPixel pixels;
    unsigned long lastVisUpdate;
};

#endif // DEBUG_LEDS_H
