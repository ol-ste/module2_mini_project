#include "cl/event_queue.h"
#include "cl/event.h"
#include "stm32f4xx_hal.h"

EventQueue::EventQueue() {
    head    = 0;
    tail    = 0;
    dropped = 0;
}

int EventQueue::post_event(even_source_t source, event_type_t type) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint8_t next = (uint8_t)((head + 1u) & QUEUE_MASK);
    if (next == tail) {
        dropped++;
        __set_PRIMASK(primask);
        return -1;
    }

    queue[head].source = source;
    queue[head].type   = type;
    queue[head].ts     = HAL_GetTick();
    head               = next;

    __set_PRIMASK(primask);
    return 0;
}

int EventQueue::get_event(event_t *out) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (head == tail) {
        __set_PRIMASK(primask);
        return -1;
    }

    *out = queue[tail];
    tail = (uint8_t)((tail + 1u) & QUEUE_MASK);

    __set_PRIMASK(primask);
    return 0;
}

int EventQueue::is_empty() {
    return head == tail;
}

uint32_t EventQueue::get_events_dropped() {
    return dropped;
}