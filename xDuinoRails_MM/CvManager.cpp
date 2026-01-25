#include "CvManager.h"
#include <EEPROM.h>

CvManager::CvManager() {
}

void CvManager::setup() {
    EEPROM.begin(512);
    if (EEPROM.read(EEPROM_MAGIC_ADDR) != EEPROM_MAGIC_VALUE) {
        initializeDefaultCvs();
        EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
        EEPROM.commit();
    } else {
        readAllCvsFromEeprom();
    }
}

uint8_t CvManager::getCv(uint16_t cv) {
    if (cv < 1 || cv > SUPPORTED_CV_COUNT) {
        return 0;
    }
    return cvs[cv];
}

void CvManager::setCv(uint16_t cv, uint8_t value) {
    if (cv >= 1 && cv <= SUPPORTED_CV_COUNT) {
        cvs[cv] = value;
        writeCvToEeprom(cv, value);
    }
}

void CvManager::readAllCvsFromEeprom() {
    for (int i = 1; i <= SUPPORTED_CV_COUNT; i++) {
        cvs[i] = EEPROM.read(i - 1);
    }
}

void CvManager::writeCvToEeprom(uint16_t cv, uint8_t value) {
    EEPROM.write(cv - 1, value);
    EEPROM.commit();
}

void CvManager::initializeDefaultCvs() {
    // Initialize all CVs to 0
    for (int i = 1; i <= SUPPORTED_CV_COUNT; i++) {
        cvs[i] = 0;
    }

    // Set default values for important CVs
    setCv(CV_PRIMARY_ADDRESS, 3);
    setCv(CV_START_VOLTAGE, 2);
    setCv(CV_ACCELERATION_RATE, 10);
    setCv(CV_DECELERATION_RATE, 10);
    setCv(CV_MAX_SPEED, 255);
    setCv(CV_MANUFACTURER_ID, 151); // ESU
    setCv(CV_29_CONFIG, 6); // Analog mode enabled, Normal direction
}
