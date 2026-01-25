#include "MotorControl.h"

const int PWM_RANGE = 1023;

MotorControl::MotorControl(int mType, int motorPinA, int motorPinB, int bemfPinA, int bemfPinB) {
    motorType = mType;
    _motorPinA = motorPinA;
    _motorPinB = motorPinB;
    _bemfPinA = bemfPinA;
    _bemfPinB = bemfPinB;
    targetPwm = 0;
    currDirection = MM2DirectionState_Forward;
    targetDirection = MM2DirectionState_Forward;
    isKickstarting_priv = false;
    kickstartBegin = 0;
    lastBemfMeasure = 0;
    lastSpeed = 0;

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
    pinMode(_motorPinA, OUTPUT);
    pinMode(_motorPinB, OUTPUT);
    pinMode(_bemfPinA, INPUT);
    pinMode(_bemfPinB, INPUT);

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

    static int previousPwm = 0;
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
        digitalWrite(_motorPinB, LOW);
        analogWrite(_motorPinA, pwm);
    } else {
        digitalWrite(_motorPinA, LOW);
        analogWrite(_motorPinB, pwm);
    }
}

int MotorControl::readBEMF() {
    digitalWrite(_motorPinA, LOW);
    digitalWrite(_motorPinB, LOW);
    delayMicroseconds(500);
    int valA = analogRead(_bemfPinA);
    int valB = analogRead(_bemfPinB);
    return abs(valA - valB);
}
