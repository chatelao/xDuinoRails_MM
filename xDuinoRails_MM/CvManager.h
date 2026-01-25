#ifndef CV_MANAGER_H
#define CV_MANAGER_H

#include <Arduino.h>

class CvManager {
 public:
  CvManager();
  void setup();
  uint8_t getCv(int cv);
  void setCv(int cv, uint8_t value);

 private:
  void initCv();
};

#endif  // CV_MANAGER_H
