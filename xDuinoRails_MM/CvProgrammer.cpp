#include "CvProgrammer.h"
#include <Arduino.h>

CvProgrammer::CvProgrammer(CvManager       *cvManager,
                           ProtocolHandler *protocolHandler) {
  this->cvManager               = cvManager;
  this->protocolHandler         = protocolHandler;
  this->programmingMode         = false;
  this->lastDirectionChangeTime = 0;
  this->lastSpeedChangeTime     = 0;
  this->directionChangeCount    = 0;
  this->cvAddress               = -1;
}

bool CvProgrammer::isProgrammingModeActive() { return programmingMode; }

void CvProgrammer::loop() {
  unsigned long lastChangeDirTs = protocolHandler->getLastChangeDirTs();
  if (lastChangeDirTs > 0 && lastChangeDirTs != lastDirectionChangeTime) {
    if (millis() - lastDirectionChangeTime < 2000) {
      directionChangeCount++;
    } else {
      directionChangeCount = 1;
    }
    lastDirectionChangeTime = lastChangeDirTs;

    if (directionChangeCount >= 4) {
      if (cvManager->getCv(CV_PROGRAMMING_LOCK) == 7) {
        programmingMode = !programmingMode;
      }
      directionChangeCount = 0;
    }
  }

  if (programmingMode) {
    unsigned long lastSpeedTs = protocolHandler->getLastSpeedChangeTs();
    if (lastSpeedTs > 0 && lastSpeedTs != lastSpeedChangeTime) {
      lastSpeedChangeTime = lastSpeedTs;
      int speed           = protocolHandler->getTargetSpeed();
      if (cvAddress == -1) {
        cvAddress = speed;
      } else {
        cvManager->setCv(cvAddress, speed);
        cvAddress       = -1;
        programmingMode = false;
      }
    }
  }
}
