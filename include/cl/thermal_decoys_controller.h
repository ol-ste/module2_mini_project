#pragma once

#include "cl/event.h"
#include "cl/event_queue.h"
#include "dl/button.h"

class ThermalDecoysController {
private:
    Button     *trigger;
    EventQueue *event_queue;

    volatile uint32_t activation_time_ms = 0;

    // volatile??
    volatile bool     current_state             = false;
    volatile uint32_t current_state_duration_ms = 0;
    volatile uint8_t  decoys_left               = 0;

    const uint16_t DECOY_ACTIVE_DURATION_MS = 100;
    const uint16_t DECOY_TOTAL_COUNT        = 3;

public:
    ThermalDecoysController(Button *trigger_value, EventQueue *event_queue_value);

    void init();

    bool can_handle_trigger(uint16_t trigger_pin);

    void handle_trigger();
    void handle_timer_tick();
    void handle_event(event_type_t type);
};
