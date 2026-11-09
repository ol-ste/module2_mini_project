#pragma once

#include "cl/event_queue.h"
#include "dl/button.h"
#include "dl/led.h"

typedef enum {
    ST_CM_IDLE,
    ST_CM_TARGET_LOCKED,
    ST_CM_FIRING,
} combat_mode_state_t;

class CombatModeController {
private:
    Button     *trigger;
    Led        *action_indicator;
    EventQueue *event_queue;

    volatile uint32_t action_recognition_time_ms = 0;
    volatile bool     short_action_detected      = false;

    volatile combat_mode_state_t current_state = ST_CM_IDLE;

    volatile bool     is_firing          = false;
    volatile uint32_t action_duration_ms = 0;
    volatile uint8_t  actions_left       = 0;

    const uint16_t ACTIONS_TOTAL_COUNT = 3;

    void enter_idle();
    void enter_target_locked();
    void enter_firing();

    void continue_fire();
    void enable_trigger_irq();
    void reset_internal_states();
    void set_action_duration(uint32_t ms);

public:
    CombatModeController(Button *trigger_value, Led *action_indicator_value, EventQueue *event_queue_value);

    void init();

    bool can_handle_trigger(uint16_t trigger_pin);
    void handle_trigger(uint16_t trigger_pin);

    bool can_handle_event(const event_t *e);
    void handle_event(const event_t *e);

    void handle_timer_tick();
};
