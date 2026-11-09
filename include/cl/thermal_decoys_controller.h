#pragma once

#include "cl/event.h"
#include "cl/event_queue.h"
#include "dl/button.h"
#include "dl/relay.h"

typedef enum {
    ST_IDLE,
    ST_FIRING,
} thermal_decoys_state_t;

class ThermalDecoysController {
private:
    Button     *trigger;
    Relay      *decoy;
    EventQueue *event_queue;

    volatile uint32_t activation_time_ms = 0;

    // volatile??
    volatile thermal_decoys_state_t current_state = ST_IDLE;

    volatile bool     is_firing          = false;
    volatile uint32_t action_duration_ms = 0;
    volatile uint8_t  actions_left       = 0;

    const uint16_t DECOY_ACTIVE_DURATION_MS = 100;
    const uint16_t DECOY_TOTAL_COUNT        = 3;

    void enter_idle();
    void enter_firing();

    void set_action_duration(uint32_t ms);
    void continue_fire();

public:
    ThermalDecoysController(Button *trigger_value, Relay *decoy_value, EventQueue *event_queue_value);

    void init();

    bool can_handle_trigger(uint16_t trigger_pin);
    void handle_trigger(uint16_t trigger_pin);

    bool can_handle_event(const event_t *e);
    void handle_event(const event_t *e);

    void handle_timer_tick();
};
