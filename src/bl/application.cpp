#include "bl/application.h"
#include "bl/config.h"
#include "cl/event_queue.h"
#include "cl/thermal_decoys_controller.h"
#include "dl/button.h"
#include "stm32f4xx_hal.h"

static TIM_HandleTypeDef htim2;
static EventQueue        event_queue;

static Button thermal_decoys_trg(THERMAL_DECOY_TRG_PORT, THERMAL_DECOY_TRG_PIN, 1, THERMAL_DECOY_TRG_IRQN);

static ThermalDecoysController thermal_decoys_ctrl(&thermal_decoys_trg, &event_queue);

extern "C" void Error_Handler(void);

/* ======================= Hardware init ======================= */

static void clocks_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

static void timer_init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = 1600 - 1; // 16 MHz / 1600 = 10 kHz
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 10 - 1; // 10 ticks at 10 kHz = 1 ms interrupt
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim2);
}

/* ======================= ISRs ======================= */
extern "C" {
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(thermal_decoys_trg.pin);
}

void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    if (thermal_decoys_ctrl.can_handle_trigger(pin)) {
        thermal_decoys_ctrl.handle_trigger();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (TIM2 == htim->Instance) {
        thermal_decoys_ctrl.handle_timer_tick();
    }
}
}

/* ======================= Public API ======================= */

void application_init() {
    clocks_init();
    timer_init();

    thermal_decoys_ctrl.init();
}

bool application_has_pending_events() {
    return !event_queue.is_empty();
}

void application_process_events() {
    // empty realisation for now
    return;
}
