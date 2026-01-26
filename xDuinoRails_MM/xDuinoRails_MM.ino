#include <Arduino.h>
#include "CvManager.h"
#include "ProtocolHandler.h"
#include "MotorControl.h"
#include "hal/Eeprom.h"
#include "LightsControl.h"
#include "DebugLeds.h"

// ==========================================
// 1. KONFIGURATION
// ==========================================
#define MOTOR_TYPE 1  // 1=HLA (Gross), 2=Glockenanker (Klein)

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
// 3. MODULE INSTANCES
// ==========================================
#ifndef PIO_UNIT_TESTING
Eeprom eeprom;
CvManager cvManager(eeprom);
ProtocolHandler protocol(DCC_MM_SIGNAL);
MotorControl motor(MOTOR_TYPE, MOTOR_PIN_A, MOTOR_PIN_B, BEMF_PIN_A, BEMF_PIN_B);
LightsControl lights(LED_F0f, LED_F0b);
DebugLeds debugLeds(NEO_PIN, NEO_PWR_PIN, NUMPIXELS, PIN_INT_RED, PIN_INT_GREEN, PIN_INT_BLUE);
#endif

// ==========================================
// 4. HELPER FUNKTIONEN
// ==========================================

const int PWM_MAX = 1023;
#if MOTOR_TYPE == 1
  const int PWM_MIN_MOVING = 350;
#else
  const int PWM_MIN_MOVING = 80;
#endif

int getLinSpeed(int step) {
    if (step == 0) return 0;
    if (step >= 14) return PWM_MAX;
    return map(step, 1, 14, PWM_MIN_MOVING, PWM_MAX);
}

// Global ISR required for attachInterrupt
void isr_protocol();

// ==========================================
// 5. SETUP
// ==========================================
#ifndef PIO_UNIT_TESTING
void setup() {
    analogReadResolution(12); // Wichtig für RP2040 (0-4095)
    cvManager.setup(512);
    protocol.setAddress(cvManager.getCv(1));
    protocol.setup();
    motor.setup();
    lights.setup();
    debugLeds.setup();
}
#endif

// ==========================================
// 6. MAIN LOOP
// ==========================================
#ifndef PIO_UNIT_TESTING
void loop() {
    protocol.loop();

    if (protocol.isTimeout()) {
        motor.stop();
    } else {
        int targetPwm = getLinSpeed(protocol.getTargetSpeed());
        motor.update(targetPwm, protocol.getTargetDirection());
    }

    lights.update(
        motor.getCurrentDirection(),
        protocol.getFunctionState(0)
    );
    debugLeds.update(
        protocol.getTargetSpeed(),
        protocol.getFunctionState(1),
        protocol.isMm2Locked(),
        motor.isKickstarting(),
        protocol.isTimeout()
    );
}
#endif
