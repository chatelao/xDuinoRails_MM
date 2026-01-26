#include "CvManager.h"
#ifndef PIO_UNIT_TESTING
#include <EEPROM.h>
#endif

const int CV_ADDRESS = 1;
const uint8_t EEPROM_MAGIC_BYTE = 0xAF;
const int EEPROM_MAGIC_BYTE_ADDR = 0;

CvManager::CvManager() {}

void CvManager::setup() {
#ifndef PIO_UNIT_TESTING
  EEPROM.begin(512);
  initCv();
#endif
}

uint8_t CvManager::getCv(int cv) {
#ifndef PIO_UNIT_TESTING
  return EEPROM.read(cv);
#else
  return 1; // Return a default value for testing
#endif
}

void CvManager::setCv(int cv, uint8_t value) {
#ifndef PIO_UNIT_TESTING
  EEPROM.write(cv, value);
  EEPROM.commit();
#endif
}

void CvManager::initCv() {
#ifndef PIO_UNIT_TESTING
  if (EEPROM.read(EEPROM_MAGIC_BYTE_ADDR) != EEPROM_MAGIC_BYTE) {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, EEPROM_MAGIC_BYTE);
    setCv(CV_ADDRESS, 24);
    EEPROM.commit();
  }
#endif
}
