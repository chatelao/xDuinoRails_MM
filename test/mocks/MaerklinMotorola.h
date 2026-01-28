#ifndef MAERKLIN_MOTOROLA_MOCK_H
#define MAERKLIN_MOTOROLA_MOCK_H

#include <Arduino.h>

// Mock implementation of MaerklinMotorola.h for native testing
enum MM2DirectionState {
  MM2DirectionState_Forward,
  MM2DirectionState_Backward,
  MM2DirectionState_Unavailable
};

struct MaerklinMotorolaData {
  int               Address;
  int               Speed;
  bool              Function;
  bool              ChangeDir;
  bool              IsMM2;
  MM2DirectionState MM2Direction;
  int               MM2FunctionIndex;
  bool              IsMM2FunctionOn;
  bool              IsMagnet;
};

class MaerklinMotorola {
public:
  // Constructor
  MaerklinMotorola(int pin) {
    // Pin is ignored in mock
    (void)pin;
    // Initialize with a null pointer to indicate no data initially
    currentData = nullptr;
  }

  // Mocked Parse method, does nothing
  void Parse() {}

  // Mocked PinChange method, does nothing
  void PinChange() {}

  // Returns the data pointer. Tests can set this pointer.
  MaerklinMotorolaData* GetData() {
    return currentData;
  }

  // Test helper method to set the data that GetData() will return
  void SetData(MaerklinMotorolaData* data) {
    currentData = data;
  }

private:
  MaerklinMotorolaData* currentData;
};

#endif // MAERKLIN_MOTOROLA_MOCK_H
