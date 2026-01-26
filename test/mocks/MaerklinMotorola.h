#ifndef MAERKLIN_MOTOROLA_H
#define MAERKLIN_MOTOROLA_H

enum MM2DirectionState {
    MM2DirectionState_Unavailable,
    MM2DirectionState_Forward,
    MM2DirectionState_Backward
};

class MaerklinMotorolaData {};
class MaerklinMotorola {
public:
    MaerklinMotorola(int pin) {}
    void PinChange() {}
    void Parse() {}
    MaerklinMotorolaData* GetData() { return nullptr; }
};

#endif // MAERKLIN_MOTOROLA_H
