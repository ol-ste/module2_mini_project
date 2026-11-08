#pragma once

#include "stm32f4xx_hal.h"

class Button {
private:
    uint32_t priority;

public:
    GPIO_TypeDef *port;
    uint16_t      pin;
    IRQn_Type     irqn;

    Button(GPIO_TypeDef *portValue, uint16_t pinValue, uint32_t priority, IRQn_Type irqnValue);

    void init();
};