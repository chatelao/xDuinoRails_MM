#include "mocks/MaerklinMotorola.h"

static MaerklinMotorolaData mockData;
static bool                 dataAvailable = false;

MaerklinMotorolaData *MaerklinMotorola::GetData() {
  if (dataAvailable) {
    dataAvailable = false;
    return &mockData;
  }
  return nullptr;
}

void MaerklinMotorola::SetData(int address, int speed, bool function,
                               bool changeDir, bool isMM2,
                               MM2DirectionState mm2Direction,
                               int mm2FunctionIndex, bool isMM2FunctionOn) {
  mockData.Address         = address;
  mockData.Speed           = speed;
  mockData.Function        = function;
  mockData.ChangeDir       = changeDir;
  mockData.IsMM2           = isMM2;
  mockData.MM2Direction    = mm2Direction;
  mockData.MM2FunctionIndex = mm2FunctionIndex;
  mockData.IsMM2FunctionOn = isMM2FunctionOn;
  mockData.IsMagnet        = false;
  dataAvailable            = true;
}
