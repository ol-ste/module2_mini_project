#pragma once

#include "stm32f4xx_hal.h"

#define THERMAL_DECOY_TRG_PORT GPIOA
#define THERMAL_DECOY_TRG_PIN  GPIO_PIN_0
#define THERMAL_DECOY_TRG_IRQN EXTI0_IRQn