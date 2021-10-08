#ifndef _APP_UART_H
#define _APP_UART_H

#include "driver/uart.h"



void uart_init(void);
void uart_dsp_event_task(void*params);

extern QueueHandle_t uart_dsp_queue;

#endif