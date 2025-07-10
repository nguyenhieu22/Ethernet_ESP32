#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

#include "common/define.h"
#include "protocol/mqtt/mqtt.h"

void consumer(xsolar_buf_t *topic, xsolar_buf_t *data);
void subscribe_register(xsolar_buf_t *topic, mqtt_callback_t callback);
void subscribe(void);

#endif