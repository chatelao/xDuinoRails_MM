#include <Arduino.h>
#include "ProtocolHandler.h"
#include "MotorControl.h"
#include "LightsControl.h"
#include "CvManager.h"
#include "DebugLeds.h"

// ==========================================
// PIN DEFINITIONEN (Seeed XIAO RP2040)
// ==========================================
const int MOTOR_PIN_A = D7;
const int MOTOR_PIN_B = D8;
const int BEMF_PIN_A = A0;
const int BEMF_PIN_B = A1;
const int DCC_MM_SIGNAL = D2;
const int LED_F0b = D9;
const int LED_F0f = D10;
const int PIN_INT_RED = 17;
const int PIN_INT_GREEN = 16;
const int PIN_INT_BLUE = 25;
const int NEO_PIN = 12;
const int NEO_PWR_PIN = 11;
const int NUMPIXELS = 1;

// ==========================================
// MODULE INSTANCES
// ==========================================
CvManager cvManager;
ProtocolHandler* protocol = nullptr;
MotorControl* motor = nullptr;
LightsControl lights(LED_F0f, LED_F0b);
DebugLeds debugLeds(NEO_PIN, NEO_PWR_PIN, NUMPIXELS, PIN_INT_RED, PIN_INT_GREEN, PIN_INT_BLUE);

// ==========================================
// ISR glue
// ==========================================
void isr_wrapper() {
  if (protocol) {
    protocol->isr();
  }
}

// ==========================================
// SETUP
// ==========================================
void setup() {
    analogReadResolution(12);

    cvManager.setup();

    // Load CVs
    uint8_t address = cvManager.getCv(1);
    uint8_t motorType = cvManager.getCv(100); // Using a custom CV for motor type
    int pwmFreq = cvManager.getCv(101) * 100;
    int pwmMinMoving = cvManager.getCv(2);
    int kickPwm = cvManager.getCv(102);
    int kickMaxTime = cvManager.getCv(103);
    int bemfThreshold = cvManager.getCv(104);
    int bemfSampleInt = cvManager.getCv(105);

    // Instantiate and setup modules
    protocol = new ProtocolHandler(address, DCC_MM_SIGNAL);
    motor = new MotorControl(
        pwmFreq, pwmMinMoving, kickPwm, kickMaxTime, bemfThreshold, bemfSampleInt,
        MOTOR_PIN_A, MOTOR_PIN_B, BEMF_PIN_A, BEMF_PIN_B
    );

    protocol->setup();
    motor->setup();
    lights.setup();
    debugLeds.setup();

    attachInterrupt(digitalPinToInterrupt(DCC_MM_SIGNAL), isr_wrapper, CHANGE);
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
    if (!protocol || !motor) return;

    protocol->loop();

    if (protocol->isTimeout()) {
        motor->stop();
    } else {
        motor->update(protocol->getTargetSpeed(), protocol->getTargetDirection());
    }

    lights.update(
        motor->getCurrentDirection(),
        protocol->getFunctionState(0)
    );
    debugLeds.update(
        protocol->getTargetSpeed(),
        protocol->getFunctionState(1),
        protocol->isMm2Locked(),
        motor->isKickstarting(),
        protocol->isTimeout()
    );
}
