#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EVT_NONE = 0,
    EVT_TRG_ACTIVATED,
    EVT_TRG_HELD,
    EVT_TIMEOUT,
} event_type_t;

typedef enum {
    SRC_UNKNOWN = 0,
    SRC_THERMAL_DECOYS_CTRL,
    SRC_COMBAT_MODE_CTRL,
    SRC_THREAD_DETECTION_CTRL
} even_source_t;

typedef struct {
    even_source_t source;
    event_type_t  type;
    uint32_t      ts;
} event_t;

#ifdef __cplusplus
}
#endif