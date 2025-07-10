#ifndef _BLE_MESH_H_
#define _BLE_MESH_H_

#include <stdio.h>
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"

#ifdef CONFIG_BT_BLUEDROID_ENABLED
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#endif

#define X_MODEL_ID 0x1234

#define BLE_MESH_XSOLAR_MODEL(client, opcode) {     \
    .model_id = X_MODEL_ID,                         \
    .pub = NULL,                                    \
    .user_data = client,                            \
    .op = opcode,                                   \
    .keys = ESP_BLE_MESH_MODEL_KEYS_UNUSED,         \
    .groups = ESP_BLE_MESH_MODEL_GROUPS_UNASSIGNED, \
}

//#ifdef CONFIG_BT_ENABLED
esp_err_t ble_mesh_init(esp_ble_mesh_elem_t *element, uint16_t element_count);
//#endif

//Ham xu ly callback cho client.
void xsolar_ble_mesh_register_model_callback(esp_ble_mesh_model_cb_t callback);
void xsolar_ble_mesh_register_generic_callback(esp_ble_mesh_cfg_client_cb_t callback);
esp_err_t xsolar_ble_mesh_send_data(esp_ble_mesh_model_t *model, uint16_t unicast, uint32_t opcode, uint8_t *data, uint16_t len);
esp_err_t xsolar_ble_mesh_group_get(esp_ble_mesh_model_t *model, uint16_t address, uint16_t element);

#endif