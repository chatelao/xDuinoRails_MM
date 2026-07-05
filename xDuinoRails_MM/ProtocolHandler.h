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
  bool              isSignalTimeout();
  unsigned long     getLastSignalTime();
  void              setSignalTimeout(int timeoutMs);
  int               getTargetSpeed();
  void              setTargetSpeed(int speed);
  MM2DirectionState getTargetDirection();
  void              setTargetDirection(MM2DirectionState direction);
  bool              getFunctionState(int f);
  void              setFunctionState(int f, bool state);
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
  int               signalTimeoutMs;
  int               mm2LockTime;
  unsigned long     lastCommandTime;
  unsigned long     lastSignalTime;
  unsigned long     lastMM2Seen;
  int               targetSpeed;
  MM2DirectionState targetDirection;
  bool              stateF0;
  bool              stateF1;
  bool              stateF2;
  bool              lastChangeDirInput;
  unsigned long     lastChangeDirTs;
  unsigned long     lastSpeedChangeTs;

  // Last reported states for logging
  int               lastTargetSpeed;
  MM2DirectionState lastTargetDirection;
  bool              lastStateF0;
  bool              lastStateF1;
  bool              lastStateF2;
  bool              wasMm2Locked;
  bool              wasSignalTimeout;
};

#endif // PROTOCOL_HANDLER_H
