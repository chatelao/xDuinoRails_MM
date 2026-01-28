#include "EEPROM.h"
#include <vector>

static std::vector<uint8_t> eeprom_memory;

EEPROMClass EEPROM;

void EEPROMClass::begin(int size) {
  eeprom_memory.assign(size, 0xFF);
}

void EEPROMClass::commit() {
  // Mock implementation
}

uint8_t EEPROMClass::read(int address) {
  if (address < eeprom_memory.size()) {
    return eeprom_memory[address];
  }
  return 0;
}

void EEPROMClass::write(int address, uint8_t value) {
  if (address < eeprom_memory.size()) {
    eeprom_memory[address] = value;
  }
}
