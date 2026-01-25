#include "CvManager.h"

// CV Defaults
#define CV_DEC_MAN_ID_DEFAULT 13 // DIY/Public Domain

CvManager::CvManager() {
    // Constructor
}

void CvManager::setup() {
    // Set default CV values from documentation
    dcc.setCV(CV_DEC_MAN_ID, CV_DEC_MAN_ID_DEFAULT);
    dcc.setCV(1, 3);      // Base Address
    dcc.setCV(2, 3);      // Start Voltage
    dcc.setCV(3, 5);      // Acceleration
    dcc.setCV(4, 5);      // Braking Time
    dcc.setCV(5, 255);    // Maximum Speed
    dcc.setCV(7, 10);     // Version
    dcc.setCV(8, CV_DEC_MAN_ID_DEFAULT); // Manufacturer ID
    dcc.setCV(17, 192);   // Long Addr. (High)
    dcc.setCV(18, 100);   // Long Addr. (Low)
    dcc.setCV(29, 6);     // Configuration
    dcc.setCV(33, 1);     // Front Light (F0f)
    dcc.setCV(34, 2);     // Rear Light (F0r)
}

void CvManager::loop() {
    // CV processing is often handled by interrupts within the DCC library
}

uint8_t CvManager::getCv(int cv) {
    return dcc.getCV(cv);
}

void CvManager::setCv(int cv, uint8_t value) {
    dcc.setCV(cv, value);
}
