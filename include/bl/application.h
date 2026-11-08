#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void application_init();

bool application_has_pending_events();

void application_process_next_event();

#ifdef __cplusplus
}
#endif