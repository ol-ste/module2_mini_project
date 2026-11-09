#include "bl/application.h"
#include "bl/config.h"
#include "cl/combat_mode_controller.h"
#include "cl/event_queue.h"
#include "cl/thermal_decoys_controller.h"
#include "dl/button.h"
#include "dl/led.h"
#include "dl/relay.h"
#include "stm32f4xx_hal.h"

static EventQueue event_queue;

static TIM_HandleTypeDef thermal_decoys_htim;
static Button            thermal_decoys_trg(THERMAL_DECOY_TRG_PORT, THERMAL_DECOY_TRG_PIN, 1, THERMAL_DECOY_TRG_IRQN);
static Relay             thermal_decoy(RELAY_PORT, RELAY_PIN);
static ThermalDecoysController thermal_decoys_ctrl(&thermal_decoys_trg, &thermal_decoy, &event_queue);

static TIM_HandleTypeDef    combat_mode_htim;
static Button               combat_mode_trg(COMBAT_MODE_TRG_PORT, COMBAT_MODE_TRG_PIN, 2, COMBAT_MODE_TRG_IRQN);
static Led                  combat_mode_action_indicator(GREEN_LED_PORT, GREEN_LED_PIN);
static CombatModeController combat_mode_ctrl(&combat_mode_trg, &combat_mode_action_indicator, &event_queue);

extern "C" void Error_Handler(void);

/* ======================= Hardware init ======================= */

static void clocks_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

static void timer_init(TIM_HandleTypeDef *htim, TIM_TypeDef *tim_instance, IRQn_Type tim_irqn, uint32_t priority) {
    htim->Instance               = TIM2;
    htim->Init.Prescaler         = 1600 - 1; // 16 MHz / 1600 = 10 kHz
    htim->Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim->Init.Period            = 10 - 1; // 10 ticks at 10 kHz = 1 ms interrupt
    htim->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(tim_irqn, priority, 0);
    HAL_NVIC_EnableIRQ(tim_irqn);

    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(htim);
}

static void timers_init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    timer_init(&thermal_decoys_htim, TIM2, TIM2_IRQn, 1);

    __HAL_RCC_TIM3_CLK_ENABLE();
    timer_init(&combat_mode_htim, TIM3, TIM3_IRQn, 2);
}

/* ======================= ISRs ======================= */
extern "C" {
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(thermal_decoys_trg.pin);
}

void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&thermal_decoys_htim);
}

void TIM3_IRQHandler(void) {
    HAL_TIM_IRQHandler(&combat_mode_htim);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    if (thermal_decoys_ctrl.can_handle_trigger(pin)) {
        thermal_decoys_ctrl.handle_trigger(pin);
    }
}

void HAL_GPI1_EXTI_Callback(uint16_t pin) {
    if (combat_mode_ctrl.can_handle_trigger(pin)) {
        combat_mode_ctrl.handle_trigger(pin);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (thermal_decoys_htim.Instance == htim->Instance) {
        thermal_decoys_ctrl.handle_timer_tick();
    }
    if (combat_mode_htim.Instance == htim->Instance) {
        combat_mode_ctrl.handle_timer_tick();
    }
}
}

/* ======================= Public API ======================= */

void application_init() {
    clocks_init();
    timers_init();

    thermal_decoys_ctrl.init();
    combat_mode_ctrl.init();
}

bool application_has_pending_events() {
    return !event_queue.is_empty();
}

void application_process_next_event() {
    event_t evt;
    if (event_queue.get_event(&evt) == 0) {
        if (thermal_decoys_ctrl.can_handle_event(&evt)) {
            thermal_decoys_ctrl.handle_event(&evt);
        }
        if (combat_mode_ctrl.can_handle_event(&evt)) {
            combat_mode_ctrl.handle_event(&evt);
        }
    }
    return;
}
