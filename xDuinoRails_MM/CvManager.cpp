#include "CvManager.h"
#include <EEPROM.h>
#include <RP2040.h>

const uint8_t EEPROM_MAGIC_BYTE = 0xAF;
const int EEPROM_MAGIC_BYTE_ADDR = 0;

CvManager::CvManager() {}

void CvManager::setup() {
  EEPROM.begin(512);
  initCv();
}

uint8_t CvManager::getCv(int cv) { return EEPROM.read(cv); }

void CvManager::setCv(int cv, uint8_t value) {
  if (cv == CV_VERSION) {
    return;
  }
  EEPROM.write(cv, value);
  EEPROM.commit();

  if (cv == CV_MANUFACTURER_ID) {
    rp2040.reboot();
  }
}

void CvManager::initCv() {
  if (EEPROM.read(EEPROM_MAGIC_BYTE_ADDR) != EEPROM_MAGIC_BYTE) {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, EEPROM_MAGIC_BYTE);
    setCv(CV_BASE_ADDRESS, 3);
    setCv(CV_START_VOLTAGE, 1);
    setCv(CV_ACCELERATION, 5);
    setCv(CV_BRAKING_TIME, 5);
    setCv(CV_MAXIMUM_SPEED, 0);
    EEPROM.write(CV_VERSION, 10);
    setCv(CV_MANUFACTURER_ID, 13);
    setCv(CV_LONG_ADDRESS_HIGH, 192);
    setCv(CV_LONG_ADDRESS_LOW, 100);
    setCv(CV_CONFIGURATION, 6);
    setCv(CV_FRONT_LIGHT_F0F, 1);
    setCv(CV_REAR_LIGHT_F0R, 2);
    setCv(CV_MOTOR_TYPE, 0);
    setCv(CV_EXT_ID_HIGH, 1);
    setCv(CV_EXT_ID_LOW, 10);
    EEPROM.commit();
  }
}
