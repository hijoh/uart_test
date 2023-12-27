#include <stdio.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"

#define BUF_SIZE (1024) // 接收缓冲区大小

static const char *TAG = "example";
static QueueHandle_t uart_queue; // 串口数据队列
TimerHandle_t timers[GPIO_NUM_MAX];

void print_task(void *pvParameters)
{
    while (1)
    {
        printf("hello_world\n");
        vTaskDelay(1000 * 30 / portTICK_PERIOD_MS);
    }
}

/*
void timer_callback(TimerHandle_t xTimer)
{
    gpio_info_t *gpio_info = (gpio_info_t *)pvTimerGetTimerID(xTimer);
    switch (gpio_info->repeat) {
        case 1:
            gpio_info->level = !gpio_info->level;
            gpio_set_level(gpio_info->gpio_number, gpio_info->level);
            xTimerStart(gpio_info->timer, 0);
            break;
        case 2:
            gpio_reset_pin(gpio_info->gpio_number);
            xTimerDelete(gpio_info->timer, portMAX_DELAY);
            free(gpio_info);
            break;
        default:
            gpio_set_level(gpio_info->gpio_number, gpio_info->level);
            if (gpio_info->repeat == 0) {
                xTimerDelete(gpio_info->timer, portMAX_DELAY);
                free(gpio_info);
            }
            break;
    }
}

void uart_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t data[BUF_SIZE];
    size_t size;
    int gpio_number;

    while (1) {
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    size = uart_read_bytes(UART_NUM_0, data, event.size, portMAX_DELAY);
                    if (size > 0) {
                        if (size == 10 && data[0] == '9' && data[1] == '9' &&
                            data[2] == '9' && data[3] == '9' && data[4] == '9' &&
                            data[5] == '9' && data[6] == '9' && data[7] == '9' && 
                            data[8] == '9' && data[9] == '9') {
                            printf("Connect Successful\n");
                        } else if (size == 10 && isdigit(data[0]) && isdigit(data[1]) &&
                            isdigit(data[2]) && isdigit(data[3]) && isdigit(data[4]) &&
                            isdigit(data[5]) && isdigit(data[6]) && isdigit(data[7]) && 
                            isdigit(data[8]) && isdigit(data[9])) {
                            gpio_info_t *gpio_info = malloc(sizeof(gpio_info_t));
                            gpio_number = (data[0] - '0') * 10 + (data[1] - '0');
                            gpio_info->gpio_number = (data[0] - '0') * 10 + (data[1] - '0');
                            if (timers[gpio_number] != NULL)
                            {
                                // 如果存在，我们删除它：
                                xTimerDelete(timers[gpio_number], portMAX_DELAY);
                                timers[gpio_number] = NULL;
                            }
                            int delay_hours = (data[2] - '0') * 10 + (data[3] - '0');
                            int delay_minutes = (data[4] - '0') * 10 + (data[5] - '0');
                            int delay_seconds = (data[6] - '0') * 10 + (data[7] - '0');
                            gpio_info->level = data[8] - '0';
                            gpio_info->repeat = data[9] - '0';
                            if (delay_hours > 0 || delay_minutes > 0 || delay_seconds > 0) {
                                gpio_info->timer = xTimerCreate("timer", pdMS_TO_TICKS(delay_hours * 3600000 + delay_minutes * 60000 + delay_seconds * 1000), pdFALSE, (void *)gpio_info, timer_callback);
                                xTimerStart(gpio_info->timer, 0);
                                timers[gpio_info->gpio_number] = gpio_info->timer;
                            } else {
                                if (gpio_info->repeat == 0) {
                                    gpio_set_level(gpio_info->gpio_number, gpio_info->level);
                                    // free(gpio_info);
                                } else if (gpio_info->repeat == 1) {
                                    gpio_set_level(gpio_info->gpio_number, !gpio_info->level);
                                } else if (gpio_info->repeat == 2) {
                                    gpio_reset_pin(gpio_info->gpio_number);
                                    xTimerDelete(gpio_info->timer, portMAX_DELAY);
                                    free(gpio_info);
                                } else if (gpio_info->repeat == 3) {
                                     xTimerDelete(gpio_info->timer, portMAX_DELAY);
                                    free(gpio_info);
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
*/

/*
typedef struct {
    TimerHandle_t timer;
    int gpio_number;
    bool level;
    bool repeat;
} gpio_info_t;

void timer_callback(TimerHandle_t xTimer)
{
    gpio_info_t *gpio_info = (gpio_info_t *)pvTimerGetTimerID(xTimer);
    gpio_set_level(gpio_info->gpio_number, gpio_info->level);
    if (gpio_info->repeat) {
        gpio_info->level = !gpio_info->level;
        xTimerStart(gpio_info->timer, 0);
    } else {
        xTimerDelete(gpio_info->timer, portMAX_DELAY);
        free(gpio_info);
    }
}

void uart_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t data[BUF_SIZE];
    size_t size;

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        while (1) {
            if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY)) {
                switch (event.type) {
                    case UART_DATA:
                        size = uart_read_bytes(UART_NUM_0, data, event.size, portMAX_DELAY);
                        if (size > 0) {
                            err = nvs_set_blob(my_handle, "uart_data", data, size);
                            if (err != ESP_OK) {
                                printf("Error (%s) writing to NVS!\n", esp_err_to_name(err));
                            }
                            nvs_commit(my_handle);

                            // Rest of the code...
                            if (size == 10 && data[0] == '9' && data[1] == '9' &&
                            data[2] == '9' && data[3] == '9' && data[4] == '9' &&
                            data[5] == '9' && data[6] == '9' && data[7] == '9' &&
                            data[8] == '9' && data[9] == '9') {
                            printf("Connect Successful\n");
                        } else if (size == 10 && isdigit(data[0]) && isdigit(data[1]) &&
                            isdigit(data[2]) && isdigit(data[3]) && isdigit(data[4]) &&
                            isdigit(data[5]) && isdigit(data[6]) && isdigit(data[7]) &&
                            isdigit(data[8]) && isdigit(data[9])) {
                            gpio_info_t *gpio_info = malloc(sizeof(gpio_info_t));
                            gpio_info->gpio_number = (data[0] - '0') * 10 + (data[1] - '0');
                            int delay_hours = (data[2] - '0') * 10 + (data[3] - '0');
                            int delay_minutes = (data[4] - '0') * 10 + (data[5] - '0');
                            int delay_seconds = (data[6] - '0') * 10 + (data[7] - '0');
                            gpio_info->level = data[8] - '0';
                            gpio_info->repeat = data[9] - '0';
                            if (delay_hours > 0 || delay_minutes > 0 || delay_seconds > 0) {
                                gpio_info->timer = xTimerCreate("timer", pdMS_TO_TICKS(delay_hours * 3600000 + delay_minutes * 60000 + delay_seconds * 1000), pdFALSE, (void *)gpio_info, timer_callback);
                                xTimerStart(gpio_info->timer, 0);
                            } else {
                                gpio_set_level(gpio_info->gpio_number, gpio_info->level);
                                if (!gpio_info->repeat) {
                                    free(gpio_info);
                                }
                            }
                        }
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    nvs_close(my_handle);
}
*/

typedef struct {
    TimerHandle_t timer;
    int gpio_number;
    bool level;
    int repeat;
} gpio_info_t;

// 创建一个全局的数组来存储所有的gpio_info_t指针
gpio_info_t *gpio_infos[GPIO_NUM_MAX] = {NULL};

void timer_callback(TimerHandle_t xTimer)
{
    gpio_info_t *gpio_info = (gpio_info_t *)pvTimerGetTimerID(xTimer);
    switch (gpio_info->repeat) {
        case 1:
            gpio_info->level = !gpio_info->level;
            gpio_set_level(gpio_info->gpio_number, gpio_info->level);
            xTimerStart(gpio_info->timer, 0);
            break;
        case 2:
            gpio_reset_pin(gpio_info->gpio_number);
            xTimerDelete(gpio_info->timer, portMAX_DELAY);
            gpio_infos[gpio_info->gpio_number] = NULL;  // 从数组中移除gpio_info
            free(gpio_info);
            break;
        default:
            gpio_set_level(gpio_info->gpio_number, gpio_info->level);
            if (gpio_info->repeat == 0) {
                xTimerDelete(gpio_info->timer, portMAX_DELAY);
                gpio_infos[gpio_info->gpio_number] = NULL;  // 从数组中移除gpio_info
                free(gpio_info);
            }
            break;
    }
}

void uart_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t data[BUF_SIZE];
    size_t size;
    int gpio_number;

    while (1) {
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    size = uart_read_bytes(UART_NUM_0, data, event.size, portMAX_DELAY);
                    if (size > 0) {
                        if (size == 10 && data[0] == '9' && data[1] == '9' &&
                            data[2] == '9' && data[3] == '9' && data[4] == '9' &&
                            data[5] == '9' && data[6] == '9' && data[7] == '9' && 
                            data[8] == '9' && data[9] == '9') {
                            printf("Connect Successful\n");
                        } else if (size == 10 && isdigit(data[0]) && isdigit(data[1]) &&
                            isdigit(data[2]) && isdigit(data[3]) && isdigit(data[4]) &&
                            isdigit(data[5]) && isdigit(data[6]) && isdigit(data[7]) && 
                            isdigit(data[8]) && isdigit(data[9])) {
                            gpio_number = (data[0] - '0') * 10 + (data[1] - '0');
                            // 如果已经存在一个gpio_info，我们先删除它
                            if (gpio_infos[gpio_number] != NULL) {
                                xTimerDelete(gpio_infos[gpio_number]->timer, portMAX_DELAY);
                                free(gpio_infos[gpio_number]);
                                gpio_infos[gpio_number] = NULL;
                            }
                            gpio_info_t *gpio_info = malloc(sizeof(gpio_info_t));
                            gpio_info->gpio_number = gpio_number;
                            int delay_hours = (data[2] - '0') * 10 + (data[3] - '0');
                            int delay_minutes = (data[4] - '0') * 10 + (data[5] - '0');
                            int delay_seconds = (data[6] - '0') * 10 + (data[7] - '0');
                            gpio_info->level = data[8] - '0';
                            gpio_info->repeat = data[9] - '0';
                            // 将新的gpio_info添加到数组中
                            gpio_infos[gpio_number] = gpio_info;
                            if (delay_hours > 0 || delay_minutes > 0 || delay_seconds > 0) {
                                gpio_info->timer = xTimerCreate("timer", pdMS_TO_TICKS(delay_hours * 3600000 + delay_minutes * 60000 + delay_seconds * 1000), pdFALSE, (void *)gpio_info, timer_callback);
                                xTimerStart(gpio_info->timer, 0);
                            } else {
                                if (gpio_info->repeat == 0) {
                                    gpio_set_level(gpio_info->gpio_number, gpio_info->level);
                                    gpio_infos[gpio_info->gpio_number] = NULL;  // 从数组中移除gpio_info
                                    free(gpio_info);
                                } else if (gpio_info->repeat == 1) {
                                    gpio_set_level(gpio_info->gpio_number, !gpio_info->level);
                                } else if (gpio_info->repeat == 2) {
                                    gpio_reset_pin(gpio_info->gpio_number);
                                    xTimerDelete(gpio_info->timer, portMAX_DELAY);
                                    gpio_infos[gpio_info->gpio_number] = NULL;  // 从数组中移除gpio_info
                                    free(gpio_info);
                                } else if (gpio_info->repeat == 3) {
                                    if (timers[gpio_number] != NULL) {
                                        xTimerDelete(timers[gpio_number], portMAX_DELAY);
                                        timers[gpio_number] = NULL;
                                        gpio_infos[gpio_info->gpio_number] = NULL;  // 从数组中移除gpio_info
                                        free(gpio_info);
                                    }
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}


void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // 配置串口参数
    uart_param_config(UART_NUM_0, &uart_config);

    // 安装串口驱动程序
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0);

    // 创建串口任务
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 10, NULL);

    // xTaskCreate(print_task, "print_task", 2048, NULL, 10, NULL);

    // 初始化 GPIO
    gpio_reset_pin(GPIO_NUM_10);
    gpio_set_direction(GPIO_NUM_10, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_10, 1); // 设置GPIO为低电平
    gpio_reset_pin(GPIO_NUM_6);
    gpio_set_direction(GPIO_NUM_6, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_6, 1); // 设置GPIO为低电平
    gpio_reset_pin(GPIO_NUM_4);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 1); // 设置GPIO为低电平

    ESP_LOGI(TAG, "UART example started");
}