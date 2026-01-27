#ifndef CV_MANAGER_H
#define CV_MANAGER_H

#include <Arduino.h>

// CV Adddresses
constexpr int CV_BASE_ADDRESS = 1;
constexpr int CV_START_VOLTAGE = 2;
constexpr int CV_ACCELERATION = 3;
constexpr int CV_BRAKING_TIME = 4;
constexpr int CV_MAXIMUM_SPEED = 5;
constexpr int CV_VERSION = 7;
constexpr int CV_MANUFACTURER_ID = 8; // Writing 8 to this CV resets the decoder
constexpr int CV_PROGRAMMING_LOCK = 15;
constexpr int CV_LONG_ADDRESS_HIGH = 17;
constexpr int CV_LONG_ADDRESS_LOW = 18;
constexpr int CV_CONFIGURATION = 29;
constexpr int CV_FRONT_LIGHT_F0F = 33;
constexpr int CV_REAR_LIGHT_F0R = 34;
constexpr int CV_MOTOR_TYPE = 52;
constexpr int CV_EXT_ID_HIGH = 107;
constexpr int CV_EXT_ID_LOW = 108;

class CvManager {
 public:
  CvManager();
  virtual void setup();
  virtual uint8_t getCv(int cv);
  virtual void setCv(int cv, uint8_t value);

 private:
  void initCv();
};

#endif  // CV_MANAGER_H
