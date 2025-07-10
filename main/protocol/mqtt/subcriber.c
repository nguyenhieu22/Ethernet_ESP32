#include <string.h>
#include "protocol/mqtt/subcriber.h"

#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "protocol/mqtt/mqtt.h"

#include "app/led/led.h"

const char *TAG = "Subscriber";


extern esp_mqtt_client_handle_t _client;

static topic_pair_t pairs[MAX_TOPIC];

void consumer(xsolar_buf_t *topic, xsolar_buf_t *data) {
    for (int i = 0; i < MAX_TOPIC; i++) {
        if ((pairs[i].topic != NULL) && (memcmp(pairs[i].topic, topic->data, topic->len) == 0)) {
            pairs[i].callback(data);
            break;
        }
    }
}

void subscribe_register(xsolar_buf_t *topic, mqtt_callback_t callback) {
    for (int i = 0; i < MAX_TOPIC; i++) {
        if (pairs[i].topic == NULL) {
            pairs[i].topic = topic->data;
            pairs[i].callback = callback;
            break;
        }
    }
}

void subscribe(void) {
    for (int i = 0; i < MAX_TOPIC; i++) {
        if (pairs[i].topic != NULL) {
            printf("Subscribing to %s\n", pairs[i].topic);
            esp_mqtt_client_subscribe(_client, pairs[i].topic, 0);
        }
    }
}