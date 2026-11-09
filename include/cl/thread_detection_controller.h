#pragma once

#include "cl/event.h"
#include "cl/event_queue.h"
#include "dl/button.h"
#include "dl/led.h"

typedef enum {
    ST_THD_IDLE,
    ST_THD_LOW_THREAD_DETECTED,
    ST_THD_HIGH_THREAD_DETECTED,
} thread_detection_state_t;

class ThreadDetectionController {
private:
    Button     *trigger;
    Led        *low_thread_indicator;
    Led        *high_thread_indicator;
    EventQueue *event_queue;

    volatile uint32_t action_recognition_time_ms = 0;
    volatile bool     short_action_detected      = false;

    volatile thread_detection_state_t current_state = ST_THD_IDLE;

    volatile bool     low_thread_indicating_state = false;
    volatile uint32_t action_duration_ms          = 0;

    void enter_idle();
    void enter_low_thread();
    void enter_high_thread();

    void continue_low_thread_indicating();
    void enable_trigger_irq();
    void reset_internal_states();
    void set_action_duration(uint32_t ms);

public:
    ThreadDetectionController(Button     *trigger_value,
                              Led        *low_thread_indicator_value,
                              Led        *high_thread_indicator_value,
                              EventQueue *event_queue_value);

    void init();

    bool can_handle_trigger(uint16_t trigger_pin);
    void handle_trigger(uint16_t trigger_pin);

    bool can_handle_event(const event_t *e);
    void handle_event(const event_t *e);

    void handle_timer_tick();
};
