#include "bl/application.h"
#include "stm32f4xx_hal.h"

extern "C" void Error_Handler(void);

/* ======================= Main loop = dispatcher ======================= */

int main(void) {
    HAL_Init();
    application_init();

    while (1) {
        while (application_has_pending_events()) {
            application_process_events();
        }

        __disable_irq();
        if (!application_has_pending_events()) {
            __WFI();
        }
        __enable_irq();
    }
}

extern "C" {
void SysTick_Handler(void) {
    HAL_IncTick();
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
}