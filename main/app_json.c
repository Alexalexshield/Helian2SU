#include "esp_system.h"
#include "esp_log.h"
#include "string.h"
#include "cJSON.h"

#include "config.h"
#include "app_json.h"

static const char *TAG = "JSON";


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
    cJSON_AddNumberToObject(samples, SIGNAL, (int) analog_signal[index]);
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





void app_json_deserialize(char * arg){
	ESP_LOGI(TAG, "Deserialize.....");
	cJSON *root = cJSON_Parse(arg);
	if (root == NULL){
		ESP_LOGI(TAG, "invalid command");
	}

	if (cJSON_GetObjectItem(root,SIGNAL) != NULL)
	{
        cJSON * array = cJSON_GetObjectItem(root, SIGNAL);

		for (uint16_t i = 0 ; i < sizeof(analog_signal)/sizeof(uint16_t); i++)
		{
    		analog_signal[i] = cJSON_GetArrayItem(array, i)->valueint;
            ESP_LOGI(TAG, "analog_signal[%d]=%d",i, analog_signal[i]);
		}
	}

	if (cJSON_GetObjectItem(root, DSP_COMMAND) !=NULL)
	{
		enum From_dsp_commands dsp_command = cJSON_GetObjectItem(root,"c")->valueint;
		switch (status)
		{
		case fd_search_ok:
			ESP_LOGI(TAG, "From DSP: %s", "quick search has been started");
			//xTaskCreate(main_activity_task, "start_vlf", 4096, NULL, 5, NULL);
			break;
		case fd_mute_ok:
			ESP_LOGI(TAG, "From DSP: %s", "current tag was muted");
			//xTaskCreate(quick_search_task, "start_vlf", 4096, NULL, 5, NULL);
			break;
		default:
			break;
		}	
	}

	if (cJSON_GetObjectItem(root, F_DSP_D1) !=NULL)
	{
		D1 = cJSON_GetObjectItem(root,F_DSP_D1)->valueint;
	}
	if (cJSON_GetObjectItem(root, F_DSP_D2) !=NULL)
	{
		D2 = cJSON_GetObjectItem(root,F_DSP_D2)->valueint;
	}
	if (cJSON_GetObjectItem(root, F_DSP_D3) !=NULL)
	{
		D3 = cJSON_GetObjectItem(root,F_DSP_D3)->valueint;
	}
	if (cJSON_GetObjectItem(root, F_DSP_TAG_ID) !=NULL)
	{
		current_tag_id = cJSON_GetObjectItem(root,F_DSP_TAG_ID)->valueint;
	}


    app_json_serialize();
	
    cJSON_Delete(root);
}