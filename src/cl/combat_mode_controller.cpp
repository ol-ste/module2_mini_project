#include "cl/combat_mode_controller.h"
#include "cl/config.h"
#include "cl/event.h"
#include "cl/event_queue.h"

CombatModeController::CombatModeController(Button     *trigger_value,
                                           Led        *action_indicator_value,
                                           EventQueue *event_queue_value)
    : trigger(trigger_value), action_indicator(action_indicator_value), event_queue(event_queue_value) {}

void CombatModeController::init() {
    trigger->init();
    action_indicator->init();
}

bool CombatModeController::can_handle_trigger(uint16_t trigger_pin) {
    return trigger->pin == trigger_pin;
}

void CombatModeController::handle_trigger(uint16_t trigger_pin) {
    if (!can_handle_trigger(trigger_pin)) {
        return;
    }
    HAL_NVIC_DisableIRQ(trigger->irqn);
    action_recognition_time = TRIGGER_HELD_MS;
}

void CombatModeController::enable_trigger_irq() {
    short_action_detected   = false;
    action_recognition_time = 0;

    __HAL_GPIO_EXTI_CLEAR_IT(trigger->pin);
    HAL_NVIC_ClearPendingIRQ(trigger->irqn);
    HAL_NVIC_EnableIRQ(trigger->irqn);
}

void CombatModeController::handle_timer_tick() {
    if (action_recognition_time != 0) {
        --action_recognition_time;
        if (TRIGGER_HELD_MS - action_recognition_time >= TRIGGER_ACTIVATED_MS) {
            GPIO_PinState pinState = HAL_GPIO_ReadPin(trigger->port, trigger->pin);
            if (pinState == GPIO_PIN_SET) {
                if (action_recognition_time == 0) {
                    event_queue->post_event(SRC_COMBAT_MODE_CTRL, EVT_TRG_HELD);
                    enable_trigger_irq();
                } else {
                    short_action_detected = true;
                }
            } else {
                if (short_action_detected) {
                    event_queue->post_event(SRC_COMBAT_MODE_CTRL, EVT_TRG_ACTIVATED);
                    enable_trigger_irq();
                } else {
                    enable_trigger_irq();
                }
            }
        }
    }

    if (action_duration_ms != 0 && --action_duration_ms == 0u) {
        event_queue->post_event(SRC_COMBAT_MODE_CTRL, EVT_TIMEOUT);
    }
}

bool CombatModeController::can_handle_event(const event_t *e) {
    return e->source == SRC_COMBAT_MODE_CTRL;
}

void CombatModeController::handle_event(const event_t *e) {
    if (!can_handle_event(e)) {
        return;
    }
}

void CombatModeController::set_action_duration(uint32_t ms) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    action_duration_ms = ms;
    __set_PRIMASK(primask);
}

void CombatModeController::enter_idle() {
    current_state      = ST_CM_IDLE;
    is_firing          = false;
    action_duration_ms = 0;
    actions_left       = 0;
}

void CombatModeController::enter_target_locked() {
    current_state      = ST_CM_TARGET_LOCKED;
    is_firing          = false;
    action_duration_ms = 0;
    actions_left       = 0;

    action_indicator->on();
}

void CombatModeController::enter_firing() {
    current_state = ST_CM_FIRING;
    actions_left  = ACTIONS_TOTAL_COUNT * 2;

    continue_fire();
}

void CombatModeController::continue_fire() {
    actions_left--;
    is_firing = !is_firing;
    is_firing ? action_indicator->on() : action_indicator->off();
    set_action_duration(DEFAULT_ACTION_DURATION_MS);
}