#include "dl/led.h"

Led::Led(GPIO_TypeDef *portValue, uint16_t pinValue) : port(portValue), pin(pinValue) {}

void Led::init() {
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin   = pin;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &gpio);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void Led::on() {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void Led::off() {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}