#include "DebugLeds.h"

DebugLeds::DebugLeds(int neoPin, int neoPwrPin, int numPixels, int redPin,
                     int greenPin, int bluePin)
    : pixels(numPixels, neoPin, NEO_GRB + NEO_KHZ800) {
  neoPin_priv    = neoPin;
  neoPwrPin_priv = neoPwrPin;
  redPin_priv    = redPin;
  greenPin_priv  = greenPin;
  bluePin_priv   = bluePin;
  lastVisUpdate  = 0;
}

void DebugLeds::setup() {
  if (neoPwrPin_priv != -1) {
    pinMode(neoPwrPin_priv, OUTPUT);
    digitalWrite(neoPwrPin_priv, HIGH);
    delay(10);
  }

  if (neoPin_priv != -1) {
    pixels.begin();
    pixels.setBrightness(40);
  }

  if (redPin_priv != -1) {
    pinMode(redPin_priv, OUTPUT);
    digitalWrite(redPin_priv, HIGH);
  }
  if (greenPin_priv != -1) {
    pinMode(greenPin_priv, OUTPUT);
    digitalWrite(greenPin_priv, HIGH);
  }
  if (bluePin_priv != -1) {
    pinMode(bluePin_priv, OUTPUT);
    digitalWrite(bluePin_priv, HIGH);
  }
}

void DebugLeds::update(int speedStep, bool f1, bool isMm2Locked,
                       bool isKickstarting, bool isTimeout) {
  unsigned long now = millis();

  if (now - lastVisUpdate < 50)
    return;
  lastVisUpdate = now;

  if (redPin_priv != -1)
    setIntLed(redPin_priv, isMm2Locked);
  if (bluePin_priv != -1)
    setIntLed(bluePin_priv, f1);

  if (neoPin_priv != -1) {
    if (isTimeout) {
      if ((now / 250) % 2) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      } else {
        pixels.setPixelColor(0, 0);
      }
    } else if (isKickstarting) {
      pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // White on Kick
    } else if (speedStep == 0) {
      int val    = (now / 20) % 255;
      int breath = (val > 127) ? 255 - val : val;
      pixels.setPixelColor(0, pixels.Color(0, 0, breath * 2));
    } else {
      int r = map(speedStep, 0, 14, 0, 255);
      int g = map(speedStep, 0, 14, 255, 0);
      pixels.setPixelColor(0, pixels.Color(r, g, 0));
    }
    pixels.show();
  }
}

void DebugLeds::setIntLed(int pin, bool on) {
  if (pin != -1)
    digitalWrite(pin, on ? LOW : HIGH);
}
