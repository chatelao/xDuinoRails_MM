#ifndef EEPROM_H
#define EEPROM_H

#include "IEeprom.h"
#include <EEPROM.h>

class Eeprom : public IEeprom {
 public:
  void begin(size_t size) override;
  uint8_t read(int address) override;
  void write(int address, uint8_t value) override;
  bool commit() override;
};

#endif  // EEPROM_H
