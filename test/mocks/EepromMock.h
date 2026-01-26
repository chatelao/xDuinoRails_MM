#ifndef EEPROM_MOCK_H
#define EEPROM_MOCK_H

#include "hal/IEeprom.h"
#include <map>

class EepromMock : public IEeprom {
 public:
  EepromMock() = default;
  ~EepromMock() override = default;

  void begin(size_t size) override {
    // In-memory storage for the mock
    storage.clear();
  }

  uint8_t read(int address) override {
    if (storage.find(address) != storage.end()) {
      return storage[address];
    }
    return 0; // Default value if not set
  }

  void write(int address, uint8_t value) override {
    storage[address] = value;
  }

  bool commit() override {
    // No real commit action needed for the mock
    return true;
  }

 private:
  std::map<int, uint8_t> storage;
};

#endif  // EEPROM_MOCK_H
