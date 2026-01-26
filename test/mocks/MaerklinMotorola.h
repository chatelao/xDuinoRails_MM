#ifndef MAERKLIN_MOTOROLA_H
#define MAERKLIN_MOTOROLA_H

// Mock implementation of MaerklinMotorola.h for native testing
enum MM2DirectionState {
    MM2_DIR_FORWARD,
    MM2_DIR_BACKWARD
};

class MaerklinMotorola {
public:
    void setup(int pin) {}
    void loop() {}
    int getAddress() { return 0; }
    int getSpeed() { return 0; }
    MM2DirectionState getDirection() { return MM2_DIR_FORWARD; }
    bool getFunctionState(int f) { return false; }
};

#endif // MAERKLIN_MOTOROLA_H
