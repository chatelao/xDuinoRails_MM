#include "LightsControl.h"

LightsControl::LightsControl(int f0fPin, int f0bPin, ILightsHardware* hardware) {
    f0fPin_priv = f0fPin;
    f0bPin_priv = f0bPin;
    hardware_priv = hardware;
}

void LightsControl::setup() {
    // Hardware setup is now handled by the main application
}

void LightsControl::update(MM2DirectionState direction, bool f0) {
    // Main lights
    if (f0) {
        if (direction == MM2DirectionState_Forward) {
            hardware_priv->setPin(f0fPin_priv, HIGH);
            hardware_priv->setPin(f0bPin_priv, LOW);
        } else {
            hardware_priv->setPin(f0fPin_priv, LOW);
            hardware_priv->setPin(f0bPin_priv, HIGH);
        }
    } else {
        hardware_priv->setPin(f0fPin_priv, LOW);
        hardware_priv->setPin(f0bPin_priv, LOW);
    }
}
