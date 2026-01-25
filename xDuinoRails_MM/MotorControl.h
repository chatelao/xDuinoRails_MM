#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <MaerklinMotorola.h>

class MotorControl {
public:
    MotorControl(int motorType);
    void setup();
    void update(int targetPwm, MM2DirectionState targetDir);
    void stop();
    bool isKickstarting();
    MM2DirectionState getCurrentDirection();

private:
    void writeMotorHardware(int pwm, MM2DirectionState dir);
    int readBEMF();

    int motorType;
    int targetPwm;
    MM2DirectionState currDirection;
    MM2DirectionState targetDirection;
    bool isKickstarting_priv;
    unsigned long kickstartBegin;
    unsigned long lastBemfMeasure;
    int lastSpeed;
    int previousPwm;

    // Motor parameters - will be set based on motorType
    int PWM_FREQ;
    int PWM_MIN_MOVING;
    int KICK_PWM;
    int KICK_MAX_TIME;
    int BEMF_THRESHOLD;
    int BEMF_SAMPLE_INT;
};

#endif // MOTOR_CONTROL_H
