#include "esp_system.h"
#include "esp_log.h"
#include "string.h"
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "config.h"
#include "app_json.h"
#include "app_ble.h"

static const char *TAG = "JSON";


union { 
 struct { 
  uint8_t mobility:4; 
  uint8_t xpt:1; 
  uint8_t bit6:1; 
  uint8_t batteryproblem:1; 
  uint8_t masked:1; 
 } s ; 
 uint8_t status; 
} m ;


char *app_json_serialize(void){
	ESP_LOGI(TAG, "Serialize.....");
	cJSON *root;
    cJSON *samples;
	root = cJSON_CreateObject();
    samples = cJSON_CreateIntArray(analog_signal,sizeof(analog_signal));
	// esp_chip_info_t chip_info;
	// esp_chip_info(&chip_info);
    //cJSON_AddArrayToObject(root, SIGNAL);

for (int index = 0; index < (sizeof(analog_signal) / (sizeof(uint16_t))); ++index)
{
    cJSON_AddNumberToObject(samples, "s", (int) analog_signal[index]);
    if (samples == NULL)
    {
        cJSON_Delete(samples);
        return '\0';
    }
}

    cJSON_AddArrayToObject(root,samples);

	// cJSON_AddNumberToObject(root, "command", 1);
	// cJSON_AddNumberToObject(root, "freq", 8000);
	// cJSON_AddNumberToObject(root, "power", 100);
	// cJSON_AddStringToObject(root, "modulation", "ASK"); //"FSK"
	// cJSON_AddTrueToObject(root, "vlf_tx_start");
	// cJSON_AddFalseToObject(root, "flag_false");
	char *my_json_string = cJSON_Print(root);
	ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);
	cJSON_Delete(root);
    return my_json_string;
}


//MSG example from DSP:
//{"id":1234,"st":123,"mag":0.123,"amp":[0.123,0.456,0.789],"dis":0.123,"dir":123}

void dsp_json_deserialize(char * arg){
	ESP_LOGI(TAG, "Deserialize.....");
	cJSON *root = cJSON_Parse(arg);
	if (root == NULL){
		ESP_LOGI(TAG, "invalid command");
	}


	//send data to APP
	strcpy(ble_send_msg, arg);
	// ble_send_msg[strlen(arg)]='\0';

	// strncpy(ble_send_msg, arg,sizeof(ble_send_msg));
	// ble_send_msg[sizeof(ble_send_msg)-1]='\0';

	ESP_LOGI(TAG, "ARG %s",ble_send_msg);
	send_ble_cmd_notification();

	// memset(ble_send_msg, 0, sizeof ble_send_msg);

	if (cJSON_GetObjectItem(root, "id") !=NULL)
	{
		current_tag_id = cJSON_GetObjectItem(root,"id")->valueint;
		ESP_LOGI(TAG, "ID=%d", current_tag_id);
	}

	if (cJSON_GetObjectItem(root, "st") !=NULL)
	{
		uint8_t state = cJSON_GetObjectItem(root,"st")->valueint;
		m.status = state;
		if (m.s.xpt){
			ESP_LOGI(TAG, "xpt");
		}
		else {
			ESP_LOGI(TAG, "hpt");
		}
		if (m.s.masked){
			/* muted */
			ESP_LOGI(TAG, "muted");
		}
		if (m.s.batteryproblem){
			/* battery problem */
			ESP_LOGI(TAG, "battery problem");
		}
	}

	if (cJSON_GetObjectItem(root, "mag") !=NULL)
	{
		mag = cJSON_GetObjectItem(root,"st")->valuedouble;
		ESP_LOGI(TAG, "magnitude=%lf", mag);
	}

	if (cJSON_GetObjectItem(root, "amp") != NULL)
	{
	    cJSON * array = cJSON_GetObjectItem(root, "amp");

		for (uint16_t i = 0 ; i < sizeof(amp)/sizeof(double); i++)
		{
    		amp[i] = cJSON_GetArrayItem(array, i)->valuedouble;
            ESP_LOGI(TAG, "analog_signal[%d]=%lf",i, amp[i]);
		}
	}
    cJSON_Delete(root);
}