#ifndef REALTIME_PROTOCOL_DECODER_H
#define REALTIME_PROTOCOL_DECODER_H

#include <Arduino.h>
#include <MaerklinMotorola.h>

class RealTimeProtocolDecoder {
public:
    RealTimeProtocolDecoder(int dccMmSignalPin);
    void setup();
    void loop();
    MaerklinMotorolaData *getData();
    MaerklinMotorola mm;

private:
    int dccMmSignalPin_priv;

};

#endif // REALTIME_PROTOCOL_DECODER_H
