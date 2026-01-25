#include "ProtocolHandler.h"

extern ProtocolHandler protocol;

void isr_protocol() {
    protocol.mm.PinChange();
}

ProtocolHandler::ProtocolHandler(int address, int dccMmSignalPin)
    : mm(dccMmSignalPin), mmAddress(address), mmTimeoutMs(MM_TIMEOUT_MS), mm2LockTime(MM2_LOCK_TIME) {
    dccMmSignalPin_priv = dccMmSignalPin;
    lastCommandTime = 0;
    lastMM2Seen = 0;
    targetSpeed = 0;
    targetDirection = MM2DirectionState_Forward;
    stateF0 = false;
    stateF1 = false;
    stateF2 = false;
    lastChangeDirInput = false;
    lastChangeDirTs = 0;
}

void ProtocolHandler::setup() {
    attachInterrupt(digitalPinToInterrupt(dccMmSignalPin_priv), isr_protocol, CHANGE);
    lastCommandTime = millis();
}

void ProtocolHandler::loop() {
    mm.Parse();
    MaerklinMotorolaData* Data = mm.GetData();
    unsigned long now = millis();

    if (Data && !Data->IsMagnet && Data->Address == mmAddress) {
        lastCommandTime = now;

        bool mm2Locked = (now - lastMM2Seen < mm2LockTime);
        if (Data->IsMM2) lastMM2Seen = now;

        if (mm2Locked && Data->IsMM2) {
            if (Data->MM2Direction != MM2DirectionState_Unavailable) {
                targetDirection = Data->MM2Direction;
            }
            if (Data->MM2FunctionIndex == 1) stateF1 = Data->IsMM2FunctionOn;
            if (Data->MM2FunctionIndex == 2) stateF2 = Data->IsMM2FunctionOn;
        } else if (!mm2Locked) {
            if (Data->ChangeDir && !lastChangeDirInput) {
                if (now - lastChangeDirTs > 250) {
                    targetDirection = (targetDirection == MM2DirectionState_Forward)
                                      ? MM2DirectionState_Backward : MM2DirectionState_Forward;
                    lastChangeDirTs = now;
                }
            }
        }
        lastChangeDirInput = Data->ChangeDir;

        if (Data->IsMM2 && Data->MM2FunctionIndex != 0) {
            // No speed change on function only packets
        } else {
            targetSpeed = Data->Speed;
        }

        stateF0 = Data->Function;
    }
}

bool ProtocolHandler::isTimeout() {
    return millis() - lastCommandTime > mmTimeoutMs;
}

int ProtocolHandler::getTargetSpeed() {
    return targetSpeed;
}

MM2DirectionState ProtocolHandler::getTargetDirection() {
    return targetDirection;
}

bool ProtocolHandler::getFunctionState(int f) {
    if (f == 0) return stateF0;
    if (f == 1) return stateF1;
    if (f == 2) return stateF2;
    return false;
}

bool ProtocolHandler::isMm2Locked() {
    return millis() - lastMM2Seen < mm2LockTime;
}
