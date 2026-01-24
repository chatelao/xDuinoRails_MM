#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <Arduino.h>
#include <MaerklinMotorola.h>

class ProtocolHandler {
public:
    ProtocolHandler(int address, int timeout, int mm2LockTime);
    void setup();
    void loop();
    bool isTimeout();
    int getTargetSpeed();
    MM2DirectionState getTargetDirection();
    bool getFunctionState(int f);
    bool isMm2Locked();

    MaerklinMotorola mm;

private:
    int mmAddress;
    int mmTimeoutMs;
    int mm2LockTime;
    unsigned long lastCommandTime;
    unsigned long lastMM2Seen;
    int targetSpeed;
    MM2DirectionState targetDirection;
    bool stateF0;
    bool stateF1;
    bool stateF2;
    bool lastChangeDirInput;
    unsigned long lastChangeDirTs;
};

#endif // PROTOCOL_HANDLER_H
