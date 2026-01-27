#include "CvManager.h"
#include "CvProgrammer.h"
#include "DebugLeds.h"
#include "LightsControl.h"
#include "MotorControl.h"
#include "ProtocolHandler.h"
#include <Arduino.h>

// ==========================================
// 1. KONFIGURATION
// ==========================================

// ==========================================
// PIN DEFINITIONEN (Seeed XIAO RP2040)
// ==========================================

const int DCC_MM_SIGNAL = D2;

const int MOTOR_PIN_A = D7;
const int MOTOR_PIN_B = D8;
const int BEMF_PIN_A  = A0;
const int BEMF_PIN_B  = A1;

const int LED_F0b = D9;
const int LED_F0f = D10;

const int PIN_INT_RED   = 17;
const int PIN_INT_GREEN = 16;
const int PIN_INT_BLUE  = 25;
const int NEO_PIN       = 12;
const int NEO_PWR_PIN   = 11;

const int NUMPIXELS = 1;

// ==========================================
// 3. MODULE INSTANCES
// ==========================================
CvManager cvManager;

ProtocolHandler protocol(DCC_MM_SIGNAL);
MotorControl motor(cvManager, MOTOR_PIN_A, MOTOR_PIN_B, BEMF_PIN_A, BEMF_PIN_B);
LightsControl lights(cvManager, LED_F0f, LED_F0b);
CvProgrammer cvProgrammer(&cvManager, &protocol);

DebugLeds debugLeds(NEO_PIN, NEO_PWR_PIN, NUMPIXELS, PIN_INT_RED, PIN_INT_GREEN,
                    PIN_INT_BLUE);

// ==========================================
// 4. HELPER FUNKTIONEN
// ==========================================

const int PWM_MAX = 1023;

int getLinSpeed(int step) {
  if (step == 0)
    return 0;
  if (step >= 14)
    return PWM_MAX;
  int pwmMinMoving = cvManager.getCv(CV_START_VOLTAGE) * 40;
  if (pwmMinMoving == 0) {
    pwmMinMoving = 1; // Ensure motor can move if CV is set to 0
  }
  return map(step, 1, 14, pwmMinMoving, PWM_MAX);
}

// Global ISR required for attachInterrupt
void isr_protocol();

// ==========================================
// 5. SETUP
// ==========================================
#ifndef PIO_UNIT_TESTING
void setup() {
  analogReadResolution(12); // Wichtig für RP2040 (0-4095)
  cvManager.setup();
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
  cvProgrammer.loop();

  if (protocol.isTimeout()) {
    motor.stop();
  } else {
    int targetPwm = getLinSpeed(protocol.getTargetSpeed());
    motor.update(targetPwm, protocol.getTargetDirection());
  }

  lights.update(motor.getCurrentDirection(), protocol.getFunctionState(0));
  debugLeds.update(protocol.getTargetSpeed(), protocol.getFunctionState(1),
                   protocol.isMm2Locked(), motor.isKickstarting(),
                   protocol.isTimeout());
}
#endif
