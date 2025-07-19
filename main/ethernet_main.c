
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "common/ethernet_init.h"
#include "util/e_time.h"
#include "common/define.h"
#include "protocol/mqtt/mqtt.h"
#include "protocol/mqtt/subcriber.h"
#include "protocol/mqtt/publisher.h"
#include "app/led/led.h"
#include "ble/ble_mesh.h"
#include "cJSON.h"
#include "gpio/button.h"

EventGroupHandle_t s_eth_event_group, s_ble_mesh_event_group;

static const char *TAG = "MAIN";

#define   X_LED_MODEL   ESP_BLE_MESH_MODEL_OP_2(0xA0, 0x01)

#define   BUTTON_OPCODE ESP_BLE_MESH_MODEL_OP_2(0xA1, 0x79)

static esp_ble_mesh_client_t xsolar_device_client;
static esp_ble_mesh_client_t xsolar_button_client;

static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_ENABLED,             //ESP_BLE_MESH_RELAY_DISABLED,
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,

#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
    .default_ttl = 2,
};

// Khoi tao opcode cho model.
static esp_ble_mesh_model_op_t x_op_led[] = {           // [1] = {ma lenh model, do dai min message, callbak xu ly}
    {X_LED_MODEL, 1, 0},
    ESP_BLE_MESH_MODEL_OP_END                           // Kết thúc danh sách lenh.
};

static esp_ble_mesh_model_op_t xsolar_button_op[] = {
    {BUTTON_OPCODE, 1, 0},
    ESP_BLE_MESH_MODEL_OP_END
};

static esp_ble_mesh_client_t config_client;

// Khoi tao model cho client.
static esp_ble_mesh_model_t x_model_root[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),   
    BLE_MESH_XSOLAR_MODEL(&xsolar_device_client, x_op_led),
};

static esp_ble_mesh_model_t xsolar_button_model[] = {
    BLE_MESH_XSOLAR_MODEL(&xsolar_button_client, xsolar_button_op),
};

// Khoi tao cac phan tu (elements) trong mesh.
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, x_model_root       , ESP_BLE_MESH_MODEL_NONE),
    ESP_BLE_MESH_ELEMENT(1, xsolar_button_model, ESP_BLE_MESH_MODEL_NONE),
};

void ble_callback(esp_ble_mesh_model_cb_event_t event,
                    esp_ble_mesh_model_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_MODEL_SEND_COMP_EVT) return;
    ESP_LOGI(TAG, "Custom model callback with event %d, opcode 0x%04x & addr 0x%04x", event, (unsigned int)param->model_operation.opcode, param->model_operation.ctx->addr);
    ESP_LOGI(TAG, "length %d", param->model_operation.length);
    ESP_LOGI(TAG, "data: ");
    for (int i = 0; i < param->model_operation.length; i++)
    {
        printf("%02x", param->model_operation.msg[i]);
    }
    printf("\n");
    //publish_request(param->model_operation.opcode, param->model_operation.msg, param->model_operation.length);
}

void led_control(uint16_t addr, uint16_t opcode, uint8_t *data, uint16_t len) {
    esp_err_t err = xsolar_ble_mesh_send_data(&x_model_root[2], addr, opcode, data, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send data");
    }
}

void xsolar_callback(xsolar_buf_t *data) {
    cJSON *root = cJSON_Parse(data->data);
    cJSON *addr_json = cJSON_GetObjectItem(root, "address");
    if (addr_json == NULL || addr_json->type != cJSON_Number) {
        ESP_LOGE(TAG, "Invalid or missing 'address' field in JSON");
        cJSON_Delete(root);
        return;
    }

    cJSON *opcode_json = cJSON_GetObjectItem(root, "opcode");
    if (opcode_json == NULL || opcode_json->type != cJSON_Number) {
        ESP_LOGE(TAG, "Invalid or missing 'opcode' field in JSON");
        cJSON_Delete(root);
        return;
    }

    cJSON *red = cJSON_GetObjectItem(root, "red");
    if (red == NULL || red->type != cJSON_Number) {
        ESP_LOGE(TAG, "Invalid or missing 'red' field in JSON");
        cJSON_Delete(root);
        return;
    }

    if (red->valueint < 0 || red->valueint > 255) {
        ESP_LOGE(TAG, "'red' value out of range (0-255)");
        cJSON_Delete(root);
        return;
    }
    cJSON *green = cJSON_GetObjectItem(root, "green");
    if (green == NULL || green->type != cJSON_Number) {
        ESP_LOGE(TAG, "Invalid or missing 'green' field in JSON");
        cJSON_Delete(root);
        return;
    }
    if (green->valueint < 0 || green->valueint > 255) {
        ESP_LOGE(TAG, "'green' value out of range (0-255)");
        cJSON_Delete(root);
        return;
    }
    cJSON *blue = cJSON_GetObjectItem(root, "blue");
    if (blue == NULL || blue->type != cJSON_Number) {
        ESP_LOGE(TAG, "Invalid or missing 'blue' field in JSON");
        cJSON_Delete(root);
        return;
    }
    if (blue->valueint < 0 || blue->valueint > 255) {
        ESP_LOGE(TAG, "'blue' value out of range (0-255)");
        cJSON_Delete(root);
        return;
    }

    uint16_t addr = (uint16_t)addr_json->valueint;
    uint16_t opcode = (uint16_t)opcode_json->valueint;
    uint8_t data_to_send[3] = {red->valueint, green->valueint, blue->valueint};
    led_strip_set(data_to_send[0], data_to_send[1], data_to_send[2]);
    xsolar_ble_mesh_send_data(&x_model_root[2], addr, opcode, data_to_send, sizeof(data_to_send));

    cJSON_Delete(root);
}

void xsolar_callback_(xsolar_buf_t *data) {
    printf("callback status\n");

}

//Button.
button_func_pair_t button_func_pair[] = {
        {GPIO_NUM_9, BUTTON_ONOFF_TOGGLE_CONTROL}
};

void button_callback(button_func_pair_t *param) {
    printf("Button pressed %d\n", param->func);
    int len = 6;
    uint8_t data[len];
    switch (param->func) {
        case BUTTON_ONOFF_TOGGLE_CONTROL:
            ESP_LOGI(TAG, "Button pressed");
           
            for (int i = 0; i < len; i++) {
                data[i] = i;
            }

            esp_err_t err = xsolar_ble_mesh_send_data(&x_model_root[2], 0x006f, X_LED_MODEL, data, len);

            if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send data");
            } else {
                ESP_LOGI(TAG, "Sent data to device");
            }
            break;
        default:
            break;
    }
}

void app_main(void)
{   
    s_eth_event_group = xEventGroupCreate();
    //s_ble_mesh_event_group = xEventGroupCreate();

    printf("<<<<<  Xsolar begin >>>>>\n");
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ble_mesh_init(elements, ARRAY_SIZE(elements));
    esp_ble_mesh_client_model_init(&x_model_root[2]);
    esp_ble_mesh_client_model_init(&xsolar_button_model[0]);
    xsolar_ble_mesh_register_model_callback(ble_callback);

    /*EventBits_t bits_s = xEventGroupWaitBits(s_ble_mesh_event_group,
            BLE_MESH_INIT_BIT | BLE_MESH_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);*/

    ethernet_init();

    // Wait for Ethernet to be connected or fail
    EventBits_t bits = xEventGroupWaitBits(s_eth_event_group,
            ETH_CONNECTED_BIT | ETH_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    if (bits & ETH_CONNECTED_BIT) {
        printf("Ethernet connected successfully\n");
    } else if (bits & ETH_FAIL_BIT) {
        printf("Ethernet connection failed\n");
    }

    char *topic= "stat/led_rgb/CONTROL"; //poiter chi den chui hang
    char *topic_= "stat/led_rgb/STATUS";
    xsolar_buf_t topic_buf = {
        .data = topic,
        .len = strlen(topic)
    };
    xsolar_buf_t topic_buf_ = {
        .data = topic_,
        .len = strlen(topic_)
    };

    subscribe_register(&topic_buf, xsolar_callback); 
    subscribe_register(&topic_buf_, xsolar_callback_); 
    mqtt_app_start();
    vTaskDelay(pdMS_TO_TICKS(5*1000)); //5s
   
    xTaskCreate(ntp_sync, "ntp_sync", 2048, NULL, 1, NULL);
    
    led_init();
    led_strip_set(0, 0, 0); // Set LED to green color
    button_driver_init(button_func_pair, PAIR_SIZE(button_func_pair), button_callback);
    /*while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // Delay 1s
        printf("Sending LED control command...\n");
        uint8_t data[3] = {0x00, 0xFF, 0x00}; // Green color data
        led_control(0x006f, X_LED_MODEL, data, 3);
    }*/
    
}
