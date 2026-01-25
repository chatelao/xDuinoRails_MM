#include "LightsControl.h"
#include "pins.h"

LightsControl::LightsControl() {
}

void LightsControl::setup() {
    pinMode(LED_F0f, OUTPUT);
    pinMode(LED_F0b, OUTPUT);
}

void LightsControl::update(MM2DirectionState direction, bool f0) {
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
}
