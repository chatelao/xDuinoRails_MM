#include "ProtocolHandler.h"
#include "CvManager.h"

// Forward declarations for DCC callbacks
void notifyDccSpeed128(uint16_t Addr, DCC_Speed_Direction_t Speed);
void notifyDccFunc(uint16_t Addr, FN_GROUP Func_Grp, uint8_t Func_State);
void notifyCVAck(void);

// External instances from the main .ino file
extern ProtocolHandler* protocol;
extern CvManager cvManager;

// This is the new ISR function that will be called from the wrapper
void ProtocolHandler::isr() {
    mm.PinChange();
    dcc.process();
}

ProtocolHandler::ProtocolHandler(int address, int dccMmSignalPin)
    : mm(dccMmSignalPin), dcc(), mmAddress(address), mmTimeoutMs(MM_TIMEOUT_MS), mm2LockTime(MM2_LOCK_TIME) {
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
    lastCommandTime = millis();

    // Configure DCC callbacks and initialize the library
    dcc.setCVReadHandler(cvManager.getCv);
    dcc.setCVWriteHandler(cvManager.setCv);
    dcc.setSpeed128A(notifyDccSpeed128);
    dcc.setFunctionGroupA(FN_GROUP_0, notifyDccFunc);
    dcc.setFunctionGroupA(FN_GROUP_1, notifyDccFunc);
    dcc.setFunctionGroupA(FN_GROUP_2, notifyDccFunc);
    dcc.init(MAN_ID_DIY, 10, FLAGS_OUTPUT_ADDRESS_MODE, 0);
}

void ProtocolHandler::loop() {
    // Process Motorola signal
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

// ==========================================
// DCC Callback Implementations
// ==========================================
void notifyDccSpeed128(uint16_t Addr, DCC_Speed_Direction_t Speed) {
    if (!protocol) return;
    protocol->lastCommandTime = millis();

    protocol->targetDirection = (Speed.Direction == 1) ? MM2DirectionState_Forward : MM2DirectionState_Backward;

    if (Speed.Speed_steps <= 1) { // Stop or E-Stop
        protocol->targetSpeed = 0;
    } else {
        // Map DCC 128 steps (2-127) to MM 14 steps (1-14)
        protocol->targetSpeed = map(Speed.Speed_steps, 2, 127, 1, 14);
    }
}

void notifyDccFunc(uint16_t Addr, FN_GROUP Func_Grp, uint8_t Func_State) {
    if (!protocol) return;
    protocol->lastCommandTime = millis();

    switch (Func_Grp) {
        case FN_GROUP_0: // F0
            protocol->stateF0 = (Func_State & FN_BIT_F0) ? true : false;
            break;
        case FN_GROUP_1: // F1-F4
            protocol->stateF1 = (Func_State & FN_BIT_F1) ? true : false;
            protocol->stateF2 = (Func_State & FN_BIT_F2) ? true : false;
            break;
        case FN_GROUP_2: // F5-F8
            break;
    }
}

void notifyCVAck(void) {
    // Acknowledge CV programming - can be used for feedback
}
