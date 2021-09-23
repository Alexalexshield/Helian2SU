#include <string.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "cJSON.h"

#include "config.h"
#include "app_json.h"


#define TXD_PIN 4
#define RXD_PIN 5
#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE 256
#define PATTERN_LEN 3

#define TAG "UART"


QueueHandle_t uart_queue;


void uart_event_task(void*params)
{
    uart_event_t uart_event;   
    char *received_buffer = malloc(RX_BUF_SIZE);
    while (true)
    {
        if(xQueueReceive(uart_queue, &uart_event, portMAX_DELAY))
        {
            switch (uart_event.type)
            {
            case(UART_DATA):       
                ESP_LOGI(TAG,"UART data event");
                uart_read_bytes(UART_NUM_1, received_buffer, uart_event.size, portMAX_DELAY);
                app_json_deserialize(received_buffer);
                break;
            case(UART_BREAK):      
                ESP_LOGI(TAG,"UART break event");
                break;
            case(UART_BUFFER_FULL):
                ESP_LOGI(TAG,"UART RX buffer full event");
                break;
            case(UART_FIFO_OVF):   
                ESP_LOGI(TAG, "UART FIFO overflow event");
                uart_flush(UART_NUM_1);
                xQueueReset(uart_queue); 
                break;
            case(UART_FRAME_ERR):  
                ESP_LOGI(TAG,"UART RX frame error event");
                break;
            case(UART_PARITY_ERR): 
                ESP_LOGI(TAG, "UART RX parity event");
                break;
            case(UART_DATA_BREAK): 
                ESP_LOGI(TAG, "UART TX data and break event");
                break;
            case(UART_PATTERN_DET):
                ESP_LOGI(TAG, "UART pattern detected ");
                break;
            default:
                break;
            }
        }
    }
    
}

void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE, TX_BUF_SIZE, 20, &uart_queue, 0);

    // uart_pattern_queue_reset(UART_NUM_1, 20);
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 10, NULL);
}
