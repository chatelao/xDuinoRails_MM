#include "CvManager.h"

const int CV_ADDRESS = 1;
const uint8_t EEPROM_MAGIC_BYTE = 0xAF;
const int EEPROM_MAGIC_BYTE_ADDR = 0;

CvManager::CvManager(IEeprom& eeprom) : _eeprom(eeprom) {}

void CvManager::setup(size_t eepromSize) {
  _eeprom.begin(eepromSize);
  initCv();
}

uint8_t CvManager::getCv(int cv) {
  return _eeprom.read(cv);
}

void CvManager::setCv(int cv, uint8_t value) {
  _eeprom.write(cv, value);
  _eeprom.commit();
}

void CvManager::initCv() {
  if (_eeprom.read(EEPROM_MAGIC_BYTE_ADDR) != EEPROM_MAGIC_BYTE) {
    _eeprom.write(EEPROM_MAGIC_BYTE_ADDR, EEPROM_MAGIC_BYTE);
    setCv(CV_ADDRESS, 24);
    _eeprom.commit();
  }
}
