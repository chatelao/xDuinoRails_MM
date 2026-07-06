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

  if (cv == CV_MANUFACTURER_ID || cv == CV_MOTOR_TYPE) {
#ifdef ARDUINO_ARCH_RP2040
    rp2040.reboot();
#elif defined(ARDUINO_ARCH_ESP32)
    ESP.restart();
#endif
  }
}

void CvManager::printAllCvs() {
  logger.println("--- Current CV Settings ---");
  logger.printf("CV 1 (Address): %d\n", getCv(CV_BASE_ADDRESS));
  logger.printf("CV 2 (Vstart): %d\n", getCv(CV_START_VOLTAGE));
  logger.printf("CV 3 (Acc): %d\n", getCv(CV_ACCELERATION));
  logger.printf("CV 4 (Dec): %d\n", getCv(CV_BRAKING_TIME));
  logger.printf("CV 5 (Vhigh): %d\n", getCv(CV_MAXIMUM_SPEED));
  logger.printf("CV 6 (Vmid): %d\n", getCv(CV_MEDIUM_SPEED));
  logger.printf("CV 7 (Version): %d\n", getCv(CV_VERSION));
  logger.printf("CV 8 (Manuf ID): %d\n", getCv(CV_MANUFACTURER_ID));
  logger.printf("CV 11 (Watchdog): %d\n", getCv(CV_WATCHDOG_TIMEOUT));
  logger.printf("CV 15 (Prog Lock): %d\n", getCv(CV_PROGRAMMING_LOCK));
  logger.printf("CV 17/18 (Long Addr): %d\n",
                (getCv(CV_LONG_ADDRESS_HIGH) << 8) | getCv(CV_LONG_ADDRESS_LOW));
  logger.printf("CV 29 (Config): %d\n", getCv(CV_CONFIGURATION));
  logger.printf("CV 33 (F0f): %d\n", getCv(CV_FRONT_LIGHT_F0F));
  logger.printf("CV 34 (F0r): %d\n", getCv(CV_REAR_LIGHT_F0R));
  logger.printf("CV 49 (BEMF Config): %d\n", getCv(CV_BEMF_CONFIG));
  logger.printf("CV 54 (BEMF K): %d\n", getCv(CV_BEMF_K));
  logger.printf("CV 55 (BEMF I): %d\n", getCv(CV_BEMF_I));
  logger.printf("CV 52 (Motor Type): %d\n", getCv(CV_MOTOR_TYPE));
  logger.printf("CV 107/108 (Ext ID): %d\n",
                (getCv(CV_EXT_ID_HIGH) << 8) | getCv(CV_EXT_ID_LOW));
  logger.printf("CV 250 (Debug): %d\n", getCv(CV_DEBUG_ENABLE));
  logger.println("---------------------------");
}

void CvManager::initCv() {
  if (EEPROM.read(EEPROM_MAGIC_BYTE_ADDR) != EEPROM_MAGIC_BYTE) {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, EEPROM_MAGIC_BYTE);
    EEPROM.write(CV_BASE_ADDRESS, 3);
    EEPROM.write(CV_START_VOLTAGE, 1);
    EEPROM.write(CV_ACCELERATION, 5);
    EEPROM.write(CV_BRAKING_TIME, 5);
    EEPROM.write(CV_MAXIMUM_SPEED, 255);
    EEPROM.write(CV_MEDIUM_SPEED, 127);
    EEPROM.write(CV_VERSION, 10);
    EEPROM.write(CV_MANUFACTURER_ID, 13);
    EEPROM.write(CV_WATCHDOG_TIMEOUT, 5);
    EEPROM.write(CV_LONG_ADDRESS_HIGH, 192);
    EEPROM.write(CV_LONG_ADDRESS_LOW, 100);
    EEPROM.write(CV_CONFIGURATION, 6);
    EEPROM.write(CV_FRONT_LIGHT_F0F, 1);
    EEPROM.write(CV_REAR_LIGHT_F0R, 2);
    EEPROM.write(CV_BEMF_CONFIG, 1);
    EEPROM.write(CV_BEMF_K, 32);
    EEPROM.write(CV_BEMF_I, 24);
    EEPROM.write(CV_MOTOR_TYPE, 0);
    EEPROM.write(CV_EXT_ID_HIGH, 1);
    EEPROM.write(CV_EXT_ID_LOW, 10);
    EEPROM.write(CV_PROGRAMMING_LOCK, 0);
    EEPROM.write(CV_DEBUG_ENABLE, 1);
    EEPROM.commit();
  }
}
