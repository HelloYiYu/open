// https://www.cnblogs.com/schips/p/rtos-freertos-06.html

#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "debug.h"
#include "my_list.h"
#include "lp262.h"
#include "fsm.h"
#include "main.h"


#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

extern osMutexId fsm_event_mtxHandle;
extern osThreadId lp262_threadHandle;

struct FSM_STATE lp262_state_init;
struct FSM_STATE lp262_state_power_up;
struct FSM_STATE lp262_state_config;
struct FSM_STATE lp262_state_wait_wifi;
struct FSM_STATE lp262_state_wait_socket;
struct FSM_STATE lp262_state_entm;

static struct FSM_ACTION lp262_action_init[] = {
    {LP262_EVT_WELCOM, NULL, NULL, &lp262_state_power_up},
};

static struct FSM_ACTION lp262_action_power_up[] = {
    {LP262_EVT_INTERNAL, NULL, NULL, &lp262_state_config},
};

static struct FSM_ACTION lp262_action_config[] = {
    {LP262_EVT_INTERNAL, NULL, NULL, &lp262_state_wait_wifi},
};

static struct FSM_ACTION lp262_action_wait_wifi[] = {
    {LP262_EVT_INTERNAL, NULL, NULL, &lp262_state_wait_socket},
};

static struct FSM_ACTION lp262_action_wait_socket[] = {
    {LP262_EVT_INTERNAL, NULL, NULL, &lp262_state_entm},
};

static struct FSM_ACTION lp262_action_entm[] = {
    {LP262_EVT_INTERNAL, NULL, NULL, &lp262_state_init},
};

struct FSM_STATE lp262_state_init = {
    .name = "init",
    .enter_func = NULL,
    .leave_func = NULL,
    .action_list_count = ARRAY_SIZE(lp262_action_init),
    .action_list = lp262_action_init,
};

struct FSM_STATE lp262_state_power_up = {
    .name = "power_up",
    .enter_func = NULL,
    .leave_func = NULL,
    .action_list_count = ARRAY_SIZE(lp262_action_power_up),
    .action_list = lp262_action_power_up,
};

struct FSM_STATE lp262_state_config = {
    .name = "config",
    .enter_func = NULL,
    .leave_func = NULL,
    .action_list_count = ARRAY_SIZE(lp262_action_config),
    .action_list = lp262_action_config,
};

struct FSM_STATE lp262_state_wait_wifi = {
    .name = "wait_wifi",
    .enter_func = NULL,
    .leave_func = NULL,
    .action_list_count = ARRAY_SIZE(lp262_action_wait_wifi),
    .action_list = lp262_action_wait_wifi,
};

struct FSM_STATE lp262_state_wait_socket = {
    .name = "wait_socket",
    .enter_func = NULL,
    .leave_func = NULL,
    .action_list_count = ARRAY_SIZE(lp262_action_wait_socket),
    .action_list = lp262_action_wait_socket,
};

struct FSM_STATE lp262_state_entm = {
    .name = "entm",
    .enter_func = NULL,
    .leave_func = NULL,
    .action_list_count = ARRAY_SIZE(lp262_action_entm),
    .action_list = lp262_action_entm,
};


static struct FSM_ENTITY lp262_fsm = {
    .name = "lp262",
    .state = &lp262_state_init,
};
static struct list_head lp262_event_list;


void lp262_thread_init(void)
{
    init_list_head(&lp262_event_list);
}

/* param should be NULL or a pointer fraom pvPortMalloc */
void lp262_send_event(uint32_t event_id, void *param)
{
    struct FSM_EVENT *event;

    event = (struct FSM_EVENT *)pvPortMalloc(sizeof(struct FSM_EVENT));
    if (!event)
        return;

    event->event_id = event_id;
    event->param = param;

    osMutexWait(fsm_event_mtxHandle, osWaitForever);
    list_add_tail(&event->node, &lp262_event_list);
    osMutexRelease(fsm_event_mtxHandle);

    xTaskNotifyGive(lp262_threadHandle);
}


static uint8_t lp262_rx_buffer[256];
static uint32_t buffer_offset = 0;
void clear_buffer(void)
{
    buffer_offset = 0;
    memset(lp262_rx_buffer, 0, 256);
}

bool clear_buffer_and_wait_str(char *str, uint32_t timeout)
{
    uint32_t wait = 0;

    clear_buffer();

    while (strstr((char *)lp262_rx_buffer, (char *)str) == NULL) {
        osDelay(10);
        wait += 10;
        if (wait >= timeout)
            return false;
    }
    return true;
}

uint32_t send_and_wait_str(char *send_str, char *wait_str, uint32_t timeout)
{
    uint32_t ret = 0;
    uint32_t wait_len = 0;
    uint8_t read[64];

    osDelay(20);
    ret = uart2_send_and_read((uint8_t *)send_str, strlen(send_str), read, &wait_len, timeout);
    print_log("wait: %s\r\n", read);
    
    if (strstr(read, wait_str) == NULL)
        return 0;
    else
        return 1;
}

void lp262_hw_reset(void)
{
    HAL_GPIO_WritePin(LP_nReset_GPIO_Port, LP_nReset_Pin, GPIO_PIN_RESET);
    osDelay(1000);
    HAL_GPIO_WritePin(LP_nReset_GPIO_Port, LP_nReset_Pin, GPIO_PIN_SET);
}

bool lp262_factory_reset(void)
{
    uint32_t wait_len = 0;

    wait_len = send_and_wait_str("AT+RELD\n", "+ok=rebooting...", 500);
    if (wait_len == 0)
        return false;

    return true;
}

bool lp262_reboot(void)
{
    uint32_t wait_len = 0;

    osDelay(100);
    wait_len = send_and_wait_str("AT+Z\n", "+ok", 500);
    if (wait_len == 0)
        return false;

    return true;
}

bool wait_device_and_enable_AT(void)
{
    uint32_t wait_len = 0;

    clear_buffer_and_wait_str("HF-LPT262", 0xFFFFFFFF);
    osDelay(500);
    uart2_send("+", 1);
    osDelay(300);
    uart2_send("+", 1);
    osDelay(300);
    uart2_send("+", 1);
    osDelay(1000);
    //wait_len = send_and_wait_str("+", "a", 500);
    //if (wait_len == 0)
    //    return false;

    wait_len = send_and_wait_str("a", "+ok", 3000);
    osDelay(1000);


    wait_len = send_and_wait_str("AT+\n", "+ok", 3000);
    osDelay(1000);


    wait_len = send_and_wait_str("AT+EVENT=off\n", "+ok", 3000);
    osDelay(1000);



    return true;
}

bool is_device_already_configed(void)
{
    uint32_t wait_len = 0;

    osDelay(20);
    wait_len = send_and_wait_str("AT+NETP\n", "www.helloyiyu.com", 5000);
    if (wait_len == 0)
        return false;

    return true;
}

bool config(void)
{
    uint32_t wait_len = 0;

    osDelay(20);
    wait_len = send_and_wait_str("AT+WMODE=sta\n", "+ok", 3000);
    osDelay(1000);

    wait_len = send_and_wait_str("AT+WSSSID=demo\n", "+ok", 3000);
    osDelay(1000);

    wait_len = send_and_wait_str("AT+WSKEY=12345689\n", "+ok", 3000);
    osDelay(1000);

    wait_len = send_and_wait_str("AT+NETP=MQTT,1883,www.helloyiyu.com\n", "+ok", 3000);
    osDelay(1000);

    wait_len = send_and_wait_str("AT+MQLOGIN=yiyu,\\2C\\2CFengyiyu1234\n", "+ok", 3000);
    osDelay(1000);

    wait_len = send_and_wait_str("AT+MQTOPIC=demo/up,demo/down\n", "+ok", 3000);
    osDelay(1000);

    return true;
}

bool lp262_wifi_connected(void)
{
    uint32_t wait_len = 0;

    wait_len = send_and_wait_str("AT+WSLK\n", "+ok=demo", 3000);
    if (wait_len == 0)
        return false;

    return true;
}

bool lp262_socket_connected(void)
{
    uint32_t wait_len = 0;

    wait_len = send_and_wait_str("AT+TCPLK\n", "+ok=on", 3000);
    if (wait_len == 0)
        return false;

    return true;
}

bool lp262_enter_ENTM(void)
{
    uint32_t wait_len = 0;

    wait_len = send_and_wait_str("AT+ENTM\n", "+ok", 3000);
    if (wait_len == 0)
        return false;

    return true;
}

void lp262_rx(uint8_t *data, uint32_t len)
{
    if (buffer_offset + len > 256)
        return;

    memcpy(&lp262_rx_buffer[buffer_offset], data, len);
    buffer_offset += len;
}


void lp262_thread_func(void const * argument)
{
    struct list_head temp_list;
    struct FSM_EVENT *event, *next;
    uint32_t wait_time = 0;

    init_list_head(&temp_list);

    osDelay(1000);
    print_log("%s started.\r\n", __func__);

    register_data_handler(&huart2, lp262_rx);
    lp262_hw_reset();

    do {
        if (!wait_device_and_enable_AT())
            break;
        print_log("lp262 AT ready\r\n");

        if (!is_device_already_configed()) {
            print_log("lp262 enter config mode\r\n");
            if (!config()) {
                print_log("lp262 config failed\r\n");
                break;
            }
            if (!lp262_reboot()) {
                print_log("lp262 reboot\r\n");
                break;
            }
            continue;
        } else {
            print_log("lp262 wait wifi connect\r\n");
            wait_time = 0;
            while (wait_time < 100000) {
                if (lp262_wifi_connected())
                    break;
                wait_time += 500;
            }
            if (lp262_wifi_connected()) {
                print_log("lp262 wait wifi connect timeout\r\n");
                break;
            }
            
            print_log("lp262 wait socket connect\r\n");
            wait_time = 0;
            while (wait_time < 100000) {
                if (lp262_socket_connected())
                    break;
                wait_time += 500;
            }
            if (lp262_socket_connected()) {
                print_log("lp262 wait socket connect timeout\r\n");
                break;
            }

            if (!lp262_enter_ENTM()) {
                print_log("lp262 failed to enter ENTM\r\n");
                break;
            }

            print_log("lp262 init done\r\n");
            while(1) {
                osDelay(10);
            }
        }

    } while(1);


    print_log("lp262 init failed\r\n");
    for(;;){osDelay(10);}

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);

        osMutexWait(fsm_event_mtxHandle, osWaitForever);
        list_splice_init(&lp262_event_list, &temp_list);
        osMutexRelease(fsm_event_mtxHandle);

        list_for_each_entry_safe(event, next, &temp_list, node) {
            fsm_handle_event(&lp262_fsm, event);

            if (event->param)
                vPortFree(event->param);
            list_del(&event->node);
            vPortFree(event);
        }

        if(!list_empty(&temp_list)) {
            // never run to here
        }
    }
}

