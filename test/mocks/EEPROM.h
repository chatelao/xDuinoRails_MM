#ifndef EEPROM_H
#define EEPROM_H

#include <cstdint>

class EEPROMClass {
public:
  void                    begin(int size);
  void                    commit();
  uint8_t                 read(int address);
  void                    write(int address, uint8_t value);
  template <typename T> T get(int address, T &data) {
    // A simple mock implementation
    return data;
  }
  template <typename T> const T &put(int address, const T &data) {
    // A simple mock implementation
    return data;
  }
};

extern EEPROMClass EEPROM;

#endif // EEPROM_H
