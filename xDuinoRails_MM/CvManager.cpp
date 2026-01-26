#include "CvManager.h"
#include <EEPROM.h>

const int CV_ADDRESS = 1;
const uint8_t EEPROM_MAGIC_BYTE = 0xAF;
const int EEPROM_MAGIC_BYTE_ADDR = 0;

CvManager::CvManager() {}

void CvManager::setup() {
  EEPROM.begin(512);
  initCv();
}

uint8_t CvManager::getCv(int cv) {
  return EEPROM.read(cv);
}

void CvManager::setCv(int cv, uint8_t value) {
  EEPROM.write(cv, value);
  EEPROM.commit();
}

void CvManager::initCv() {
  if (EEPROM.read(EEPROM_MAGIC_BYTE_ADDR) != EEPROM_MAGIC_BYTE) {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, EEPROM_MAGIC_BYTE);
    setCv(CV_ADDRESS, 24);
    EEPROM.commit();
  }
}
