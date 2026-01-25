#ifndef CV_MANAGER_H
#define CV_MANAGER_H

#include <Arduino.h>
#include <NmraDcc.h>

class CvManager {
public:
    CvManager();
    void setup();
    void loop();
    uint8_t getCv(int cv);
    void startFactoryReset();

private:
    NmraDcc dcc;
    uint8_t factoryDefaultCVIndex;
};

#endif // CV_MANAGER_H
