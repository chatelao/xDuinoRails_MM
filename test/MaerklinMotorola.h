#ifndef MAERKLIN_MOTOROLA_H
#define MAERKLIN_MOTOROLA_H

enum MM2DirectionState {
    MM2DirectionState_Unavailable,
    MM2DirectionState_Forward,
    MM2DirectionState_Backward
};

class MaerklinMotorolaData {
public:
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
    MaerklinMotorola(int pin) {}
    void PinChange() {}
    void Parse() {}
    MaerklinMotorolaData* GetData() { return nullptr; }
};

#endif // MAERKLIN_MOTOROLA_H
