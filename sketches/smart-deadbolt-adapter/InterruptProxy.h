#ifndef INTERRUPT_PROXY_H
#define INTERRUPT_PROXY_H

#include <Arduino.h>

class InterruptProxy
{
public:
    InterruptProxy(uint8_t digitalPin, void (*callback)(), int mode);
    InterruptProxy() {};
    void enable();
    void disable();

private:
    uint8_t interruptPin;
    void (*callback)();
    int mode;

};
#endif