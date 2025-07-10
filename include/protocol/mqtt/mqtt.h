#ifndef _MQTT_H_
#define _MQTT_H_

#include "mqtt_client.h"
#include "common/define.h"

#define MQTT_HOSTNAME "mqtt.xsolar.energy"
#define MQTT_PORT 1883
#define MQTT_TRANSPORT 1 //TCP
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "xSoLar@123234@sb"

#define MAX_TOPIC 2
// Function Pointer
typedef void (*mqtt_callback_t) (xsolar_buf_t *data);
//
typedef struct topic_pair {
    char * topic;
    mqtt_callback_t callback;
} topic_pair_t;

void mqtt_app_start(void);

#endif