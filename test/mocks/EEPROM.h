#ifndef EEPROM_H
#define EEPROM_H

#include <cstdint>
#include <vector>

class EEPROMClass {
 public:
  void begin(size_t size) {
    if (size > _data.size()) {
      _data.resize(size);
    }
  }

  uint8_t read(int address) {
    if (address >= _data.size()) {
      return 0;
    }
    return _data[address];
  }

  void write(int address, uint8_t value) {
    if (address >= _data.size()) {
      _data.resize(address + 1);
    }
    _data[address] = value;
  }

  void put(int address, uint8_t value) { write(address, value); }

  void commit() {}

 private:
  std::vector<uint8_t> _data;
};

extern EEPROMClass EEPROM;

#endif // EEPROM_H
