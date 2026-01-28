#include "ProtocolHandler.h"

#ifndef PIO_UNIT_TESTING
extern ProtocolHandler protocol;

void isr_protocol() { protocol.mm.PinChange(); }
#endif

ProtocolHandler::ProtocolHandler(int dccMmSignalPin)
    : mm(dccMmSignalPin), mmTimeoutMs(MM_TIMEOUT_MS),
      mm2LockTime(MM2_LOCK_TIME) {
  dccMmSignalPin_priv = dccMmSignalPin;
  lastCommandTime     = 0;
  lastMM2Seen         = 0;
  targetSpeed         = 0;
  targetDirection     = MM2DirectionState_Forward;
  stateF0             = false;
  stateF1             = false;
  stateF2             = false;
  lastChangeDirInput  = false;
  lastChangeDirTs     = 0;
  lastSpeedChangeTs   = 0;
  mm2SeenEver         = false;
}

void ProtocolHandler::setup() {
#ifndef PIO_UNIT_TESTING
  attachInterrupt(digitalPinToInterrupt(dccMmSignalPin_priv), isr_protocol,
                  CHANGE);
#endif
  lastCommandTime = millis();
}

void ProtocolHandler::setAddress(int address) { mmAddress = address; }

void ProtocolHandler::loop() {
  mm.Parse();
  MaerklinMotorolaData *Data = mm.GetData();
  unsigned long         now  = millis();

  if (Data && !Data->IsMagnet && Data->Address == mmAddress) {
    lastCommandTime = now;

    if (Data->IsMM2) {
      lastMM2Seen = now;
      mm2SeenEver = true;
    }

    bool mm2Locked = mm2SeenEver && (now - lastMM2Seen < mm2LockTime);

    if (mm2Locked && Data->IsMM2) {
      if (Data->MM2Direction != MM2DirectionState_Unavailable) {
        targetDirection = Data->MM2Direction;
      }
      if (Data->MM2FunctionIndex == 1)
        stateF1 = Data->IsMM2FunctionOn;
      if (Data->MM2FunctionIndex == 2)
        stateF2 = Data->IsMM2FunctionOn;
    } else if (!mm2Locked) {
      if (Data->ChangeDir && !lastChangeDirInput) {
        if (lastChangeDirTs == 0 || now - lastChangeDirTs > 250) {
          targetDirection = (targetDirection == MM2DirectionState_Forward)
                                ? MM2DirectionState_Backward
                                : MM2DirectionState_Forward;
          lastChangeDirTs = now;
        }
      }
    }
    lastChangeDirInput = Data->ChangeDir;

    if (Data->IsMM2 && Data->MM2FunctionIndex != 0) {
      // No speed change on function only packets
    } else {
      if (targetSpeed != Data->Speed) {
        lastSpeedChangeTs = now;
      }
      targetSpeed = Data->Speed;
    }

    stateF0 = Data->Function;
  }
}

bool ProtocolHandler::isTimeout() {
  return millis() - lastCommandTime > mmTimeoutMs;
}

int ProtocolHandler::getTargetSpeed() { return targetSpeed; }

MM2DirectionState ProtocolHandler::getTargetDirection() {
  return targetDirection;
}

bool ProtocolHandler::getFunctionState(int f) {
  if (f == 0)
    return stateF0;
  if (f == 1)
    return stateF1;
  if (f == 2)
    return stateF2;
  return false;
}

bool ProtocolHandler::isMm2Locked() {
  return mm2SeenEver && (millis() - lastMM2Seen < mm2LockTime);
}

unsigned long ProtocolHandler::getLastChangeDirTs() { return lastChangeDirTs; }

unsigned long ProtocolHandler::getLastSpeedChangeTs() {
  return lastSpeedChangeTs;
}
