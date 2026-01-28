#ifndef MOCK_PROTOCOL_HANDLER_H
#define MOCK_PROTOCOL_HANDLER_H

#include "ProtocolHandler.h"

class ProtocolHandlerMock : public ProtocolHandler {
public:
  ProtocolHandlerMock() : ProtocolHandler(0) {}

  unsigned long getLastChangeDirTs() override { return lastChangeDirTs; }
  unsigned long getLastSpeedChangeTs() override { return lastSpeedChangeTs; }
  int getTargetSpeed() override { return targetSpeed; }

  void setLastChangeDirTs(unsigned long ts) { lastChangeDirTs = ts; }
  void setLastSpeedChangeTs(unsigned long ts) { lastSpeedChangeTs = ts; }
  void setTargetSpeed(int speed) { targetSpeed = speed; }

private:
  unsigned long lastChangeDirTs = 0;
  unsigned long lastSpeedChangeTs = 0;
  int targetSpeed = 0;
};

#endif // MOCK_PROTOCOL_HANDLER_H
