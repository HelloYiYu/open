#include "uart_thread.h"
#include "debug.h"

#define UART_BUFFER_SIZE 128

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

struct SEND_DATA {
    struct list_head node;

    uint32_t size;
    uint8_t *data;
};

struct UART_BUFFER {
    UART_HandleTypeDef *p_uart;

    /* hw rx */
    uint32_t rx_hw_buffer_size;
    uint32_t rx_hw_buffer_idx;
    uint32_t rx_hw_buffer_len;
    uint8_t *rx_hw_buffer[2];

    /* hw tx */
    uint32_t tx_hw_buffer_size;
    uint8_t *tx_hw_buffer;
    uint32_t is_tx_busy;

    /* sw rx */
    uint32_t rx_sw_buffer_size;
    uint8_t *rx_sw_buffer;
    uint32_t rx_len;
    uint32_t rx_type;  // 0-to sw rx buffer, 1-to data_handler_func

    /* sw tx */
    osMutexId tx_list_mtx;
    struct list_head tx_list;

    /* send and read */
    uint32_t read_status;
    osSemaphoreId sem;
    uint8_t *read_data;
    uint32_t *read_len;

    data_handler data_handler_func;
};
static struct UART_BUFFER __uart[4];


extern osThreadId uart_threadHandle;
extern osMutexId uart1_mtxHandle;
extern osMutexId uart2_mtxHandle;
extern osMutexId uart3_mtxHandle;
extern osMutexId uart4_mtxHandle;
extern osSemaphoreId uart1_send_semHandle;
extern osSemaphoreId uart2_send_semHandle;
extern osSemaphoreId uart3_send_semHandle;
extern osSemaphoreId uart4_send_semHandle;
extern void debug_rx_handler(uint8_t *data, uint32_t len);


void uart_thread_init(void)
{
    for (int i = 0; i < 4; i++) {
        memset(&__uart[i], 0, sizeof(struct UART_BUFFER));
    }

    __uart[0].rx_hw_buffer_size = 256;
    __uart[1].rx_hw_buffer_size = 256;
    __uart[2].rx_hw_buffer_size = 1;
    __uart[3].rx_hw_buffer_size = 1;

    __uart[0].tx_hw_buffer_size = 256;
    __uart[1].tx_hw_buffer_size = 256;
    __uart[2].tx_hw_buffer_size = 256;
    __uart[3].tx_hw_buffer_size = 256;

    __uart[0].rx_sw_buffer_size = 256;
    __uart[1].rx_sw_buffer_size = 256;
    __uart[2].rx_sw_buffer_size = 256;
    __uart[3].rx_sw_buffer_size = 256;

    __uart[0].p_uart = &huart1;
    __uart[1].p_uart = &huart2;
    __uart[2].p_uart = &huart3;
    __uart[3].p_uart = &huart4;

    __uart[2].rx_type = 1;
    __uart[3].rx_type = 1;

    for (int i = 0; i < 4; i++) {
        if (__uart[i].rx_hw_buffer_size > 0) {
            __uart[i].rx_hw_buffer[0] = pvPortMalloc(__uart[i].rx_hw_buffer_size);
            __uart[i].rx_hw_buffer[1] = pvPortMalloc(__uart[i].rx_hw_buffer_size);
        }
        if (__uart[i].tx_hw_buffer_size > 0) {
            __uart[i].tx_hw_buffer = pvPortMalloc(__uart[i].tx_hw_buffer_size);
        }
        if (__uart[i].rx_sw_buffer_size > 0) {
            __uart[i].rx_sw_buffer = pvPortMalloc(__uart[i].rx_sw_buffer_size);
        }
        init_list_head(&__uart[i].tx_list);
    }
}

//uint8_t debug2[42];
static void __uart_send(struct UART_BUFFER *uart, uint8_t *data, uint32_t len)
{
    struct SEND_DATA *send_data_item;

	osMutexWait(uart->tx_list_mtx, osWaitForever);

    send_data_item = (struct SEND_DATA *)pvPortMalloc(sizeof(struct SEND_DATA));
    if (!send_data_item) {
		osMutexRelease(uart->tx_list_mtx);
        return;
	}

    if (len > uart->tx_hw_buffer_size) {
        send_data_item->size = len > uart->tx_hw_buffer_size;
    } else {
        send_data_item->size = len;
    }

    send_data_item->data = (uint8_t *)pvPortMalloc(send_data_item->size);
    if (!send_data_item->data) {
        vPortFree(send_data_item);
		osMutexRelease(uart->tx_list_mtx);
        return;
    }

	//memset(debug2, 0xFF, 42);
    //memcpy(debug2, data, len);
    memcpy(send_data_item->data, data, len);

    list_add_tail(&send_data_item->node, &uart->tx_list);
    osMutexRelease(uart->tx_list_mtx);

    xTaskNotifyGive(uart_threadHandle);
}

static uint32_t __uart_send_and_read(struct UART_BUFFER *uart,
                        uint8_t *send_data, uint32_t send_len,
                        uint8_t *read_data, uint32_t *read_len,
                        TickType_t timeout)
{
	uint32_t wait_time = 0;

    uart->read_status = 1;
    uart->read_data = read_data;
    uart->read_len = read_len;
    __uart_send(uart, send_data, send_len);
    xSemaphoreTake(uart->sem, timeout);
	while(uart->read_status == 1) {
		if (wait_time > timeout) {
			print_log("wait timeout %lu/%lu\r\n", wait_time, timeout);
			break;
		}

		osDelay(10);
		wait_time += 10;
	}

    return uart->read_status;
}

void uart1_send(uint8_t *data, uint32_t len) { __uart_send(&__uart[0], data, len); }
void uart2_send(uint8_t *data, uint32_t len) { __uart_send(&__uart[1], data, len); }
void uart3_send(uint8_t *data, uint32_t len) { __uart_send(&__uart[2], data, len); }
void uart4_send(uint8_t *data, uint32_t len) { __uart_send(&__uart[3], data, len); }

/* return: 0-read success, 1-read timeout */
uint32_t uart1_send_and_read(uint8_t *send_data, uint32_t send_len, uint8_t *read_data, uint32_t *read_len, TickType_t timeout)
{ return __uart_send_and_read(&__uart[0], send_data, send_len, read_data, read_len, timeout); }
uint32_t uart2_send_and_read(uint8_t *send_data, uint32_t send_len, uint8_t *read_data, uint32_t *read_len, TickType_t timeout)
{ return __uart_send_and_read(&__uart[1], send_data, send_len, read_data, read_len, timeout); }
uint32_t uart3_send_and_read(uint8_t *send_data, uint32_t send_len, uint8_t *read_data, uint32_t *read_len, TickType_t timeout)
{ return __uart_send_and_read(&__uart[2], send_data, send_len, read_data, read_len, timeout); }
uint32_t uart4_send_and_read(uint8_t *send_data, uint32_t send_len, uint8_t *read_data, uint32_t *read_len, TickType_t timeout)
{ return __uart_send_and_read(&__uart[3], send_data, send_len, read_data, read_len, timeout); }

void register_data_handler(UART_HandleTypeDef *p_uart, data_handler func)
{
    for (int i = 0; i < 4; i++) {
        if (__uart[i].p_uart != p_uart)
            continue;

        __uart[i].data_handler_func = func;
        break;
    }
}

//uint8_t _debug[64];
void uart_thread_func(void const * argument)
{
    __uart[0].tx_list_mtx = uart1_mtxHandle;
    __uart[0].sem = uart1_send_semHandle;
    __uart[1].p_uart = &huart2;
    __uart[1].tx_list_mtx = uart2_mtxHandle;
    __uart[1].sem = uart2_send_semHandle;
    __uart[2].p_uart = &huart3;
    __uart[2].tx_list_mtx = uart3_mtxHandle;
    __uart[2].sem = uart3_send_semHandle;
    __uart[3].p_uart = &huart4;
    __uart[3].tx_list_mtx = uart4_mtxHandle;
    __uart[3].sem = uart4_send_semHandle;

    /* setup hw rx */
    for (int i = 0; i < 4; i++) {
        if (__uart[i].rx_hw_buffer_size == 1) {
            HAL_UART_Receive_IT(__uart[i].p_uart, __uart[i].rx_hw_buffer[0], 1);
        } else {
            HAL_UART_ReceiverTimeout_Config(__uart[i].p_uart,
                                            __uart[i].p_uart->Init.BaudRate / 100);  // 20ms
            HAL_UART_EnableReceiverTimeout(__uart[i].p_uart);
            HAL_UART_Receive_DMA(__uart[i].p_uart,
                                __uart[i].rx_hw_buffer[__uart[i].rx_hw_buffer_idx],
                                __uart[i].rx_hw_buffer_size);
        }
    }

    
    // print_log("uart_thread start\r\n");

    for (;;) {
        // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		osDelay(5);

        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

        /* get rx hw data */
        for (int i = 0; i < 4; i++) {
            if (__uart[i].rx_hw_buffer_len > 1) {
                if (__uart[i].rx_hw_buffer_len > __uart[i].rx_sw_buffer_size) {
                    __uart[i].rx_hw_buffer_len = __uart[i].rx_sw_buffer_size;
                    print_log("error: rx_sw_buffer[%d] oversize, %lu > %lu\r\n",
                            i, __uart[i].rx_hw_buffer_len, __uart[i].rx_sw_buffer_size);
                }
                uint32_t data_buffer_index = (__uart[i].rx_hw_buffer_idx + 1) & 0x01;
                memcpy(__uart[i].rx_sw_buffer,
                       __uart[i].rx_hw_buffer[data_buffer_index],
                       __uart[i].rx_hw_buffer_len);
                __uart[i].rx_len = __uart[i].rx_hw_buffer_len;
                __uart[i].rx_hw_buffer_len = 0;
            }
        }

        /* send data */
        for (int i = 0; i < 4; i++) {
            if (__uart[i].is_tx_busy == 0) {
                // get data
                struct SEND_DATA *send_data_item = NULL;
                osMutexWait(__uart[i].tx_list_mtx, osWaitForever);
                if (!list_empty(&__uart[i].tx_list)) {
                    send_data_item = list_first_entry(&__uart[i].tx_list, struct SEND_DATA, node);
                    list_del(&send_data_item->node);
                }
                osMutexRelease(__uart[i].tx_list_mtx);

                if (send_data_item != NULL) {
                    __uart[i].is_tx_busy = 1;

                    // send
					//memset(_debug, 0, 64);
                    //memcpy(_debug, send_data_item->data, send_data_item->size);
                    memcpy(__uart[i].tx_hw_buffer, send_data_item->data, send_data_item->size);

                    __uart[i].tx_hw_buffer[send_data_item->size] = 0;
                    if (i != 3)
                    	print_log("u%d T: %s\r\n", i, __uart[i].tx_hw_buffer);
                    if (i == 0 || i == 1) {
                        HAL_UART_Transmit_DMA(__uart[i].p_uart, __uart[i].tx_hw_buffer, send_data_item->size);
                    } else {
                        HAL_UART_Transmit_IT(__uart[i].p_uart, __uart[i].tx_hw_buffer, send_data_item->size);
                    }

                    // free
                    vPortFree(send_data_item->data);
                    vPortFree(send_data_item);
                }
            }
        }

        // handle received buffer
        for (int i = 0; i < 4; i++) {
            if (__uart[i].rx_len > 0) {
            	__uart[i].rx_sw_buffer[__uart[i].rx_len] = 0;
				print_log("u%d R: %s\r\n", i, __uart[i].rx_sw_buffer);
                if (__uart[i].read_status != 0) {
                    *__uart[i].read_len = __uart[i].rx_len;
                    memcpy(__uart[i].read_data, __uart[i].rx_sw_buffer, __uart[i].rx_len);
                    __uart[i].read_status = 0;
                    __uart[i].rx_len = 0;

                    xSemaphoreGive(__uart[i].sem);
                    
                    continue;
                }
                if (__uart[i].data_handler_func)
                    __uart[i].data_handler_func(__uart[i].rx_sw_buffer, __uart[i].rx_len);

                __uart[i].rx_len = 0;
            }
        }

    }
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    uint16_t len = 0;
    struct UART_BUFFER *uart = NULL;

    if (huart == &huart1) {
        uart = &__uart[0];
		HAL_DMA_Init(&hdma_usart1_rx);
    } else if (huart == &huart2) {
        uart = &__uart[1];
		HAL_DMA_Init(&hdma_usart2_rx);
    } else {
        return;
    }

    //if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)) {
        if((huart->ErrorCode & HAL_UART_ERROR_RTO) == HAL_UART_ERROR_RTO) {
            uint16_t dma_remaining_rx_data = (uint16_t)__HAL_DMA_GET_COUNTER(huart->hdmarx);
            len = uart->rx_hw_buffer_size - dma_remaining_rx_data;
        }
        uart->rx_hw_buffer_idx = (uart->rx_hw_buffer_idx + 1) & 0x1;
        HAL_UART_Receive_DMA(huart,
                             uart->rx_hw_buffer[uart->rx_hw_buffer_idx],
                             uart->rx_hw_buffer_size);
        if (len > 0) {
            uart->rx_hw_buffer_len = len;
            if (uart_threadHandle)
                vTaskNotifyGiveFromISR(uart_threadHandle, &pxHigherPriorityTaskWoken);
        }
    //}

    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    struct UART_BUFFER *uart = NULL;

    if (huart == &huart1) {
        uart = &__uart[0];
    } else if (huart == &huart2) {
        uart = &__uart[1];
    } else if (huart == &huart3) {
        uart = &__uart[2];
    } else if (huart == &huart4) {
        uart = &__uart[3];
    } else {
        return;
    }

    if (huart == &huart1 || huart == &huart2) {
        if (uart_threadHandle)
            vTaskNotifyGiveFromISR(uart_threadHandle, &pxHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
	uart->is_tx_busy = 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    struct UART_BUFFER *uart = NULL;

    if (huart == &huart1) {
        uart = &__uart[0];
    } else if (huart == &huart2) {
        uart = &__uart[1];
    } else if (huart == &huart3) {
        uart = &__uart[2];
    } else if (huart == &huart4) {
        uart = &__uart[3];
    } else {
        return;
    }

    if (uart->rx_type == 0) {
        uart->rx_hw_buffer_idx = (uart->rx_hw_buffer_idx + 1) & 0x1;
        HAL_UART_Receive_DMA(uart->p_uart,
                             uart->rx_hw_buffer[uart->rx_hw_buffer_idx],
                             uart->rx_hw_buffer_size);

        uart->rx_hw_buffer_len = uart->rx_hw_buffer_size;

        if (uart_threadHandle)
            vTaskNotifyGiveFromISR(uart_threadHandle, &pxHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    } else {
        if (uart->data_handler_func)
            uart->data_handler_func(uart->rx_hw_buffer[0], 1);
        HAL_UART_Receive_IT(uart->p_uart, uart->rx_hw_buffer[0], 1);
    }
}
