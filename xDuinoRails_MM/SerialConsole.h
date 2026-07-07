#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#include "CvManager.h"
#include "ProtocolHandler.h"
#include <Arduino.h>

class SerialConsole {
public:
  SerialConsole(CvManager *cvManager, ProtocolHandler *protocol);
  void loop();

private:
  CvManager *      cvManager;
  ProtocolHandler *protocol;
  char             inputBuffer[64];
  int              bufferIndex;

  void parseCommand(char *line);
  void printHelp();
};

#endif // SERIAL_CONSOLE_H
