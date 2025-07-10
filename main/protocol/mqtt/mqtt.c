#include "protocol/mqtt/mqtt.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "protocol/mqtt/subcriber.h"
#include "protocol/mqtt/publisher.h"

static const char *TAG = "mqtt";

esp_mqtt_client_handle_t _client = NULL;

//esp_mqtt_event_id_t _status = MQTT_EVENT_ANY;
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    //_status = event_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            //TODO publish esp start to topic "esp/start"
            //esp_mqtt_client_publish(_client, "esp/start", "ESP32 started", 0, 1, 0);
            mqtt_publish("tele/test", "esp_started", 0, 0);
            subscribe();
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            
            
            xsolar_buf_t topic = {
                .data = event->topic,
                .len = event->topic_len
            };
            xsolar_buf_t data = {
                .data = event->data,
                .len = event->data_len
            };
            consumer(&topic, &data);
            
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_app_start(void) {
    
    nvs_handle_t nvs;   
    nvs_open("storage", NVS_READONLY, &nvs);
    char hostname[50];
    int32_t port;
    char username[50];
    char password[50];
    size_t hostname_len = sizeof(hostname);
    size_t username_len = sizeof(username);
    size_t password_len = sizeof(password);
    esp_err_t hostname_err = nvs_get_str(nvs, "mqtt_host", hostname, &hostname_len);
    nvs_get_i32(nvs, "mqtt_port", &port);
    nvs_get_str(nvs, "mqtt_user", username, &username_len);
    nvs_get_str(nvs, "mqtt_password", password, &password_len);
    nvs_close(nvs);
    if(hostname_err == ESP_OK) {
        esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.hostname = hostname,
            .broker.address.port = port,
            .broker.address.transport = MQTT_TRANSPORT,
            .credentials.username = username,
            .credentials.authentication.password = password
        };
        ESP_LOGI(TAG, "connect to hostname: %s, port: %ld, username: %s, password: %s", hostname, port, username, password);
        _client = esp_mqtt_client_init(&mqtt_cfg);
    }else {
        esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.hostname = MQTT_HOSTNAME,
            .broker.address.port = MQTT_PORT,
            .broker.address.transport = MQTT_TRANSPORT,
            .credentials.username = MQTT_USERNAME,
            .credentials.authentication.password = MQTT_PASSWORD
        };
        _client = esp_mqtt_client_init(&mqtt_cfg);
    }
    
    esp_mqtt_client_register_event(_client, ESP_EVENT_ANY_ID, mqtt_event_handler, _client);
    esp_mqtt_client_start(_client);
}