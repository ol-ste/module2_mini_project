#include "dl/relay.h"

Relay::Relay(GPIO_TypeDef *portValue, uint16_t pinValue) : port(portValue), pin(pinValue) {}

void Relay::init() {
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin   = pin;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &gpio);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void Relay::open() {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void Relay::close() {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}