#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "uart_thread.h"

extern uint8_t debug_buffer[128];
extern osMutexId debug_mtxHandle;

#define print_log(...)                                  \
{                                                       \
    uint32_t len;                                       \
    uint8_t *temp_buffer;                               \
    osMutexWait(debug_mtxHandle, osWaitForever);        \
    memset(debug_buffer, 0, 128);                       \
    len = snprintf((char *)debug_buffer, 128,  __VA_ARGS__);  \
    temp_buffer = (uint8_t*)pvPortMalloc(len + 1);      \
    memcpy(temp_buffer, debug_buffer, len);             \
    osMutexRelease(debug_mtxHandle);                    \
    temp_buffer[len+1] = 0;                             \
    uart4_send(temp_buffer, len);                       \
    vPortFree(temp_buffer);                            \
}

void debug_init(void);
void debug_process(void);

#endif
