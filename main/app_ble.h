#ifndef _APP_BLE_H
#define _APP_BLE_H

#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define DEVICE_NAME "Helian2SU"



void ble_app_advertise(void);
void ble_init(void);

void ble_app_on_sync(void);

void send_spp_data(TimerHandle_t xTimer);
void update_batter_status(TimerHandle_t xTimer);


#endif