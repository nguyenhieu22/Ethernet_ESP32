#include "protocol/mqtt/subcriber.h"

#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"

#include "app/led/led.h"

const char *TAG = "Subscriber";

extern esp_mqtt_client_handle_t _client;

void subscribe(void) {
    //char *topic = util_get_topic_with_mac_ntf("tele/tasmota_%s/request");
    char topic[10] = "tele/test";
    printf("subscribe to topic %s\n", topic);
    esp_mqtt_client_subscribe(_client, topic, 0);
    //free(topic);
}

void consumer(char * data, int length) {
    cJSON *root = cJSON_Parse(data);
    cJSON *red = cJSON_GetObjectItem(root, "red");
    if (red == NULL || !cJSON_IsNumber(red)) {
        ESP_LOGE(TAG, "Command not found");
        cJSON_Delete(root);
        return;
    }
    cJSON *green = cJSON_GetObjectItem(root, "green");
    if (green == NULL || !cJSON_IsNumber(green)) {
        ESP_LOGE(TAG, "Command not found");
        cJSON_Delete(root);
        return;
    }
    cJSON *blue = cJSON_GetObjectItem(root, "blue");
    if (blue == NULL || !cJSON_IsNumber(blue)) {    
        ESP_LOGE(TAG, "Command not found");
        cJSON_Delete(root);
        return;
    }
    ESP_LOGI(TAG, "Command found: red=%d, green=%d, blue=%d", red->valueint, green->valueint, blue->valueint);
    // Call led_strip_set with the values from the JSON object
    led_strip_set(red->valueint, green->valueint, blue->valueint);
    cJSON_Delete(root);
    ESP_LOGI(TAG, "LED color set to: red=%d, green=%d, blue=%d", red->valueint, green->valueint, blue->valueint);
}