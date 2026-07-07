#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "CvManager.h"
#include <Arduino.h>
#include <MaerklinMotorola.h>

class MotorControl {
public:
  MotorControl(CvManager &cvManager, int pinA, int pinB, int bemfA, int bemfB,
               int shutPin);
  void              setup();
  void              setSpeed(int step, MM2DirectionState targetDir);
  void              stop();
  bool              isKickstarting();
  MM2DirectionState getCurrentDirection();

private:
  void update(int targetPwm, MM2DirectionState targetDir);
  void writeMotorHardware(int pwm, MM2DirectionState dir);
  int  readBEMF();

  CvManager        &cvManager;
  int               pinA_priv;
  int               pinB_priv;
  int               bemfA_priv;
  int               bemfB_priv;
  int               shutPin_priv;
  int               targetPwm;
  MM2DirectionState currDirection;
  MM2DirectionState targetDirection;
  bool              isKickstarting_priv;
  unsigned long     kickstartBegin;
  unsigned long     lastBemfMeasure;
  int               lastSpeed;
  int               previousPwm;
  long              bemfErrorSum;
  int               lastAdjustment;

  int               lastWrittenPwm;
  MM2DirectionState lastWrittenDir;

  // Telemetry
  unsigned long lastTelemetryTime;
  long          bemfSum;
  int           bemfCount;
  int           lastMeasuredBemf;
  long          pwmSum;
  int           pwmCount;
  int           lastSentPwm;

  // Motor parameters - will be set based on motorType
  int PWM_FREQ;
  int KICK_PWM;
  int KICK_MAX_TIME;
  int BEMF_THRESHOLD;
  int BEMF_SAMPLE_INT;
};

#endif // MOTOR_CONTROL_H
