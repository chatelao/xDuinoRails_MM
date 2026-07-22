/*
 * Seeed Studio XIAO RP2040 Pinout (Thor Mapping):
 *
 *                         +---USB---+
 *           (BEMF A)  D0  | [ ] [ ] |  5V
 *           (BEMF B)  D1  | [ ] [ ] |  GND
 *       (Motor Shut)  D2  | [ ] [ ] |  3V3
 *        (Func Shut)  D3  | [ ] [ ] |  D10 (Rear Light)
 *          (DCC-ACK)  D4  | [ ] [ ] |  D9  (Front Light)
 *        (DCC Input)  D5  | [ ] [ ] |  D8  (Motor B)
 *          (RailCom)  D6  | [ ] [ ] |  D7  (Motor A)
 *                         +---------+
 */

#include "CvManager.h"
#include "CvProgrammer.h"
#include "DebugLeds.h"
#include "LightsControl.h"
#include "Logger.h"
#include "MotorControl.h"
#include "ProtocolHandler.h"
#include "SerialConsole.h"
#include <Arduino.h>

// ==========================================
// 1. KONFIGURATION
// ==========================================

// ==========================================
// PIN DEFINITIONEN
// ==========================================

const int DCC_MM_SIGNAL  = D5;
const int DCC_ACK_PIN    = D4;
const int RAILCOM_PIN    = D6;

const int MOTOR_PIN_A    = D7;
const int MOTOR_PIN_B    = D8;

#ifdef ARDUINO_ARCH_RP2040
const int BEMF_PIN_A     = A0;
const int BEMF_PIN_B     = A1;
const int MOTOR_SHUT_PIN = A2; // D2

const int LED_F0b       = D10;
const int LED_F0f       = D9;
const int FUNC_SHUT_PIN = A3; // D3

const int PIN_INT_RED   = 15;
const int PIN_INT_GREEN = -1;
const int PIN_INT_BLUE  = 16;
const int NEO_PIN       = 12;
const int NEO_PWR_PIN   = 11;
#elif defined(ARDUINO_ARCH_ESP32)
const int BEMF_PIN_A     = D0;
const int BEMF_PIN_B     = D1;
const int MOTOR_SHUT_PIN = D2;

const int LED_F0b       = D10;
const int LED_F0f       = D9;
const int FUNC_SHUT_PIN = D3;

const int PIN_INT_RED   = -1;
const int PIN_INT_GREEN = -1;
const int PIN_INT_BLUE  = 15; // LED_BUILTIN / D4
const int NEO_PIN       = -1;
const int NEO_PWR_PIN   = -1;
#endif

const int NUMPIXELS = 1;

// ==========================================
// 3. MODULE INSTANCES
// ==========================================
CvManager cvManager;

ProtocolHandler protocol(DCC_MM_SIGNAL);
MotorControl motor(cvManager, MOTOR_PIN_A, MOTOR_PIN_B, BEMF_PIN_A, BEMF_PIN_B,
                   MOTOR_SHUT_PIN);
LightsControl lights(cvManager, LED_F0f, LED_F0b, FUNC_SHUT_PIN);
CvProgrammer  cvProgrammer(&cvManager, &protocol);
SerialConsole serialConsole(&cvManager, &protocol);

DebugLeds debugLeds(NEO_PIN, NEO_PWR_PIN, NUMPIXELS, PIN_INT_RED, PIN_INT_GREEN,
                    PIN_INT_BLUE);

// ==========================================
// 4. HELPER FUNKTIONEN
// ==========================================

// Global ISR required for attachInterrupt
void isr_protocol();

// ==========================================
// 5. SETUP
// ==========================================
#ifndef PIO_UNIT_TESTING
void setup() {
  analogReadResolution(12);
  cvManager.setup();
  logger.begin(&cvManager, 115200);
  logger.println("xDuinoRails_MM starting...");
  cvManager.printAllCvs();

  protocol.setAddress(cvManager.getCv(1));
  protocol.setSignalTimeout(cvManager.getCv(CV_WATCHDOG_TIMEOUT) * 100);
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
  serialConsole.loop();
  protocol.loop();
  cvProgrammer.loop();

  if (protocol.isSignalTimeout()) {
    unsigned long timeSinceLastSignal = millis() - protocol.getLastSignalTime();
    int           watchdogTimeout     = cvManager.getCv(CV_WATCHDOG_TIMEOUT) * 100;
    unsigned long elapsed             = timeSinceLastSignal - watchdogTimeout;

    if (elapsed >= 500) {
      motor.stop();
    } else {
      int targetSpeed = protocol.getTargetSpeed();
      int rampSpeed   = map(elapsed, 0, 500, targetSpeed, 0);
      motor.setSpeed(rampSpeed, protocol.getTargetDirection());
    }
  } else {
    motor.setSpeed(protocol.getTargetSpeed(), protocol.getTargetDirection());
  }

  lights.update(motor.getCurrentDirection(), protocol.getFunctionState(0));
  debugLeds.update(protocol.getTargetSpeed(), motor.getCurrentDirection(),
                   protocol.getFunctionState(1), protocol.isMm2Locked(),
                   motor.isKickstarting(), protocol.isSignalTimeout());
}
#endif
