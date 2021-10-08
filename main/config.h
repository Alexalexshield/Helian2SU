#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>

// #define DEBUG_WITHOUT_STM


#define SEARCH_UNIT_ID 0x01

extern double mag;
extern double amp[3]; //[X,Y,Z];
extern uint16_t analog_signal[19];
extern uint8_t battery_level;

extern int16_t current_tag_id;
extern int16_t search_unit_id;


typedef struct
{
    uint16_t id;
    uint16_t st;
    float mag;
    float amp[3];
    float dist;
    float dir;
} MSG_DSP;

typedef struct
{
    uint16_t id;
    uint16_t st;
    uint16_t cmd;
} MSG_APP;


#endif