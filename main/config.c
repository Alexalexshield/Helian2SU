#include "config.h"

uint16_t analog_signal[19] = {0};
uint8_t battery_level = 100;



int16_t current_tag_id =-1;
int16_t search_unit_id =-1;
char command_to_dsp = '0';
char command_to_app = '0';
//triangulation data
//D1,D2,D3 - data between tag and search unit, m
int16_t D1 = -1;
int16_t D2 = -1;
int16_t D3 = -1;