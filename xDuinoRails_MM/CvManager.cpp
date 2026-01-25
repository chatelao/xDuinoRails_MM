#include "CvManager.h"
#include "pins.h"
#include <NmraDcc.h>

const byte VER_MAJOR = 1;
const byte VER_MINOR = 0;
const byte MANUF_ID = 151; // ESU

struct CVPair
{
  uint16_t  CV;
  uint8_t   Value;
};

CVPair FactoryDefaultCVs [] =
{
  {CV_MULTIFUNCTION_PRIMARY_ADDRESS, 3},
  {CV_START_VOLTAGE, 5},
  {CV_ACCELERATION_RATE, 3},
  {CV_DECELERATION_RATE, 3},
  {CV_MAXIMUM_SPEED, 255},
  {CV_MID_SPEED, 128},
  {CV_MANUFACTURER_ID, MANUF_ID},
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_MSB, CALC_MULTIFUNCTION_EXTENDED_ADDRESS_MSB(100)},
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_LSB, CALC_MULTIFUNCTION_EXTENDED_ADDRESS_LSB(100)},
  {CV_29_CONFIG, CV29_F0_LOCATION},
};

CvManager::CvManager() : factoryDefaultCVIndex(0) {
}

void CvManager::setup() {
    dcc.pin(digitalPinToInterrupt(DCC_PIN), DCC_PIN, 1);

    if ((dcc.getCV(CV_MANUFACTURER_ID) != MANUF_ID) || (dcc.getCV(CV_VERSION_ID) != VER_MAJOR)) {
        startFactoryReset();
    }

    dcc.init(MANUF_ID, VER_MAJOR, FLAGS_MY_ADDRESS_ONLY, 0);
}

void CvManager::loop() {
    dcc.process();

    if (factoryDefaultCVIndex && dcc.isSetCVReady())
    {
        factoryDefaultCVIndex--;
        dcc.setCV(FactoryDefaultCVs[factoryDefaultCVIndex].CV, FactoryDefaultCVs[factoryDefaultCVIndex].Value);
    }
}

uint8_t CvManager::getCv(int cv) {
    return dcc.getCV(cv);
}

void CvManager::startFactoryReset() {
    factoryDefaultCVIndex = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
}
