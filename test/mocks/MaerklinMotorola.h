#ifndef MAERKLIN_MOTOROLA_H
#define MAERKLIN_MOTOROLA_H

#include <cstdint>

class MaerklinMotorola {
public:
    MaerklinMotorola(uint8_t pin);
    void setup();
    void loop();
    bool isTimeout();
    uint8_t getTargetSpeed();
    uint8_t getTargetDirection();
    bool getFunctionState(uint8_t func);
    bool isMm2Locked();
    void setAddress(uint8_t addr);
};

#endif // MAERKLIN_MOTOROLA_H
