#include "CvManagerMock.h"
#include "MotorControl.h"
#include "Arduino.h"
#include <unity.h>
#include <deque>
#include <map>

void test_read_bemf_internal_median(void) {
  CvManagerMock cvManager;
  // BEMF pins: 2 and 3
  MotorControl motor(cvManager, 10, 11, 2, 3, 12);
  motor.setup();

  // Test Case 1: Median is the middle value
  // Samples will be |200-0| = 200, |100-0| = 100, |300-0| = 300
  // Median of {200, 100, 300} is 200
  analog_read_sequences[2] = {200, 100, 300};
  analog_read_sequences[3] = {0, 0, 0};

  int result = motor.readBEMF();
  TEST_ASSERT_EQUAL(200, result);

  // Test Case 2: Median with different pins
  // Samples: |0-500| = 500, |0-100| = 100, |0-400| = 400
  // Median of {500, 100, 400} is 400
  analog_read_sequences[2] = {0, 0, 0};
  analog_read_sequences[3] = {500, 100, 400};

  result = motor.readBEMF();
  TEST_ASSERT_EQUAL(400, result);

  // Test Case 3: All same
  analog_read_sequences[2] = {150, 150, 150};
  analog_read_sequences[3] = {0, 0, 0};

  result = motor.readBEMF();
  TEST_ASSERT_EQUAL(150, result);

  // Test Case 4: Two same
  // {100, 200, 100} -> Median 100
  analog_read_sequences[2] = {100, 200, 100};
  analog_read_sequences[3] = {0, 0, 0};

  result = motor.readBEMF();
  TEST_ASSERT_EQUAL(100, result);
}
