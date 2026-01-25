#include <Arduino.h>
#include "ProtocolHandler.h"
#include "MotorControl.h"
#include "CvManager.h"
#include "LightsControl.h"

// ==========================================
// 1. KONFIGURATION
// ==========================================
#define MOTOR_TYPE 1  // 1=HLA (Gross), 2=Glockenanker (Klein)
const int MM_TIMEOUT_MS    = 1500;
const int MM2_LOCK_TIME    = 5000;

#include "pins.h"

// ==========================================
// 3. MODULE INSTANCES
// ==========================================
CvManager cvManager;
ProtocolHandler protocol(cvManager.getCv(CvManager::CV_PRIMARY_ADDRESS), MM_TIMEOUT_MS, MM2_LOCK_TIME, cvManager);
MotorControl motor(MOTOR_TYPE, cvManager);
LightsControl lights;

// ==========================================
// 4. HELPER FUNKTIONEN
// ==========================================

const int PWM_MAX = 1023;

int getLinSpeed(int step) {
    if (step == 0) return 0;
    int minSpeed = cvManager.getCv(CvManager::CV_START_VOLTAGE);
    int maxSpeed = cvManager.getCv(CvManager::CV_MAX_SPEED);
    if (step >= 14) return maxSpeed;
    return map(step, 1, 14, minSpeed, maxSpeed);
}

// Global ISR required for attachInterrupt
void isr_protocol();

// ==========================================
// 5. SETUP
// ==========================================
void setup() {
    analogReadResolution(12); // Wichtig für RP2040 (0-4095)
    cvManager.setup();
    protocol.setup();
    motor.setup();
    lights.setup();
}

// ==========================================
// 6. MAIN LOOP
// ==========================================
void loop() {
    protocol.loop();

    if (protocol.isTimeout()) {
        motor.stop();
    } else {
        int targetPwm = getLinSpeed(protocol.getTargetSpeed());
        motor.update(targetPwm, protocol.getTargetDirection());
    }

    lights.update(
        protocol.getTargetSpeed(),
        motor.getCurrentDirection(),
        protocol.getFunctionState(0),
        protocol.getFunctionState(1),
        protocol.isMm2Locked(),
        motor.isKickstarting(),
        protocol.isTimeout()
    );
}
