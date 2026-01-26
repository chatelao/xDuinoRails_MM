#ifndef CV_PROGRAMMER_H
#define CV_PROGRAMMER_H

#include "CvManager.h"
#include "ProtocolHandler.h"

class CvProgrammer {
 public:
  CvProgrammer(CvManager* cvManager, ProtocolHandler* protocolHandler);
  void loop();

 private:
  CvManager* cvManager;
  ProtocolHandler* protocolHandler;
  bool programmingMode;
  unsigned long lastDirectionChangeTime;
  unsigned long lastSpeedChangeTime;
  int directionChangeCount;
  int cvAddress;
};

#endif  // CV_PROGRAMMER_H
