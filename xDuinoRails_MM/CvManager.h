#ifndef CV_MANAGER_H
#define CV_MANAGER_H

#include "hal/IEeprom.h"

class CvManager {
 public:
  explicit CvManager(IEeprom& eeprom);
  void setup(size_t eepromSize);
  uint8_t getCv(int cv);
  void setCv(int cv, uint8_t value);

 private:
  IEeprom& _eeprom;
  void initCv();
};

#endif  // CV_MANAGER_H
