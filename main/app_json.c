#include "esp_system.h"
#include "esp_log.h"
#include "string.h"
#include "cJSON.h"

#include "config.h"
#include "app_json.h"



//"s" - analog signal, array of uint16_t [190] example: {"s":[13,32,255,14,14]} 
//TODO: send analog signal via pattern interrupt
//"c" - command from DSP

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
        cJSON * array = cJSON_GetObjectItem(root, "s");

		for (uint16_t i = 0 ; i < sizeof(analog_signal)/sizeof(uint16_t); i++)
		{
    		analog_signal[i] = cJSON_GetArrayItem(array, i)->valueint;
            ESP_LOGI(TAG, "analog_signal[%d]=%d",i, analog_signal[i]);
		}
	}


    app_json_serialize();
	// if (cJSON_GetObjectItem(root, "status") !=NULL)
	// {
	// 	int status = cJSON_GetObjectItem(root,"status")->valueint;
	// 	switch (status)
	// 	{
	// 	case MAIN_ACTIVITY:
	// 		ESP_LOGI(TAG, "status=%s", "MAIN_ACTIVITY");
	// 		//xTaskCreate(main_activity_task, "start_vlf", 4096, NULL, 5, NULL);
	// 		break;
	// 	case QUICK_SEARCH:
	// 		ESP_LOGI(TAG, "status=%s", "QUICK_SEARCH");
	// 		//xTaskCreate(quick_search_task, "start_vlf", 4096, NULL, 5, NULL);
	// 		break;
	// 	case ID_SEARCH:
	// 		ESP_LOGI(TAG, "status=%s", "ID_SEARCH");
	// 		//xTaskCreate(id_search_task, "start_vlf", 4096, NULL, 5, NULL);
	// 		break;
	// 	case CALIBRATION:
	// 		ESP_LOGI(TAG, "status=%s", "CALIBRATION");
	// 		//xTaskCreate(calibration_task, "start_vlf", 4096, NULL, 5, NULL);
	// 		break;
	// 	default:
	// 		break;
	// 	}	
	// }

    cJSON_Delete(root);
}