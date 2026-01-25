#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <MaerklinMotorola.h>
#include "CvManager.h"

class MotorControl {
public:
    MotorControl(int motorType, CvManager& cvManager);
    void setup();
    void update(int targetPwm, MM2DirectionState targetDir);
    void stop();
    bool isKickstarting();
    MM2DirectionState getCurrentDirection();

private:
    void writeMotorHardware(int pwm, MM2DirectionState dir);
    int readBEMF();

    CvManager& cvManager;
    int motorType;
    int targetPwm;
    MM2DirectionState currDirection;
    MM2DirectionState targetDirection;
    bool isKickstarting_priv;
    unsigned long kickstartBegin;
    unsigned long lastBemfMeasure;
    int lastSpeed;

    // Motor parameters will be loaded from CVs
};

#endif // MOTOR_CONTROL_H
