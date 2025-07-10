#include "ble/ble_mesh.h"

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
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "common/define.h"
#define CID_ESP 0x02E5    

extern EventGroupHandle_t s_ble_mesh_event_group;

static const char *TAG = "BLE_MESH";

static uint8_t dev_uuid[16] = { 0xdd, 0xdd };
static esp_ble_mesh_comp_t composition = {};
static esp_ble_mesh_prov_t provision   = {};

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    if (dev_uuid == NULL) {
        ESP_LOGE(TAG, "%s, Invalid device uuid", __func__);
        return;
    }
    memcpy(dev_uuid + 2, esp_bt_dev_get_address(), BD_ADDR_LEN);
}

esp_err_t bluetooth_init(void)
{
    esp_err_t ret;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed", __func__);
        return ret;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed", __func__);
        return ret;
    }
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// CALLBACK

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index) {
    ESP_LOGI(TAG, "net_idx: 0x%04x, addr: 0x%04x", net_idx, addr);
    ESP_LOGI(TAG, "flags: 0x%02x, iv_index: 0x%08" PRIx32, flags, iv_index);
}

static void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                    esp_ble_mesh_prov_cb_param_t *param) {
    switch (event) {
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
            param->node_prov_complete.flags, param->node_prov_complete.iv_index);
        /*esp_err_t err = esp_ble_mesh_model_subscribe_group_addr(param->node_prov_complete.addr, BLE_MESH_CID_NVAL, X_MODEL_ID, 0xC001);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to subscribe to group address");
        }
        err = esp_ble_mesh_model_subscribe_group_addr(param->node_prov_complete.addr, BLE_MESH_CID_NVAL, X_MODEL_ID, 0xC002);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to subscribe to group address");
        }*/
        break;
    default:
        break;
    }
}

//config server

static void ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                        esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
            case    ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                    ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
                    ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x",
                        param->value.state_change.appkey_add.net_idx,
                        param->value.state_change.appkey_add.app_idx);
                    ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
                    //xEventGroupSetBits(s_ble_mesh_event_group, ETH_CONNECTED_BIT);
                    break;
            case    ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                    ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
                    ESP_LOGI(TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                        param->value.state_change.mod_app_bind.element_addr,
                        param->value.state_change.mod_app_bind.app_idx,
                        param->value.state_change.mod_app_bind.company_id,
                        param->value.state_change.mod_app_bind.model_id);
                        
                    break;
            case    ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
                    ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
                    ESP_LOGI(TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                        param->value.state_change.mod_sub_add.element_addr,
                        param->value.state_change.mod_sub_add.sub_addr,
                        param->value.state_change.mod_sub_add.company_id,
                        param->value.state_change.mod_sub_add.model_id);
                    break;
            default:
                    break;
        }
    }
}

//gui data den node
esp_err_t xsolar_ble_mesh_send_data(esp_ble_mesh_model_t *model, uint16_t unicast, uint32_t opcode, uint8_t *data, uint16_t len) {
    esp_err_t err;
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = 0,
        .app_idx = 0,
        .send_ttl = 3,
        .addr = unicast
    };
    err = esp_ble_mesh_client_model_send_msg(
        model,
        &ctx,
        opcode,
        len,
        data,
        0,
        0,
        ROLE_NODE
    );
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send data to device: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Sent data to device");
    }
    return err;
}

// Dang ky ham callback cho customer

void xsolar_ble_mesh_register_model_callback(esp_ble_mesh_model_cb_t callback) {
    esp_ble_mesh_register_custom_model_callback(callback);    //nhận và xử lý các message đến từ các model mà bạn tự định nghĩa
}

void xsolar_ble_mesh_register_generic_callback(esp_ble_mesh_cfg_client_cb_t callback) {
    esp_ble_mesh_register_config_client_callback(callback);  //xử lý sự kiện cho Configuration Client Model
}

//////////////////////////////////////////////////////////////////////////////////////////////
esp_err_t ble_mesh_init(esp_ble_mesh_elem_t *element, uint16_t element_count) {
    // Innitialize the bluetooth.
    esp_err_t err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
        return err;
    }
    
    ble_mesh_get_dev_uuid(dev_uuid);
    provision.uuid = dev_uuid;
    composition.elements = element;
    composition.element_count = element_count;
    composition.cid = CID_ESP;
    err = esp_ble_mesh_init(&provision, &composition);

    esp_ble_mesh_register_prov_callback(ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(ble_mesh_config_server_cb);

    if (!esp_ble_mesh_node_is_provisioned()) {
        err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to enable mesh node");
            return err;
        }
    }
    return ESP_OK;
}