#include "CvManager.h"
#include "Logger.h"
#include <EEPROM.h>
#ifdef ARDUINO_ARCH_RP2040
#include <RP2040.h>
#endif

const uint8_t EEPROM_MAGIC_BYTE      = 0xAF;
const int     EEPROM_MAGIC_BYTE_ADDR = 0;

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
  if (cv == CV_MANUFACTURER_ID && (value == 0 || value == 8)) {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, 0);
    initCv();

#ifdef ARDUINO_ARCH_RP2040
    rp2040.reboot();
#elif defined(ARDUINO_ARCH_ESP32)
    ESP.restart();
#endif
    return;
  }
  EEPROM.write(cv, value);
  EEPROM.commit();

  logger.printf("CV %d set to %d\n", cv, value);

  if (cv == CV_MANUFACTURER_ID) {
#ifdef ARDUINO_ARCH_RP2040
    rp2040.reboot();
#elif defined(ARDUINO_ARCH_ESP32)
    ESP.restart();
#endif
  }
}

void CvManager::initCv() {
  if (EEPROM.read(EEPROM_MAGIC_BYTE_ADDR) != EEPROM_MAGIC_BYTE) {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, EEPROM_MAGIC_BYTE);
    EEPROM.write(CV_BASE_ADDRESS, 3);
    EEPROM.write(CV_START_VOLTAGE, 1);
    EEPROM.write(CV_ACCELERATION, 5);
    EEPROM.write(CV_BRAKING_TIME, 5);
    EEPROM.write(CV_MAXIMUM_SPEED, 0);
    EEPROM.write(CV_VERSION, 10);
    EEPROM.write(CV_MANUFACTURER_ID, 13);
    EEPROM.write(CV_WATCHDOG_TIMEOUT, 5);
    EEPROM.write(CV_LONG_ADDRESS_HIGH, 192);
    EEPROM.write(CV_LONG_ADDRESS_LOW, 100);
    EEPROM.write(CV_CONFIGURATION, 6);
    EEPROM.write(CV_FRONT_LIGHT_F0F, 1);
    EEPROM.write(CV_REAR_LIGHT_F0R, 2);
    EEPROM.write(CV_MOTOR_TYPE, 0);
    EEPROM.write(CV_EXT_ID_HIGH, 1);
    EEPROM.write(CV_EXT_ID_LOW, 10);
    EEPROM.write(CV_PROGRAMMING_LOCK, 0);
    EEPROM.write(CV_DEBUG_ENABLE, 0);
    EEPROM.commit();
  }
}
