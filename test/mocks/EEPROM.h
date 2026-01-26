#ifndef EEPROM_H
#define EEPROM_H

#include <cstdint>

class EEPROMClass {
public:
    uint8_t read(int address);
    void write(int address, uint8_t value);
    void commit();
    void begin(size_t size);
};

extern EEPROMClass EEPROM;

#endif // EEPROM_H
