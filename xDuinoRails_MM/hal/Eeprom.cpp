#ifndef PIO_UNIT_TESTING

#include "Eeprom.h"

void Eeprom::begin(size_t size) {
  EEPROM.begin(size);
}

uint8_t Eeprom::read(int address) {
  return EEPROM.read(address);
}

void Eeprom::write(int address, uint8_t value) {
  EEPROM.write(address, value);
}

bool Eeprom::commit() {
  return EEPROM.commit();
}

#endif // PIO_UNIT_TESTING
