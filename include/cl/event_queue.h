#pragma once

#include "event.h"

class EventQueue {
private:
    static constexpr uint8_t QUEUE_SIZE = 16;
    static constexpr uint8_t QUEUE_MASK = QUEUE_SIZE - 1;

    event_t           queue[QUEUE_SIZE];
    volatile uint8_t  head;
    volatile uint8_t  tail;
    volatile uint32_t dropped;

public:
    EventQueue();

    int      post_event(even_source_t source, event_type_t type);
    int      get_event(event_t *out);
    uint32_t get_events_dropped();
    int      is_empty();
};
