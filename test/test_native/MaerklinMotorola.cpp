#include "mocks/MaerklinMotorola.h"

MaerklinMotorola::MaerklinMotorola(int pin) { this->dataAvailable = false; }

bool MaerklinMotorola::hasChanged() { return this->dataAvailable; }

MaerklinMotorolaData *MaerklinMotorola::GetData() {
  if (this->dataAvailable) {
    this->dataAvailable = false;
    return &this->data;
  }
  return nullptr;
}

void MaerklinMotorola::SetData(int address, int speed, bool function,
                               bool changeDir, bool isMM2,
                               MM2DirectionState mm2Direction,
                               int mm2FunctionIndex, bool isMM2FunctionOn) {
  this->data.Address          = address;
  this->data.Speed            = speed;
  this->data.Function         = function;
  this->data.ChangeDir        = changeDir;
  this->data.IsMM2            = isMM2;
  this->data.MM2Direction     = mm2Direction;
  this->data.MM2FunctionIndex = mm2FunctionIndex;
  this->data.IsMM2FunctionOn  = isMM2FunctionOn;
  this->data.IsMagnet         = false;
  this->dataAvailable         = true;
}
