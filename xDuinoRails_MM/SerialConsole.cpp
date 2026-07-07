#include "SerialConsole.h"
#include "Logger.h"
#include <stdio.h>
#include <string.h>

SerialConsole::SerialConsole(CvManager *cvManager, ProtocolHandler *protocol)
    : cvManager(cvManager), protocol(protocol), bufferIndex(0) {
  memset(inputBuffer, 0, sizeof(inputBuffer));
}

void SerialConsole::loop() {
  while (DEBUG_SERIAL.available() > 0) {
    char c = DEBUG_SERIAL.read();
    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        inputBuffer[bufferIndex] = '\0';
        parseCommand(inputBuffer);
        bufferIndex = 0;
      }
    } else {
      if (bufferIndex < (int)sizeof(inputBuffer) - 1) {
        inputBuffer[bufferIndex++] = c;
      }
    }
  }
}

void SerialConsole::parseCommand(char *line) {
  char cmd[16];
  int  val1, val2;

  if (sscanf(line, "cv %d %d", &val1, &val2) == 2) {
    cvManager->setCv(val1, (uint8_t)val2);
    logger.printf("Serial: CV %d set to %d\n", val1, val2);
  } else if (strcmp(line, "cv") == 0) {
    cvManager->printAllCvs();
  } else if (sscanf(line, "s %d", &val1) == 1) {
    protocol->setTargetSpeed(val1);
    logger.printf("Serial: Speed set to %d\n", val1);
  } else if (line[0] == 'd' && line[1] == ' ') {
    if (line[2] == 'f') {
      protocol->setTargetDirection(MM2DirectionState_Forward);
      logger.println("Serial: Direction Forward");
    } else if (line[2] == 'b') {
      protocol->setTargetDirection(MM2DirectionState_Backward);
      logger.println("Serial: Direction Backward");
    }
  } else if (sscanf(line, "f %d", &val1) == 1) {
    // We assume Function 1 as per request "f 0 : disable function 1", "f 1 : enable function 1"
    protocol->setFunctionState(1, val1 > 0);
    logger.printf("Serial: Function 1 set to %d\n", val1 > 0);
  } else if (line[0] == 'L' || line[0] == 'l') {
    if (line[1] == ' ' && line[2] != '\0') {
      char sub = line[2];
      if (sub == 'p') {
        logger.toggleCategory(LogCategory::Protocol);
        DEBUG_SERIAL.print("Serial: Protocol Logging ");
        DEBUG_SERIAL.println(logger.isCategoryEnabled(LogCategory::Protocol) ? "ON" : "OFF");
      } else if (sub == 'w') {
        logger.toggleCategory(LogCategory::PWM);
        DEBUG_SERIAL.print("Serial: PWM Logging ");
        DEBUG_SERIAL.println(logger.isCategoryEnabled(LogCategory::PWM) ? "ON" : "OFF");
      } else if (sub == 'c') {
        logger.toggleCategory(LogCategory::CV);
        DEBUG_SERIAL.print("Serial: CV Logging ");
        DEBUG_SERIAL.println(logger.isCategoryEnabled(LogCategory::CV) ? "ON" : "OFF");
      } else if (sub == 'b') {
        logger.toggleCategory(LogCategory::BEMF);
        DEBUG_SERIAL.print("Serial: BEMF Logging ");
        DEBUG_SERIAL.println(logger.isCategoryEnabled(LogCategory::BEMF) ? "ON" : "OFF");
      }
    } else {
      logger.toggleLogging();
      DEBUG_SERIAL.print("Serial: Logging ");
      DEBUG_SERIAL.println(logger.isLoggingEnabled() ? "ON" : "OFF");
    }
  } else if (line[0] == 'h' || line[0] == '?') {
    printHelp();
  }
}

void SerialConsole::printHelp() {
  DEBUG_SERIAL.println("--- xDuinoRails CLI Help ---");
  DEBUG_SERIAL.println("cv <num> <val> : Set CV value");
  DEBUG_SERIAL.println("cv             : Print all CV values");
  DEBUG_SERIAL.println("s <speed>      : Set speed (0-14)");
  DEBUG_SERIAL.println("d f/b          : Set direction (f: forward, b: backward)");
  DEBUG_SERIAL.println("f <0/1>        : Set Function 1 (0: off, 1: on)");
  DEBUG_SERIAL.println("L [p/w/c]      : Toggle logging (p: protocol, w: pwm, c: cv)");
  DEBUG_SERIAL.println("h/?            : Show this help");
  DEBUG_SERIAL.println("----------------------------");
}
