#include "CvProgrammer.h"
#include "CvManager.h"
#include "ProtocolHandler.h"
#include <Arduino.h>
#include <unity.h>

void test_cv_programmer_instantiation(void);
void test_cv_programming_success(void);
void test_cv_programming_fail_lock_cv(void);

void test_cv_programmer_instantiation(void) {
  CvManager       cvManager;
  ProtocolHandler protocolHandler(0);
  CvProgrammer    cvProgrammer(&cvManager, &protocolHandler);
  TEST_ASSERT_NOT_NULL(&cvProgrammer);
}

void test_cv_programming_success(void) {
  CvManager       cvManager;
  ProtocolHandler protocolHandler(0);
  CvProgrammer    cvProgrammer(&cvManager, &protocolHandler);
  protocolHandler.setAddress(1);
  cvManager.setup();

  // Set CV 15 to 7 to enable programming
  cvManager.setCv(CV_PROGRAMMING_LOCK, 7);

  // Advance time so the first change is not at millis() = 0
  advance_millis(300);

  // 4 direction changes
  for (int i = 0; i < 4; i++) {
    protocolHandler.mm.SetData(1, 0, true, false, false,
                               MM2DirectionState_Unavailable, 0, false);
    protocolHandler.loop();
    cvProgrammer.loop();
    protocolHandler.mm.SetData(1, 0, false, false, false,
                               MM2DirectionState_Unavailable, 0, false);
    protocolHandler.loop();
    cvProgrammer.loop();
    advance_millis(300);
  }

  TEST_ASSERT_TRUE(cvProgrammer.isProgrammingModeActive());

  // Set CV 10 to 42
  protocolHandler.mm.SetData(1, 10, false, false, false,
                             MM2DirectionState_Unavailable, 0, false);
  protocolHandler.loop();
  cvProgrammer.loop();
  protocolHandler.mm.SetData(1, 42, false, false, false,
                             MM2DirectionState_Unavailable, 0, false);
  protocolHandler.loop();
  cvProgrammer.loop();

  TEST_ASSERT_EQUAL(42, cvManager.getCv(10));
  TEST_ASSERT_FALSE(cvProgrammer.isProgrammingModeActive());
}

void test_cv_programming_fail_lock_cv(void) {
  CvManager       cvManager;
  ProtocolHandler protocolHandler(0);
  CvProgrammer    cvProgrammer(&cvManager, &protocolHandler);
  protocolHandler.setAddress(1);
  cvManager.setup();

  // Set CV 15 to 0 to disable programming
  cvManager.setCv(CV_PROGRAMMING_LOCK, 0);

  // 4 direction changes
  for (int i = 0; i < 4; i++) {
    protocolHandler.mm.SetData(1, 0, true, false, false,
                               MM2DirectionState_Unavailable, 0, false);
    protocolHandler.loop();
    cvProgrammer.loop();
    protocolHandler.mm.SetData(1, 0, false, false, false,
                               MM2DirectionState_Unavailable, 0, false);
    protocolHandler.loop();
    cvProgrammer.loop();
    advance_millis(100);
  }

  TEST_ASSERT_FALSE(cvProgrammer.isProgrammingModeActive());
}
