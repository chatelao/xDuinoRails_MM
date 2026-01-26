#ifndef I_EEPROM_H
#define I_EEPROM_H

#include <stdint.h>
#include <cstddef>

class IEeprom {
 public:
  virtual ~IEeprom() = default;
  virtual void begin(size_t size) = 0;
  virtual uint8_t read(int address) = 0;
  virtual void write(int address, uint8_t value) = 0;
  virtual bool commit() = 0;
};

#endif  // I_EEPROM_H
