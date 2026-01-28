#ifndef CV_MANAGER_MOCK_H
#define CV_MANAGER_MOCK_H

#include "CvManager.h"
#include <map>

class CvManagerMock : public CvManager {
public:
  CvManagerMock() {
    // Seed with default CVs
    cvStore[CV_BASE_ADDRESS] = 3;
    cvStore[CV_START_VOLTAGE] = 1;
    cvStore[CV_ACCELERATION] = 5;
    cvStore[CV_BRAKING_TIME] = 5;
    cvStore[CV_MAXIMUM_SPEED] = 0;
    cvStore[CV_VERSION] = 10;
    cvStore[CV_MANUFACTURER_ID] = 13;
    cvStore[CV_LONG_ADDRESS_HIGH] = 192;
    cvStore[CV_LONG_ADDRESS_LOW] = 100;
    cvStore[CV_CONFIGURATION] = 6;
    cvStore[CV_FRONT_LIGHT_F0F] = 1;
    cvStore[CV_REAR_LIGHT_F0R] = 2;
    cvStore[CV_EXT_ID_HIGH] = 1;
    cvStore[CV_EXT_ID_LOW] = 10;
    cvStore[CV_MOTOR_TYPE] = 0;
  }

  uint8_t getCv(int cv) override {
    if (cvStore.count(cv)) {
      return cvStore[cv];
    }
    return 0;
  }

  void setCv(int cv, uint8_t value) override {
    cvStore[cv] = value;
    lastSetCv = cv;
    lastSetValue = value;
  }

  void setup() override {
    // Do nothing in mock
  }

  std::map<int, uint8_t> cvStore;
  int lastSetCv = -1;
  uint8_t lastSetValue = 0;
};

#endif // CV_MANAGER_MOCK_H
