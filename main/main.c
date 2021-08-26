/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"


#define DEVICE_NAME "UNDERGROUND BEACON"
uint8_t ble_addr_type;
void ble_app_advertise(void);

static xTimerHandle timer_handler;
static xTimerHandle timer_handler_spp;
uint16_t battery_char_att_hdl;
uint16_t spp_char_att_hdl;
uint16_t conn_hdl;


#define DEVICE_INFO_SERVICE 0x180A
//characteristics
    #define MANUFACTURER_NAME 0x2A29
    #define FIRMWARE_REVISION 0x2A26

#define SPP_SERVICE 0xABF0
    #define SPP_DATA_RECEIVE      0xABF1    //receive big data from client
    #define SPP_DATA_NOTIFY       0xABF2    //send big data to client
    #define SPP_COMMAND_RECEIVE   0xABF3    //receive chort data from client
    #define SPP_COMMAND_NOTIFY    0xABF4    //send short data to client


#define BATTERY_SERVICE                          0x180F
//characteristics
    #define BATTERY_LEVEL                        0x2A19



static int dump_cb(uint16_t conn_handler, uint16_t attr_handler, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("dumb call back from: %d\n", attr_handler);

    char xyzaxis[20];
    xyzaxis[0]='X';
    xyzaxis[1]='Y';
    xyzaxis[2]='Z';
    xyzaxis[3]='=';
    for (uint8_t i = 4; i < sizeof(xyzaxis)/sizeof(xyzaxis[0]); i++)
    {
        xyzaxis[i] = (char)(i%3 + rand()%10); 
    }

    os_mbuf_append(ctxt->om, &xyzaxis, sizeof(xyzaxis));

     printf("%s\n", xyzaxis);

    return 0;
}

static int device_write(uint16_t conn_handler, uint16_t attr_handler, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("incoming message: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
    return 0;
}

static int device_info(uint16_t conn_handler, uint16_t attr_handler, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char * message = (char *) arg;
    os_mbuf_append(ctxt->om, message, strlen(message));
    return 0;
}

static int battery_read(uint16_t conn_handler, uint16_t attr_handler, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    static uint8_t level = 50;
    os_mbuf_append(ctxt->om, &level, sizeof(level));
    return 0;
}


uint8_t battery_level = 100;
void update_batter_status()
{
    if (battery_level-- == 10)
    {
        battery_level = 100;
    }
    printf("notify battery level is %d\n", battery_level);
    struct os_mbuf *om = ble_hs_mbuf_from_flat(&battery_level,sizeof(battery_level)); 
    ble_gattc_notify_custom(conn_hdl, battery_char_att_hdl, om);   
}

uint8_t xyzaxis[19];
void send_spp_data()
{
    // xyzaxis[0]='X';
    // xyzaxis[1]='Y';
    // xyzaxis[2]='Z';
    // xyzaxis[3]='=';
    for (uint8_t i = 0; i < sizeof(xyzaxis)/sizeof(xyzaxis[0]); i++)
    {
        xyzaxis[i] = (i%3 + 10*(rand()%10)); //beauty graph
        printf("%d", xyzaxis[i]);
    }
    xyzaxis[18]='\0';
    printf("\n");
    struct os_mbuf *om = ble_hs_mbuf_from_flat(xyzaxis,sizeof(xyzaxis)); 
    ble_gattc_notify_custom(conn_hdl, spp_char_att_hdl, om);   
}

static const struct ble_gatt_svc_def gat_svcs[] ={
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(DEVICE_INFO_SERVICE),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid       = BLE_UUID16_DECLARE(MANUFACTURER_NAME),
                .flags      = BLE_GATT_CHR_F_READ,
                .access_cb  = device_info,
                .arg        = "MineRadioSystems"
            },
            {
                .uuid       = BLE_UUID16_DECLARE(FIRMWARE_REVISION),
                .flags      = BLE_GATT_CHR_F_READ,
                .access_cb  = device_info,
                .arg        = "1v01",
            },{0}}},
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BATTERY_SERVICE),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid       = BLE_UUID16_DECLARE(BATTERY_LEVEL),
                .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &battery_char_att_hdl,
                .access_cb  = battery_read
            },{0}}},
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SPP_SERVICE),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid       = BLE_UUID16_DECLARE(SPP_DATA_RECEIVE),
                .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
                .val_handle = &spp_char_att_hdl,
                .access_cb  = device_write
            },
            {
                .uuid       = BLE_UUID16_DECLARE(SPP_DATA_NOTIFY),
                .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &spp_char_att_hdl,
                .access_cb  = dump_cb
            },
            {
                .uuid       = BLE_UUID16_DECLARE(SPP_COMMAND_RECEIVE),
                .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
                .val_handle = &spp_char_att_hdl,
                .access_cb  = device_write
            },
            {
                .uuid       = BLE_UUID16_DECLARE(SPP_COMMAND_NOTIFY),
                .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &spp_char_att_hdl,
                .access_cb  = dump_cb
            },{0}}},
    {0}};

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP","BLE_GAP_EVENT_CONNECT %s", event->connect.status == 0? "OK":"Faild");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        conn_hdl = event->connect.conn_handle;
        break;    
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP","BLE_GAP_EVENT_DISCONNECT");
        ble_app_advertise();
        break;       
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP","BLE_GAP_EVENT_ADV_COMPLETE");
        ble_app_advertise();
        break;         
    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI("GAP","BLE_GAP_EVENT_SUBSCRIBE");
        if (event->subscribe.attr_handle == battery_char_att_hdl)
        {
            if (event->subscribe.cur_notify == 1)
            {
               xTimerStart(timer_handler, 0);
            }
            else
            {
                xTimerStop(timer_handler, 0);
            }

        }else if (event->subscribe.attr_handle == spp_char_att_hdl)
        {
            if (event->subscribe.cur_notify == 1)
            {
               xTimerStart(timer_handler_spp, 0);
            }
            else
            {
                xTimerStop(timer_handler_spp, 0);
            }
        }

        break;
    default: break;
    }
    return 0;
}


void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_DISC_LTD | BLE_HS_ADV_F_BREDR_UNSUP; //GEN - generally discovered | LTD- limited disc
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *) ble_svc_gap_device_name();
    fields.name_len = strlen(ble_svc_gap_device_name());
    fields.name_is_complete = 1;
    
    fields.adv_itvl_is_present = 1;
    fields.adv_itvl = (uint16_t) 0xff40; //5 sec

    // uint8_t manufact_data = 24;
    // uint8_t * mfg_data_ptr = &manufact_data;
    // fields.mfg_data = mfg_data_ptr;
    // fields.mfg_data_len = sizeof(mfg_data_ptr);
    
    int rc;
        rc = ble_gap_adv_set_fields(&fields);
    printf("ERROR CHECK IS %d\n", rc);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode =  BLE_GAP_CONN_MODE_UND; //unidirected-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

void host_task(void *param)
{
    nimble_port_run();
}

void app_main(void)
{
    nvs_flash_init();
    esp_nimble_hci_and_controller_init();
    nimble_port_init();

    ESP_ERROR_CHECK(ble_svc_gap_device_name_set(DEVICE_NAME));
    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_gatts_count_cfg(gat_svcs);
    ble_gatts_add_svcs(gat_svcs);

    timer_handler = xTimerCreate("update_batter_status", pdMS_TO_TICKS(1000), pdTRUE, NULL, update_batter_status);
    timer_handler_spp = xTimerCreate("send_spp_data", pdMS_TO_TICKS(1000), pdTRUE, NULL, send_spp_data);
    
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}
