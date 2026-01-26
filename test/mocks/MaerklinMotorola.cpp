#include "MaerklinMotorola.h"

MaerklinMotorola::MaerklinMotorola(uint8_t pin) {}
void MaerklinMotorola::setup() {}
void MaerklinMotorola::loop() {}
bool MaerklinMotorola::isTimeout() { return false; }
uint8_t MaerklinMotorola::getTargetSpeed() { return 0; }
uint8_t MaerklinMotorola::getTargetDirection() { return 0; }
bool MaerklinMotorola::getFunctionState(uint8_t func) { return false; }
bool MaerklinMotorola::isMm2Locked() { return false; }
void MaerklinMotorola::setAddress(uint8_t addr) {}
