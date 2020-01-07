#include "InterruptProxy.h"

InterruptProxy::InterruptProxy(uint8_t digitalPin, void (*callback)(), int mode) : interruptPin(digitalPinToInterrupt(digitalPin)), callback(callback), mode(mode)
{
}

void InterruptProxy::enable()
{
    attachInterrupt(interruptPin, callback, mode);
}

void InterruptProxy::disable()
{
    detachInterrupt(interruptPin);
}