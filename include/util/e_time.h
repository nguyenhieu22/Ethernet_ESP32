#ifndef _E_TIME_H_
#define _E_TIME_H_
#include "time.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"

void ntp_sync(void * arg);
void get_current_time(char *buffer);

#endif