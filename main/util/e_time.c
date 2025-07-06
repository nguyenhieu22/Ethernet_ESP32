#include "util/e_time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// Function to synchronize time with NTP server
void ntp_sync(void *arg) {
    setenv("TZ", "GMT-7", 1);
    for (;;) {
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("3.asia.pool.ntp.org");
        esp_netif_sntp_init(&config);
        esp_err_t error = ESP_FAIL;
        error = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000));
        char buffer[20];
        get_current_time(buffer);
        if (error != ESP_OK) {
            printf("Failed to update system time within 10s timeout error %d\n", error);
            esp_netif_sntp_deinit();
            vTaskDelay(pdMS_TO_TICKS(20000));
            continue;
        }else {
            printf("Time updated\n");
            printf("Current time: %s\n", buffer);
        }
        esp_netif_sntp_deinit();
        vTaskDelay(pdMS_TO_TICKS(60*60*1000));
    }
}

void get_current_time(char *buffer) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;  
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2024 - 1900)) {
        memset(buffer, 0, 20);
        return;
    }
    strftime(buffer, 20, "%Y-%m-%dT%H:%M:%S", &timeinfo);
    
    printf("Formatted time: %s\n", buffer);
}
