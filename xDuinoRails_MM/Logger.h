#ifndef LOGGER_H
#define LOGGER_H

#include "CvManager.h"
#include <Arduino.h>

class Logger {
public:
  Logger();
  void begin(CvManager *cvManager, unsigned long baudRate = 115200);
  void print(const char *message);
  void println(const char *message);
  void printf(const char *format, ...);

private:
  CvManager *cvManager;
  bool       isEnabled();
  bool       cachedEnabled;
  bool       isInitialized;
};

extern Logger logger;

#endif // LOGGER_H
