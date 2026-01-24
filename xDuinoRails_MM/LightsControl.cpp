#include "LightsControl.h"
#include "pins.h"

LightsControl::LightsControl() : pixels(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800) {
    lastVisUpdate = 0;
}

void LightsControl::setup() {
    pinMode(NEO_PWR_PIN, OUTPUT);
    digitalWrite(NEO_PWR_PIN, HIGH);
    delay(10);
    pixels.begin();
    pixels.setBrightness(40);

    pinMode(PIN_INT_RED, OUTPUT);   digitalWrite(PIN_INT_RED, HIGH);
    pinMode(PIN_INT_GREEN, OUTPUT); digitalWrite(PIN_INT_GREEN, HIGH);
    pinMode(PIN_INT_BLUE, OUTPUT);  digitalWrite(PIN_INT_BLUE, HIGH);

    pinMode(LED_F0f, OUTPUT);
    pinMode(LED_F0b, OUTPUT);
}

void LightsControl::update(int speedStep, MM2DirectionState direction, bool f0, bool f1, bool isMm2Locked, bool isKickstarting, bool isTimeout) {
    unsigned long now = millis();

    // Main lights
    if (f0) {
        if (direction == MM2DirectionState_Forward) {
            digitalWrite(LED_F0f, HIGH);
            digitalWrite(LED_F0b, LOW);
        } else {
            digitalWrite(LED_F0f, LOW);
            digitalWrite(LED_F0b, HIGH);
        }
    } else {
        digitalWrite(LED_F0f, LOW);
        digitalWrite(LED_F0b, LOW);
    }

    // Visual Debug
    if (now - lastVisUpdate < 50) return;
    lastVisUpdate = now;

    setIntLed(PIN_INT_RED, isMm2Locked);
    setIntLed(PIN_INT_BLUE, f1);

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
