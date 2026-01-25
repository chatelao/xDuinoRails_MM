#ifndef CV_MANAGER_H
#define CV_MANAGER_H

#include <Arduino.h>

class CvManager {
public:
    CvManager();
    void setup();
    uint8_t getCv(uint16_t cv);
    void setCv(uint16_t cv, uint8_t value);

    // NMRA Standard CVs
    static const uint16_t CV_PRIMARY_ADDRESS = 1;
    static const uint16_t CV_START_VOLTAGE = 2;
    static const uint16_t CV_ACCELERATION_RATE = 3;
    static const uint16_t CV_DECELERATION_RATE = 4;
    static const uint16_t CV_MAX_SPEED = 5;
    static const uint16_t CV_MANUFACTURER_ID = 8;
    static const uint16_t CV_29_CONFIG = 29;


private:
    void readAllCvsFromEeprom();
    void writeCvToEeprom(uint16_t cv, uint8_t value);
    void initializeDefaultCvs();

    static const int SUPPORTED_CV_COUNT = 128;
    static const int EEPROM_MAGIC_ADDR = 511;
    static const uint8_t EEPROM_MAGIC_VALUE = 0xAB;

    uint8_t cvs[SUPPORTED_CV_COUNT + 1]; // CVs are 1-based, so we use index 1-128
};

#endif // CV_MANAGER_H
