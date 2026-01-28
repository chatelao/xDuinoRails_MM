#ifndef MAERKLIN_MOTOROLA_H
#define MAERKLIN_MOTOROLA_H

#include <Arduino.h>

// Mock implementation of MaerklinMotorola.h for native testing
enum MM2DirectionState {
  MM2DirectionState_Forward,
  MM2DirectionState_Backward,
  MM2DirectionState_Unavailable
};

struct MaerklinMotorolaData {
  int               Address;
  int               Speed;
  bool              Function;
  bool              ChangeDir;
  bool              IsMM2;
  MM2DirectionState MM2Direction;
  int               MM2FunctionIndex;
  bool              IsMM2FunctionOn;
  bool              IsMagnet;
};

class MaerklinMotorola {
public:
  MaerklinMotorola(int pin){};
  void                 PinChange(){};
  void                 Parse(){};
  MaerklinMotorolaData *GetData();
  void SetData(int address, int speed, bool function, bool changeDir, bool isMM2,
               MM2DirectionState mm2Direction, int mm2FunctionIndex,
               bool isMM2FunctionOn);

private:
  MaerklinMotorolaData data;
};

#endif // MAERKLIN_MOTOROLA_H
