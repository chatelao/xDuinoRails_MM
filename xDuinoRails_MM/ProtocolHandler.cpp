#include "ProtocolHandler.h"
#include "Logger.h"

#ifndef PIO_UNIT_TESTING
extern ProtocolHandler protocol;

void isr_protocol() { protocol.mm.PinChange(); }
#endif

ProtocolHandler::ProtocolHandler(int dccMmSignalPin)
    : mm(dccMmSignalPin), mmTimeoutMs(MM_TIMEOUT_MS), signalTimeoutMs(500),
      mm2LockTime(MM2_LOCK_TIME) {
  dccMmSignalPin_priv = dccMmSignalPin;
  lastCommandTime     = 0;
  lastSignalTime      = 0;
  lastMM2Seen         = 0;
  targetSpeed         = 0;
  targetDirection     = MM2DirectionState_Forward;
  stateF0             = false;
  stateF1             = false;
  stateF2             = false;
  lastChangeDirInput  = false;
  lastChangeDirTs     = 0;
  lastSpeedChangeTs   = 0;

  lastTargetSpeed     = 0;
  lastTargetDirection = MM2DirectionState_Forward;
  lastStateF0         = false;
  lastStateF1         = false;
  lastStateF2         = false;
  wasMm2Locked        = false;
  wasSignalTimeout    = false;
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

  if (Data) {
    lastSignalTime = now;
  }

  bool signalTimeout = isSignalTimeout();
  if (signalTimeout != wasSignalTimeout) {
    if (signalTimeout) {
      logger.println("MM Signal lost");
    } else {
      logger.println("MM Signal recovered");
    }
    wasSignalTimeout = signalTimeout;
  }

  if (Data && !Data->IsMagnet && Data->Address == mmAddress) {
    lastCommandTime = now;

    bool mm2Locked = (lastMM2Seen > 0 && now - lastMM2Seen < mm2LockTime);

    if (Data->IsMM2) {
      lastMM2Seen = now;
      if (Data->MM2Direction != MM2DirectionState_Unavailable) {
        targetDirection = Data->MM2Direction;
      }
      if (Data->MM2FunctionIndex == 1)
        stateF1 = Data->IsMM2FunctionOn;
      if (Data->MM2FunctionIndex == 2)
        stateF2 = Data->IsMM2FunctionOn;
    } else if (!mm2Locked) {
      if (Data->ChangeDir && !lastChangeDirInput) {
        if (now - lastChangeDirTs > 250) {
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

    if (targetSpeed != lastTargetSpeed || targetDirection != lastTargetDirection ||
        stateF0 != lastStateF0 || stateF1 != lastStateF1 ||
        stateF2 != lastStateF2 || mm2Locked != wasMm2Locked) {

      logger.printf("State: Addr=%d, Speed=%d, Dir=%s, F0=%d, F1=%d, F2=%d, "
                    "MM2Lock=%d\n",
                    mmAddress, targetSpeed,
                    targetDirection == MM2DirectionState_Forward ? "F" : "B",
                    stateF0, stateF1, stateF2, mm2Locked);

      lastTargetSpeed     = targetSpeed;
      lastTargetDirection = targetDirection;
      lastStateF0         = stateF0;
      lastStateF1         = stateF1;
      lastStateF2         = stateF2;
      wasMm2Locked        = mm2Locked;
    }
  }
}

bool ProtocolHandler::isTimeout() {
  return millis() - lastCommandTime > mmTimeoutMs;
}

bool ProtocolHandler::isSignalTimeout() {
  return millis() - lastSignalTime > signalTimeoutMs;
}

unsigned long ProtocolHandler::getLastSignalTime() { return lastSignalTime; }

void ProtocolHandler::setSignalTimeout(int timeoutMs) {
  signalTimeoutMs = timeoutMs;
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
  return millis() - lastMM2Seen < mm2LockTime;
}

unsigned long ProtocolHandler::getLastChangeDirTs() { return lastChangeDirTs; }

unsigned long ProtocolHandler::getLastSpeedChangeTs() {
  return lastSpeedChangeTs;
}
