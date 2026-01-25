#include <Arduino.h>
#include "ProtocolHandler.h"
#include "MotorControl.h"
#include "LightsControl.h"
#include "CvManager.h"

// ==========================================
// 1. KONFIGURATION
// ==========================================
// MOTOR_TYPE and MM_ADDRESS are now managed by CvManager

#include "pins.h"

// ==========================================
// 3. MODULE INSTANCES
// ==========================================
CvManager cvManager;
ProtocolHandler protocol(&cvManager);
MotorControl motor(&cvManager);
LightsControl lights;

// ==========================================
// 4. HELPER FUNKTIONEN
// ==========================================

const int PWM_MAX = 1023;

int getLinSpeed(int step) {
    if (step == 0) return 0;
    if (step >= 14) return PWM_MAX;

    int min_moving = cvManager.getCv(CV_START_VOLTAGE) * 4;
    int max_speed = cvManager.getCv(CV_MAXIMUM_SPEED) * 4;
    if (max_speed > PWM_MAX) max_speed = PWM_MAX;
    if (min_moving > max_speed) min_moving = max_speed;

    return map(step, 1, 14, min_moving, max_speed);
}

// Global ISR required for attachInterrupt
void isr_protocol();

void notifyCVAck(void)
{
  digitalWrite( ACK_PIN, HIGH );
  delay( 6 );
  digitalWrite( ACK_PIN, LOW );
}

// ==========================================
// 5. SETUP
// ==========================================
void setup() {
    pinMode(ACK_PIN, OUTPUT);
    digitalWrite(ACK_PIN, LOW);

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
    cvManager.loop();
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
