#include "RealTimeProtocolDecoder.h"

#ifndef PIO_UNIT_TESTING
extern RealTimeProtocolDecoder realTimeProtocolDecoder;

void isr_protocol_decoder() { realTimeProtocolDecoder.mm.PinChange(); }
#endif

RealTimeProtocolDecoder::RealTimeProtocolDecoder(int dccMmSignalPin)
    : mm(dccMmSignalPin) {
  dccMmSignalPin_priv = dccMmSignalPin;
}

void RealTimeProtocolDecoder::setup() {
#ifndef PIO_UNIT_TESTING
  attachInterrupt(digitalPinToInterrupt(dccMmSignalPin_priv), isr_protocol_decoder,
                  CHANGE);
#endif
}

void RealTimeProtocolDecoder::loop() {
  mm.Parse();
}

MaerklinMotorolaData *RealTimeProtocolDecoder::getData() {
    return mm.GetData();
}
