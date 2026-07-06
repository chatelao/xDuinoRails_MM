#include "MotorControl.h"
#include "Logger.h"

const int PWM_RANGE = 1023;

MotorControl::MotorControl(CvManager &cvManager, int pinA, int pinB, int bemfA,
                           int bemfB)
    : cvManager(cvManager) {
  pinA_priv           = pinA;
  pinB_priv           = pinB;
  bemfA_priv          = bemfA;
  bemfB_priv          = bemfB;
  targetPwm           = 0;
  currDirection       = MM2DirectionState_Forward;
  targetDirection     = MM2DirectionState_Forward;
  isKickstarting_priv = false;
  kickstartBegin      = 0;
  lastBemfMeasure     = 0;
  lastSpeed           = 0;
  previousPwm         = 0;
  bemfErrorIntegral   = 0;
  lastLoadCompUpdate  = 0;
}

void MotorControl::setup() {
  pinMode(pinA_priv, OUTPUT);
  pinMode(pinB_priv, OUTPUT);
  pinMode(bemfA_priv, INPUT);
  pinMode(bemfB_priv, INPUT);

  int         motorType = cvManager.getCv(CV_MOTOR_TYPE);
  const char *typeName  = "Standard DC";

  switch (motorType) {
  case 1: // Faulhaber
    PWM_FREQ        = 400;
    KICK_PWM        = 1023;
    KICK_MAX_TIME   = 150;
    BEMF_THRESHOLD  = 120;
    BEMF_SAMPLE_INT = 15;
    typeName        = "Faulhaber";
    break;
  case 2: // Maxon
    PWM_FREQ        = 20000;
    KICK_PWM        = 600;
    KICK_MAX_TIME   = 80;
    BEMF_THRESHOLD  = 80;
    BEMF_SAMPLE_INT = 10;
    typeName        = "Maxon";
    break;
  default: // Standard DC
    PWM_FREQ        = 20000;
    KICK_PWM        = 800;
    KICK_MAX_TIME   = 100;
    BEMF_THRESHOLD  = 100;
    BEMF_SAMPLE_INT = 12;
    typeName        = "Standard DC";
    break;
  }

  logger.printf("Motor: Type=%s, PWM Freq=%d Hz\n", typeName, PWM_FREQ);

#ifdef ARDUINO_ARCH_RP2040
  analogWriteFreq(PWM_FREQ);
  analogWriteRange(PWM_RANGE);
#elif defined(ARDUINO_ARCH_ESP32)
  analogWriteFrequency(pinA_priv, PWM_FREQ);
  analogWriteFrequency(pinB_priv, PWM_FREQ);
  analogWriteResolution(pinA_priv, 10); // 10 bits = 1023
  analogWriteResolution(pinB_priv, 10); // 10 bits = 1023
#endif

  writeMotorHardware(0, MM2DirectionState_Forward);
}

const int PWM_MAX = 1023;

void MotorControl::setSpeed(int step, MM2DirectionState dir) {
  if (step == 0) {
    update(0, dir);
    return;
  }

  // Map CVs (0-255) to PWM (0-1023)
  int vStart = map(cvManager.getCv(CV_START_VOLTAGE), 0, 255, 0, PWM_MAX);
  int vHigh  = map(cvManager.getCv(CV_MAXIMUM_SPEED), 0, 255, 0, PWM_MAX);
  int vMid   = map(cvManager.getCv(CV_MEDIUM_SPEED), 0, 255, 0, PWM_MAX);

  if (vHigh == 0) vHigh = PWM_MAX;
  if (vStart == 0) vStart = 1;
  if (vStart > vHigh) vStart = vHigh;
  if (vMid == 0) {
    vMid = (vStart + vHigh) / 2;
  } else {
    if (vMid < vStart) vMid = vStart;
    if (vMid > vHigh) vMid = vHigh;
  }

  int pwm;
  if (step <= 7) {
    pwm = map(step, 1, 7, vStart, vMid);
  } else {
    pwm = map(step, 7, 14, vMid, vHigh);
  }

  update(pwm, dir);
}

void MotorControl::update(int pwm, MM2DirectionState dir) {
  unsigned long now = millis();
  if (now == 0) now = 1;

  MM2DirectionState effectiveDir = dir;
  if (effectiveDir == MM2DirectionState_Unavailable) {
    effectiveDir = currDirection;
  }

  // Handle Stop
  if (pwm == 0) {
    if (targetPwm != 0 || isKickstarting_priv) {
      writeMotorHardware(0, effectiveDir);
    }
    currDirection = effectiveDir;
    targetDirection = effectiveDir;
    targetPwm = 0;
    previousPwm = 0;
    isKickstarting_priv = false;
    bemfErrorIntegral = 0;
    return;
  }

  bool directionChanged = (effectiveDir != currDirection);
  bool targetPwmChanged = (pwm != targetPwm);

  targetPwm = pwm;
  targetDirection = effectiveDir;

  // Handle Direction Change
  if (directionChanged && !isKickstarting_priv) {
    writeMotorHardware(0, currDirection);
    currDirection = targetDirection;
    previousPwm = 0;
    bemfErrorIntegral = 0;
    lastLoadCompUpdate = 0;
    return;
  }

  // Kickstart trigger
  if (previousPwm == 0 && !isKickstarting_priv && KICK_MAX_TIME > 0) {
    isKickstarting_priv = true;
    kickstartBegin = now;
    lastBemfMeasure = now;
    logger.println("Motor: Kickstart started");
    writeMotorHardware(KICK_PWM, currDirection);
  }

  bool kickstartEndedNow = false;
  if (isKickstarting_priv) {
    if (now - kickstartBegin >= (unsigned long)KICK_MAX_TIME) {
      isKickstarting_priv = false;
      kickstartEndedNow = true;
      logger.println("Motor: Kickstart ended (timeout)");
    } else {
      bool bemfEnabled = (cvManager.getCv(CV_BEMF_CONFIG) & 0x01);
      if (bemfEnabled && (now - lastBemfMeasure >= (unsigned long)BEMF_SAMPLE_INT)) {
        int currentBEMF = readBEMF();
        lastBemfMeasure = now;
        if (currentBEMF > BEMF_THRESHOLD) {
          isKickstarting_priv = false;
          kickstartEndedNow = true;
          logger.println("Motor: Kickstart ended (BEMF)");
        }
      }
    }
  }

  if (!isKickstarting_priv) {
    bool loadCompEnabled = (cvManager.getCv(CV_BEMF_CONFIG) & 0x01);
    if (targetPwmChanged) bemfErrorIntegral = 0;

    if (loadCompEnabled) {
      if (now - lastLoadCompUpdate >= (unsigned long)BEMF_SAMPLE_INT || targetPwmChanged || kickstartEndedNow) {
        int measuredBEMF = readBEMF();
        int targetBEMF = targetPwm * 4;
        int error = targetBEMF - measuredBEMF;
        float k = cvManager.getCv(CV_BEMF_K) / 32.0f;
        float i = cvManager.getCv(CV_BEMF_I) / 128.0f;
        bemfErrorIntegral += error;
        if (bemfErrorIntegral > 2000) bemfErrorIntegral = 2000;
        if (bemfErrorIntegral < -2000) bemfErrorIntegral = -2000;
        int adj = (int)(error * k + bemfErrorIntegral * i);
        int finalPwm = targetPwm + adj;
        if (finalPwm > 1023) finalPwm = 1023;
        if (finalPwm < 0) finalPwm = 0;
        writeMotorHardware(finalPwm, currDirection);
        lastLoadCompUpdate = now;
      }
    } else if (targetPwmChanged || kickstartEndedNow) {
      writeMotorHardware(targetPwm, currDirection);
    }
  }

  previousPwm = targetPwm;
}

void MotorControl::stop() { update(0, currDirection); }
bool MotorControl::isKickstarting() { return isKickstarting_priv; }
MM2DirectionState MotorControl::getCurrentDirection() { return currDirection; }

void MotorControl::writeMotorHardware(int pwm, MM2DirectionState dir) {
  if (pwm > PWM_RANGE) pwm = PWM_RANGE;
  if (pwm < 0) pwm = 0;
  if (dir == MM2DirectionState_Forward) {
    digitalWrite(pinB_priv, LOW);
    analogWrite(pinA_priv, pwm);
  } else {
    digitalWrite(pinA_priv, LOW);
    analogWrite(pinB_priv, pwm);
  }
}

int MotorControl::readBEMF() {
  digitalWrite(pinA_priv, LOW);
  digitalWrite(pinB_priv, LOW);
  delayMicroseconds(500);
  int valA = analogRead(bemfA_priv);
  int valB = analogRead(bemfB_priv);
  return (valA > valB) ? (valA - valB) : (valB - valA);
}
