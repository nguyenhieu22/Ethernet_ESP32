#include "protocol/mqtt/subcriber.h"

#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "Subscriber";
extern esp_mqtt_client_handle_t _client;

void mqtt_publish(const char *topic, const char *message, int qos, int retain) {
    if (_client == NULL) {
        ESP_LOGE(TAG, "MQTT client is not initialized");
        return;
    }
    ESP_LOGI(TAG, "Free heap size: %ld", esp_get_free_heap_size());
    // Publish the message to the specified topic
    int msg_id = esp_mqtt_client_publish(_client, topic, message, 0, qos, retain);

    if (msg_id >= 0) {
        ESP_LOGI(TAG, "Message published, msg_id=%d", msg_id);
    } else {
        ESP_LOGE(TAG, "Failed to publish message");
    }
}