#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "debug.h"


void main_thread_init(void)
{

}


void main_thread_func(void const * argument)
{
	int i = 0;

    for (;;) {
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

        debug_process();

        osDelay(500);
    }
}
