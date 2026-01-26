#ifndef I_MOTOR_HARDWARE_H
#define I_MOTOR_HARDWARE_H

#include <MaerklinMotorola.h>

class IMotorHardware {
public:
    virtual ~IMotorHardware() {}
    virtual void writeMotor(int pwm, MM2DirectionState dir) = 0;
    virtual int readBemf() = 0;
};

#endif // I_MOTOR_HARDWARE_H
