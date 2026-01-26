#ifndef I_LIGHTS_HARDWARE_H
#define I_LIGHTS_HARDWARE_H

class ILightsHardware {
public:
    virtual ~ILightsHardware() {}
    virtual void setPin(int pin, bool state) = 0;
};

#endif // I_LIGHTS_HARDWARE_H
