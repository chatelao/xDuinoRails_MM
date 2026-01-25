#include "LightsControl.h"

LightsControl::LightsControl(int ledF0f, int ledF0b, int pinIntRed, int pinIntGreen, int pinIntBlue, int neoPin, int neoPwrPin, int numPixels) : pixels(numPixels, neoPin, NEO_GRB + NEO_KHZ800) {
    _ledF0f = ledF0f;
    _ledF0b = ledF0b;
    _pinIntRed = pinIntRed;
    _pinIntGreen = pinIntGreen;
    _pinIntBlue = pinIntBlue;
    _neoPwrPin = neoPwrPin;
    lastVisUpdate = 0;
}

void LightsControl::setup() {
    pinMode(_neoPwrPin, OUTPUT);
    digitalWrite(_neoPwrPin, HIGH);
    delay(10);
    pixels.begin();
    pixels.setBrightness(40);

    pinMode(_pinIntRed, OUTPUT);   digitalWrite(_pinIntRed, HIGH);
    pinMode(_pinIntGreen, OUTPUT); digitalWrite(_pinIntGreen, HIGH);
    pinMode(_pinIntBlue, OUTPUT);  digitalWrite(_pinIntBlue, HIGH);

    pinMode(_ledF0f, OUTPUT);
    pinMode(_ledF0b, OUTPUT);
}

void LightsControl::update(int speedStep, MM2DirectionState direction, bool f0, bool f1, bool isMm2Locked, bool isKickstarting, bool isTimeout) {
    unsigned long now = millis();

    // Main lights
    if (f0) {
        if (direction == MM2DirectionState_Forward) {
            digitalWrite(_ledF0f, HIGH);
            digitalWrite(_ledF0b, LOW);
        } else {
            digitalWrite(_ledF0f, LOW);
            digitalWrite(_ledF0b, HIGH);
        }
    } else {
        digitalWrite(_ledF0f, LOW);
        digitalWrite(_ledF0b, LOW);
    }

    // Visual Debug
    if (now - lastVisUpdate < 50) return;
    lastVisUpdate = now;

    setIntLed(_pinIntRed, isMm2Locked);
    setIntLed(_pinIntBlue, f1);

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

void LightsControl::setIntLed(int pin, bool on) {
    digitalWrite(pin, on ? LOW : HIGH);
}
