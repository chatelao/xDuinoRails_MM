#ifndef PIO_UNIT_TESTING

#include "MotorControl.h"

const int PWM_RANGE = 1023;

MotorControl::MotorControl(int mType, int pinA, int pinB, int bemfA, int bemfB) {
    motorType = mType;
    pinA_priv = pinA;
    pinB_priv = pinB;
    bemfA_priv = bemfA;
    bemfB_priv = bemfB;
    targetPwm = 0;
    currDirection = MM2DirectionState_Forward;
    targetDirection = MM2DirectionState_Forward;
    isKickstarting_priv = false;
    kickstartBegin = 0;
    lastBemfMeasure = 0;
    lastSpeed = 0;
    previousPwm = 0;

    if (motorType == 1) { // HLA
        PWM_FREQ = 400;
        PWM_MIN_MOVING = 350;
        KICK_PWM = 1023;
        KICK_MAX_TIME = 150;
        BEMF_THRESHOLD = 120;
        BEMF_SAMPLE_INT = 15;
    } else { // Glockenanker
        PWM_FREQ = 20000;
        PWM_MIN_MOVING = 80;
        KICK_PWM = 600;
        KICK_MAX_TIME = 80;
        BEMF_THRESHOLD = 80;
        BEMF_SAMPLE_INT = 10;
    }
}

void MotorControl::setup() {
    pinMode(pinA_priv, OUTPUT);
    pinMode(pinB_priv, OUTPUT);
    pinMode(bemfA_priv, INPUT);
    pinMode(bemfB_priv, INPUT);

    analogWriteFreq(PWM_FREQ);
    analogWriteRange(PWM_RANGE);

    writeMotorHardware(0, MM2DirectionState_Forward);
}

void MotorControl::update(int pwm, MM2DirectionState dir) {
    targetPwm = pwm;
    targetDirection = dir;
    unsigned long now = millis();

    // Bei Stillstand Richtung sofort übernehmen
    if (targetPwm == 0) currDirection = targetDirection;

    if (previousPwm == 0 && targetPwm > 0 && KICK_MAX_TIME > 0) {
        isKickstarting_priv = true;
        kickstartBegin = now;
        lastBemfMeasure = 0;
    }
    if (targetPwm == 0) {
        isKickstarting_priv = false;
    }

    if (isKickstarting_priv) {
        if (now - kickstartBegin >= KICK_MAX_TIME) {
            isKickstarting_priv = false;
        } else {
            if (now - lastBemfMeasure > BEMF_SAMPLE_INT) {
                int currentBEMF = readBEMF();
                lastBemfMeasure = now;
                if (currentBEMF > BEMF_THRESHOLD) {
                    isKickstarting_priv = false;
                }
            }
            if (isKickstarting_priv) {
                writeMotorHardware(KICK_PWM, currDirection);
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
        digitalWrite(pinB_priv, LOW);
        analogWrite(pinA_priv, pwm);
    } else {
        digitalWrite(pinA_priv, LOW);
        analogWrite(pinB_priv, pwm);
    }
}

int MotorControl::readBEMF() {
    digitalWrite(pinA_priv, LOW);
    digitalWrite(pinB_priv, LOW);
    delayMicroseconds(500);
    int valA = analogRead(bemfA_priv);
    int valB = analogRead(bemfB_priv);
    return abs(valA - valB);
}

#endif // PIO_UNIT_TESTING
