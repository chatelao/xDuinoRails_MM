#include "LightsControl.h"

LightsControl::LightsControl(int f0fPin, int f0bPin) {
    f0fPin_priv = f0fPin;
    f0bPin_priv = f0bPin;
}

void LightsControl::setup() {
    pinMode(f0fPin_priv, OUTPUT);
    pinMode(f0bPin_priv, OUTPUT);
}

void LightsControl::update(MM2DirectionState direction, bool f0) {
    // Main lights
    if (f0) {
        if (direction == MM2DirectionState_Forward) {
            digitalWrite(f0fPin_priv, HIGH);
            digitalWrite(f0bPin_priv, LOW);
        } else {
            digitalWrite(f0fPin_priv, LOW);
            digitalWrite(f0bPin_priv, HIGH);
        }
    } else {
        digitalWrite(f0fPin_priv, LOW);
        digitalWrite(f0bPin_priv, LOW);
    }
}
