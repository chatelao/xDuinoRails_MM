#include "CvProgrammer.h"
#include "Logger.h"
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

void CvProgrammer::loop() {
  unsigned long lastChangeDirTs = protocolHandler->getLastChangeDirTs();
  if (lastChangeDirTs > 0 && lastChangeDirTs != lastDirectionChangeTime) {
    if (millis() - lastDirectionChangeTime < 2000) {
      directionChangeCount++;
      logger.printf(LogCategory::CV, "Prog: ChangeDir count %d\n",
                    directionChangeCount);
    } else {
      directionChangeCount = 1;
    }
    lastDirectionChangeTime = lastChangeDirTs;

    if (directionChangeCount >= 4) {
      if (cvManager->getCv(CV_PROGRAMMING_LOCK) == 7) {
        programmingMode = !programmingMode;
        logger.printf(LogCategory::CV, "Prog: Programming Mode %s\n",
                      programmingMode ? "Enabled" : "Disabled");
      }
      directionChangeCount = 0;
    }
  }

  if (programmingMode) {
    unsigned long lastSpeedTs = protocolHandler->getLastSpeedChangeTs();
    if (lastSpeedTs > 0 && lastSpeedTs != lastSpeedChangeTime) {
      lastSpeedChangeTime = lastSpeedTs;
      int speed = protocolHandler->getTargetSpeed();
      if (cvAddress == -1) {
        cvAddress = speed;
        logger.printf(LogCategory::CV, "Prog: CV Address %d received\n",
                      cvAddress);
      } else {
        logger.printf(LogCategory::CV, "Prog: Writing CV %d = %d\n", cvAddress,
                      speed);
        cvManager->setCv(cvAddress, speed);
        cvAddress       = -1;
        programmingMode = false;
      }
    }
  }
}
