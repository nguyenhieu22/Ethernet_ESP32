
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "common/ethernet_init.h"
#include "util/e_time.h"
#include "common/define.h"
#include "protocol/mqtt/mqtt.h"
#include "app/led/led.h"


EventGroupHandle_t s_eth_event_group;

static const char *TAG = "MAIN";

void app_main(void)
{   
    s_eth_event_group = xEventGroupCreate();

    printf("<<<<<  Xsolar begin >>>>>\n");
    nvs_flash_init();
    ethernet_init();

    // Wait for Ethernet to be connected or fail
    EventBits_t bits = xEventGroupWaitBits(s_eth_event_group,
            ETH_CONNECTED_BIT | ETH_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    if (bits & ETH_CONNECTED_BIT) {
        printf("Ethernet connected successfully\n");
    } else if (bits & ETH_FAIL_BIT) {
        printf("Ethernet connection failed\n");
    }
    
    mqtt_app_start();
    vTaskDelay(pdMS_TO_TICKS(5*1000)); //5s
   
    xTaskCreate(ntp_sync, "ntp_sync", 2048, NULL, 1, NULL);

    led_init();
    led_strip_set(0, 0, 0); // Set LED to green color
    
}
