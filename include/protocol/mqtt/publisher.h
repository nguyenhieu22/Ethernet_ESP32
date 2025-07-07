#ifndef _PUBLISHER_H_
#define _PUBLISHER_H_

void mqtt_publish(const char *topic, const char *message, int qos, int retain);

#endif