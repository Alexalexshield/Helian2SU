#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>

#define DEBUG_WITHOUT_STM

extern uint16_t analog_signal[19];
extern uint8_t battery_level;

extern int16_t current_tag_id;
extern int16_t search_unit_id;

extern char command_to_dsp;
extern char command_to_app;

extern int16_t D1;
extern int16_t D2;
extern int16_t D3;


#endif