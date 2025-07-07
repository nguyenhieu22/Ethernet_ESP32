#include "app/led/led.h"
#include <led_strip.h>
#include <esp_err.h>

static led_strip_handle_t led_strip;

void led_init(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = 8,
        .max_leds       = 1,    // Số lượng LED
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000 // Tần số tín hiệu 10MHz
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

void led_strip_set(uint8_t red, uint8_t green, uint8_t blue) {
    led_strip_set_pixel(led_strip, 0, red, green, blue);
    led_strip_refresh(led_strip);
}