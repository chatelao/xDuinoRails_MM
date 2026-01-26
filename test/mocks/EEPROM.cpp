#include "EEPROM.h"

uint8_t EEPROMClass::read(int address) { return 0; }
void EEPROMClass::write(int address, uint8_t value) {}
void EEPROMClass::commit() {}
void EEPROMClass::begin(size_t size) {}

EEPROMClass EEPROM;
