#ifndef DEBUG_LEDS_H
#define DEBUG_LEDS_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <MaerklinMotorola.h>

class DebugLeds {
public:
  DebugLeds(int neoPin, int neoPwrPin, int numPixels, int redPin, int greenPin,
            int bluePin);
  void setup();
  void update(int speedStep, bool f1, bool isMm2Locked, bool isKickstarting,
              bool isTimeout);

private:
  void              setIntLed(int pin, bool on);
  Adafruit_NeoPixel pixels;
  unsigned long     lastVisUpdate;
  int               neoPin_priv;
  int               neoPwrPin_priv;
  int               redPin_priv;
  int               greenPin_priv;
  int               bluePin_priv;
};

#endif // DEBUG_LEDS_H
