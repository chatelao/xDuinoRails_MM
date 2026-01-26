#ifndef EEPROM_H
#define EEPROM_H

#include <cstdint>
#include <vector>

class EEPROMClass {
public:
    void begin(size_t size) {
        _data.resize(size);
    }

    uint8_t read(int address) {
        return _data[address];
    }

    void write(int address, uint8_t value) {
        _data[address] = value;
    }

    void commit() {}

private:
    std::vector<uint8_t> _data;
};

extern EEPROMClass EEPROM;

#endif // EEPROM_H
