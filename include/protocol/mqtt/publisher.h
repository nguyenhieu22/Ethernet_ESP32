#ifndef _PUBLISHER_H_
#define _PUBLISHER_H_
#include "common/define.h"

void mqtt_publish(const char *topic, const char *message, int qos, int retain);

#endif