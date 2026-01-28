#ifndef MAERKLIN_MOTOROLA_MOCK_H
#define MAERKLIN_MOTOROLA_MOCK_H

// This is a mock implementation of the MaerklinMotorola library for testing purposes.

// Enum for direction states as used in the original library
enum MM2DirectionState {
  MM2DirectionState_Forward,
  MM2DirectionState_Backward,
  MM2DirectionState_Unavailable
};

// Mock data struct that the GetData() method will return
struct MaerklinMotorolaData {
  bool IsMagnet;
  int Address;
  bool IsMM2;
  MM2DirectionState MM2Direction;
  int MM2FunctionIndex;
  bool IsMM2FunctionOn;
  bool ChangeDir;
  int Speed;
  bool Function;
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
