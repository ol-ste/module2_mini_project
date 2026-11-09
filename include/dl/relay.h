#pragma once

#include "stm32f4xx_hal.h"

class Relay {
private:
    GPIO_TypeDef *port;
    uint16_t      pin;

public:
    Relay(GPIO_TypeDef *portValue, uint16_t pinValue);

    void init();

    void open();
    void close();
};