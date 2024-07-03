#ifndef __UART_THREAD_H__
#define __UART_THREAD_H__

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "my_list.h"
#include "usart.h"

typedef void (*data_handler)(uint8_t *data, uint32_t len);


void uart_thread_init(void);
void uart_thread_func(void const * argument);

void register_data_handler(UART_HandleTypeDef *p_uart, data_handler func);

void uart1_send(uint8_t *data, uint32_t len);
void uart2_send(uint8_t *data, uint32_t len);
void uart3_send(uint8_t *data, uint32_t len);
void uart4_send(uint8_t *data, uint32_t len);

/* return: 0-read success, 1-read timeout */
uint32_t uart1_send_and_read(uint8_t *send_data, uint32_t send_len,
                             uint8_t *read_data, uint32_t *read_len,
                             TickType_t timeout);
uint32_t uart2_send_and_read(uint8_t *send_data, uint32_t send_len,
                             uint8_t *read_data, uint32_t *read_len,
                             TickType_t timeout);
uint32_t uart3_send_and_read(uint8_t *send_data, uint32_t send_len,
                             uint8_t *read_data, uint32_t *read_len,
                             TickType_t timeout);
uint32_t uart4_send_and_read(uint8_t *send_data, uint32_t send_len,
                             uint8_t *read_data, uint32_t *read_len,
                             TickType_t timeout);


#endif
