#include "uart_thread.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

uint8_t debug_buffer[1024];
osMutexId debug_mtxHandle;

void debug_rx_handler(uint8_t *data, uint32_t len);


void debug_init(void)
{
    osMutexDef(debug_mtx);
    debug_mtxHandle = osMutexCreate(osMutex(debug_mtx));

    register_data_handler(&huart4, debug_rx_handler);
}


void debug_rx_handler(uint8_t *data, uint32_t len)
{

}

/* in main_thread */
void debug_process(void)
{

}
