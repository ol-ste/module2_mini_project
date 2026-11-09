#include "cl/thermal_decoys_controller.h"
#include "cl/config.h"
#include "cl/event.h"
#include "cl/event_queue.h"

ThermalDecoysController::ThermalDecoysController(Button     *trigger_value,
                                                 Relay      *decoy_value,
                                                 EventQueue *event_queue_value)
    : trigger(trigger_value), decoy(decoy_value), event_queue(event_queue_value) {}

void ThermalDecoysController::init() {
    trigger->init();
    decoy->init();
}

bool ThermalDecoysController::can_handle_trigger(uint16_t trigger_pin) {
    return trigger->pin == trigger_pin;
}

void ThermalDecoysController::handle_trigger(uint16_t trigger_pin) {
    if (!can_handle_trigger(trigger_pin)) {
        return;
    }
    HAL_NVIC_DisableIRQ(trigger->irqn);
    activation_time_ms = TRIGGER_HELD_MS;
}

void ThermalDecoysController::handle_timer_tick() {
    if (activation_time_ms != 0 && --activation_time_ms == 0u) {
        if (GPIO_PIN_SET == HAL_GPIO_ReadPin(trigger->port, trigger->pin)) {
            event_queue->post_event(SRC_THERMAL_DECOYS_CTRL, EVT_TRG_HELD);
        }

        __HAL_GPIO_EXTI_CLEAR_IT(trigger->pin);
        HAL_NVIC_ClearPendingIRQ(trigger->irqn);
        HAL_NVIC_EnableIRQ(trigger->irqn);
    }

    if (action_duration_ms != 0 && --action_duration_ms == 0u) {
        event_queue->post_event(SRC_THERMAL_DECOYS_CTRL, EVT_TIMEOUT);
    }
}

bool ThermalDecoysController::can_handle_event(const event_t *e) {
    return e->source == SRC_THERMAL_DECOYS_CTRL;
}

void ThermalDecoysController::handle_event(const event_t *e) {
    if (!can_handle_event(e)) {
        return;
    }
    switch (current_state) {
    case ST_IDLE:
        if (EVT_TRG_HELD == e->type) {
            enter_firing();
        }
        break;
    case ST_FIRING:
        if (EVT_TIMEOUT == e->type) {
            if (0u == actions_left) {
                enter_idle();
            } else {
                continue_fire();
            }
        }
        break;
    }
}

void ThermalDecoysController::set_action_duration(uint32_t ms) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    action_duration_ms = ms;
    __set_PRIMASK(primask);
}

void ThermalDecoysController::enter_idle() {
    current_state      = ST_IDLE;
    is_firing          = false;
    action_duration_ms = 0;
    actions_left       = 0;
}

void ThermalDecoysController::enter_firing() {
    current_state = ST_FIRING;
    actions_left  = DECOY_TOTAL_COUNT * 2;
    continue_fire();
}

void ThermalDecoysController::continue_fire() {
    actions_left--;
    is_firing = !is_firing;
    is_firing ? decoy->open() : decoy->close();
    set_action_duration(is_firing ? DECOY_ACTIVE_DURATION_MS : DEFAULT_ACTION_DURATION_MS);
}