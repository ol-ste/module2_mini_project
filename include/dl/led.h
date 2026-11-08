#pragma once

#include "stm32f4xx_hal.h"

class Led {
private:
    GPIO_TypeDef *port;
    uint16_t      pin;

public:
    Led(GPIO_TypeDef *portValue, uint16_t pinValue);

    void init();

    void on();
    void off();
};