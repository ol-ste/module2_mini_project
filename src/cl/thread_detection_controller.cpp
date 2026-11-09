#include "cl/thread_detection_controller.h"
#include "cl/config.h"
#include "cl/event.h"
#include "cl/event_queue.h"

ThreadDetectionController::ThreadDetectionController(Button     *trigger_value,
                                                     Led        *low_thread_indicator_value,
                                                     Led        *high_thread_indicator_value,
                                                     EventQueue *event_queue_value)
    : trigger(trigger_value), low_thread_indicator(low_thread_indicator_value),
      high_thread_indicator(high_thread_indicator_value), event_queue(event_queue_value) {}

void ThreadDetectionController::init() {
    trigger->init();
    low_thread_indicator->init();
    high_thread_indicator->init();
}

bool ThreadDetectionController::can_handle_trigger(uint16_t trigger_pin) {
    return trigger->pin == trigger_pin;
}

void ThreadDetectionController::handle_trigger(uint16_t trigger_pin) {
    if (!can_handle_trigger(trigger_pin)) {
        return;
    }
    HAL_NVIC_DisableIRQ(trigger->irqn);
    action_recognition_time_ms = TRIGGER_HELD_MS;
}

void ThreadDetectionController::handle_timer_tick() {
    if (action_recognition_time_ms != 0) {
        --action_recognition_time_ms;
        if (TRIGGER_HELD_MS - action_recognition_time_ms >= TRIGGER_ACTIVATED_MS) {
            GPIO_PinState pinState = HAL_GPIO_ReadPin(trigger->port, trigger->pin);
            if (pinState == GPIO_PIN_SET) {
                if (action_recognition_time_ms == 0) {
                    event_queue->post_event(SRC_THREAD_DETECTION_CTRL, EVT_TRG_HELD);
                    enable_trigger_irq();
                } else {
                    short_action_detected = true;
                }
            } else {
                if (short_action_detected) {
                    event_queue->post_event(SRC_THREAD_DETECTION_CTRL, EVT_TRG_ACTIVATED);
                }
                enable_trigger_irq();
            }
        }
    }

    if (action_duration_ms != 0 && --action_duration_ms == 0u) {
        event_queue->post_event(SRC_THREAD_DETECTION_CTRL, EVT_TIMEOUT);
    }
}

bool ThreadDetectionController::can_handle_event(const event_t *e) {
    return e->source == SRC_COMBAT_MODE_CTRL || (e->source == SRC_THERMAL_DECOYS_CTRL && e->type == EVT_TRG_HELD);
}

void ThreadDetectionController::handle_event(const event_t *e) {
    if (!can_handle_event(e)) {
        return;
    }
    switch (current_state) {
    case ST_THD_IDLE:
        if (EVT_TRG_ACTIVATED == e->type) {
            enter_low_thread();
        }
        break;
    case ST_THD_LOW_THREAD_DETECTED:
        if (EVT_TRG_HELD == e->type) {
            enter_high_thread();
        }
        break;
    case ST_THD_HIGH_THREAD_DETECTED:
        bool thermalDecoyFired = e->source == SRC_THERMAL_DECOYS_CTRL && e->type == EVT_TRG_HELD;
        if (thermalDecoyFired) {
            enter_idle();
        }
        break;
    }
}

void ThreadDetectionController::enter_idle() {
    current_state = ST_THD_IDLE;
    reset_internal_states();

    high_thread_indicator->off();
}

void ThreadDetectionController::enter_low_thread() {
    current_state = ST_THD_LOW_THREAD_DETECTED;
    reset_internal_states();

    continue_low_thread_indicating();
}

void ThreadDetectionController::enter_high_thread() {
    current_state = ST_THD_HIGH_THREAD_DETECTED;
    reset_internal_states();

    low_thread_indicator->off();
    high_thread_indicator->on();
}

void ThreadDetectionController::continue_low_thread_indicating() {
    low_thread_indicating_state = !low_thread_indicating_state;
    low_thread_indicating_state ? low_thread_indicator->on() : low_thread_indicator->off();
    set_action_duration(DEFAULT_ACTION_DURATION_MS);
}

void ThreadDetectionController::enable_trigger_irq() {
    short_action_detected      = false;
    action_recognition_time_ms = 0;

    __HAL_GPIO_EXTI_CLEAR_IT(trigger->pin);
    HAL_NVIC_ClearPendingIRQ(trigger->irqn);
    HAL_NVIC_EnableIRQ(trigger->irqn);
}

void ThreadDetectionController::reset_internal_states() {
    low_thread_indicating_state = false;
    action_recognition_time_ms  = 0;
    action_duration_ms          = 0;
    short_action_detected       = false;
}

void ThreadDetectionController::set_action_duration(uint32_t ms) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    action_duration_ms = ms;
    __set_PRIMASK(primask);
}