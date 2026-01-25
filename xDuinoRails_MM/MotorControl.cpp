#include "MotorControl.h"
#include "pins.h"

const int PWM_RANGE = 1023;

MotorControl::MotorControl(int mType, CvManager& cvMgr) : cvManager(cvMgr) {
    motorType = mType;
    targetPwm = 0;
    currDirection = MM2DirectionState_Forward;
    targetDirection = MM2DirectionState_Forward;
    isKickstarting_priv = false;
    kickstartBegin = 0;
    lastBemfMeasure = 0;
    lastSpeed = 0;
}

void MotorControl::setup() {
    pinMode(MOTOR_PIN_A, OUTPUT);
    pinMode(MOTOR_PIN_B, OUTPUT);
    pinMode(BEMF_PIN_A, INPUT);
    pinMode(BEMF_PIN_B, INPUT);

    // analogWriteFreq(PWM_FREQ); // This will be set based on CVs later
    analogWriteRange(PWM_RANGE);

    writeMotorHardware(0, MM2DirectionState_Forward);
}

void MotorControl::update(int pwm, MM2DirectionState dir) {
    targetPwm = pwm;
    targetDirection = dir;
    unsigned long now = millis();

    // Bei Stillstand Richtung sofort übernehmen
    if (targetPwm == 0) currDirection = targetDirection;

    static int previousPwm = 0;
    int kickTime = cvManager.getCv(60); // CV for Kick Time
    if (previousPwm == 0 && targetPwm > 0 && kickTime > 0) {
        isKickstarting_priv = true;
        kickstartBegin = now;
        lastBemfMeasure = 0;
    }
    if (targetPwm == 0) {
        isKickstarting_priv = false;
    }

    if (isKickstarting_priv) {
        if (now - kickstartBegin >= kickTime) {
            isKickstarting_priv = false;
        } else {
            if (now - lastBemfMeasure > 10) { // BEMF sample interval
                int currentBEMF = readBEMF();
                lastBemfMeasure = now;
                if (currentBEMF > cvManager.getCv(59)) { // BEMF Threshold
                    isKickstarting_priv = false;
                }
            }
            if (isKickstarting_priv) {
                writeMotorHardware(cvManager.getCv(61), currDirection); // Kick PWM
            }
        }
    }

    if (!isKickstarting_priv) {
        if (currDirection != targetDirection) {
            writeMotorHardware(0, currDirection);
            currDirection = targetDirection;
        } else {
            writeMotorHardware(targetPwm, currDirection);
        }
    }
    previousPwm = targetPwm;
}

void MotorControl::stop() {
    targetPwm = 0;
    writeMotorHardware(0, currDirection);
}

bool MotorControl::isKickstarting() {
    return isKickstarting_priv;
}

MM2DirectionState MotorControl::getCurrentDirection() {
    return currDirection;
}

void MotorControl::writeMotorHardware(int pwm, MM2DirectionState dir) {
    if (pwm > PWM_RANGE) pwm = PWM_RANGE;
    if (pwm < 0) pwm = 0;

    if (dir == MM2DirectionState_Forward) {
        digitalWrite(MOTOR_PIN_B, LOW);
        analogWrite(MOTOR_PIN_A, pwm);
    } else {
        digitalWrite(MOTOR_PIN_A, LOW);
        analogWrite(MOTOR_PIN_B, pwm);
    }
}

int MotorControl::readBEMF() {
    digitalWrite(MOTOR_PIN_A, LOW);
    digitalWrite(MOTOR_PIN_B, LOW);
    delayMicroseconds(500);
    int valA = analogRead(BEMF_PIN_A);
    int valB = analogRead(BEMF_PIN_B);
    return abs(valA - valB);
}
