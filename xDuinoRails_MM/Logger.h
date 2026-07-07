#ifndef LOGGER_H
#define LOGGER_H

#include "CvManager.h"
#include <Arduino.h>

enum class LogCategory { General, Protocol, PWM, CV };

class Logger {
public:
  Logger();
  void begin(CvManager *cvManager, unsigned long baudRate = 115200);
  void print(const char *message, LogCategory category = LogCategory::General);
  void println(const char *message,
               LogCategory category = LogCategory::General);
  void printf(LogCategory category, const char *format, ...);
  void printf(const char *format, ...); // Overload for General
  void toggleLogging();
  void toggleCategory(LogCategory category);
  bool isLoggingEnabled();
  bool isCategoryEnabled(LogCategory category);

private:
  CvManager *cvManager;
  bool       isEnabled();
  bool       cachedEnabled;
  bool       protocolEnabled;
  bool       pwmEnabled;
  bool       cvEnabled;
  bool       isInitialized;
};

extern Logger logger;

#endif // LOGGER_H
