#include "cl/thermal_decoys_controller.h"
#include "cl/config.h"
#include "cl/event.h"
#include "cl/event_queue.h"

ThermalDecoysController::ThermalDecoysController(Button *trigger_value, EventQueue *event_queue_value)
    : trigger(trigger_value), event_queue(event_queue_value) {}

void ThermalDecoysController::init() {
    trigger->init();
}

bool ThermalDecoysController::can_handle_trigger(uint16_t trigger_pin) {
    return trigger->pin == trigger_pin;
}

void ThermalDecoysController::handle_trigger() {
    HAL_NVIC_DisableIRQ(trigger->irqn);
    activation_time_ms = TRIGGER_HELD_MS;
}

void ThermalDecoysController::handle_timer_tick() {
    if (activation_time_ms != 0 && --activation_time_ms == 0u) {
        if (GPIO_PIN_SET == HAL_GPIO_ReadPin(trigger->port, trigger->pin)) {
            event_queue->post_event(SRC_THERMAL_DECOY, EVT_TRG_HELD);
        }

        __HAL_GPIO_EXTI_CLEAR_IT(trigger->pin);
        HAL_NVIC_ClearPendingIRQ(trigger->irqn);
        HAL_NVIC_EnableIRQ(trigger->irqn);
    }
}

void ThermalDecoysController::handle_event(event_type_t type) {
    // TODO: add state machine here
}