#include "DebugLeds.h"

DebugLeds::DebugLeds(int neoPin, int neoPwrPin, int numPixels, int redPin, int greenPin, int bluePin)
    : pixels(numPixels, neoPin, NEO_GRB + NEO_KHZ800) {
    neoPwrPin_priv = neoPwrPin;
    redPin_priv = redPin;
    greenPin_priv = greenPin;
    bluePin_priv = bluePin;
    lastVisUpdate = 0;
}

void DebugLeds::setup() {
    pinMode(neoPwrPin_priv, OUTPUT);
    digitalWrite(neoPwrPin_priv, HIGH);
    delay(10);
    pixels.begin();
    pixels.setBrightness(40);

    pinMode(redPin_priv, OUTPUT);   digitalWrite(redPin_priv, HIGH);
    pinMode(greenPin_priv, OUTPUT); digitalWrite(greenPin_priv, HIGH);
    pinMode(bluePin_priv, OUTPUT);  digitalWrite(bluePin_priv, HIGH);
}

void DebugLeds::update(int speedStep, bool f1, bool isMm2Locked, bool isKickstarting, bool isTimeout) {
    unsigned long now = millis();

    if (now - lastVisUpdate < 50) return;
    lastVisUpdate = now;

    setIntLed(redPin_priv, isMm2Locked);
    setIntLed(bluePin_priv, f1);

    if (isTimeout) {
        if ((now / 250) % 2) {
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        } else {
            pixels.setPixelColor(0, 0);
        }
    } else if (isKickstarting) {
        pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // White on Kick
    } else if (speedStep == 0) {
        int val = (now / 20) % 255;
        int breath = (val > 127) ? 255 - val : val;
        pixels.setPixelColor(0, pixels.Color(0, 0, breath * 2));
    } else {
        int r = map(speedStep, 0, 14, 0, 255);
        int g = map(speedStep, 0, 14, 255, 0);
        pixels.setPixelColor(0, pixels.Color(r, g, 0));
    }
    pixels.show();
}

void DebugLeds::setIntLed(int pin, bool on) {
    digitalWrite(pin, on ? LOW : HIGH);
}
