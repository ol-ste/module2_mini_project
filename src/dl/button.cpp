#include "dl/button.h"

Button::Button(GPIO_TypeDef *portValue, uint16_t pinValue, uint32_t priorityValue, IRQn_Type irqnValue)
    : port(portValue), pin(pinValue), priority(priorityValue), irqn(irqnValue) {}

void Button::init() {
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin  = pin;
    gpio.Mode = GPIO_MODE_IT_RISING;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(port, &gpio);

    HAL_NVIC_SetPriority(irqn, priority, 0);
    HAL_NVIC_EnableIRQ(irqn);
}