#ifndef CV_MANAGER_H
#define CV_MANAGER_H

#include <Arduino.h>
#include <NmraDcc.h>

#define CV_DEC_MAN_ID 13 // DIY/Public Domain

class CvManager {
public:
    CvManager();
    void setup();
    void loop();

    uint8_t getCv(int cv);
    void setCv(int cv, uint8_t value);

private:
    NmraDcc dcc;
};

#endif // CV_MANAGER_H
