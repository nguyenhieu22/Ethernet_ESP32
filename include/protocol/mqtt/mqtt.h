#ifndef _MQTT_H_
#define _MQTT_H_

#include "mqtt_client.h"

#define MQTT_HOSTNAME "mqtt.xsolar.energy"
#define MQTT_PORT 1883
#define MQTT_TRANSPORT 1 //TCP
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "xSoLar@123234@sb"

void mqtt_app_start(void);

#endif