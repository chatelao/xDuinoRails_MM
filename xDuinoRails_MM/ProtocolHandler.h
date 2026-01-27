#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <Arduino.h>
#include <MaerklinMotorola.h>

class ProtocolHandler {
public:
  ProtocolHandler(int dccMmSignalPin);
  void              setup();
  void              loop();
  void              setAddress(int address);
  bool              isTimeout();
  int               getTargetSpeed();
  MM2DirectionState getTargetDirection();
  bool              getFunctionState(int f);
  bool              isMm2Locked();
  unsigned long     getLastChangeDirTs();
  unsigned long     getLastSpeedChangeTs();

  MaerklinMotorola mm;

private:
  static const int  MM_TIMEOUT_MS = 1500;
  static const int  MM2_LOCK_TIME = 5000;
  int               mmAddress;
  int               dccMmSignalPin_priv;
  int               mmTimeoutMs;
  int               mm2LockTime;
  unsigned long     lastCommandTime;
  unsigned long     lastMM2Seen;
  int               targetSpeed;
  MM2DirectionState targetDirection;
  bool              stateF0;
  bool              stateF1;
  bool              stateF2;
  bool              lastChangeDirInput;
  unsigned long     lastChangeDirTs;
  unsigned long     lastSpeedChangeTs;
};

#endif // PROTOCOL_HANDLER_H
