
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "gpio/button.h"

static QueueHandle_t gpio_evt_queue = NULL;
static button_func_pair_t *button_func_pair;
static esp_button_callback_t func_ptr;
static uint8_t button_num;

static const char *TAG = "BUTTON";

static void IRAM_ATTR gpio_isr_handler(void *arg) {
    xQueueSendFromISR(gpio_evt_queue, (button_func_pair_t *)arg, NULL);
}

static void button_driver_gpios_intr_enabled(bool enabled) {
    for (int i = 0; i < button_num; ++i) {
        if (enabled) {
            gpio_intr_enable((button_func_pair + i)->pin);
        } else {
            gpio_intr_disable((button_func_pair + i)->pin);
        }
    }
}


static void button_driver_button_detected(void *arg) {
    gpio_num_t io_num = GPIO_NUM_NC;
    button_func_pair_t button_func_pair;
    static button_state_t button_state = BUTTON_IDLE;
    bool evt_flag = false;

    for (;;) {
        /* check if there is any queue received, if yes read out the button_func_pair */
        if (xQueueReceive(gpio_evt_queue, &button_func_pair, portMAX_DELAY)) {
            io_num =  button_func_pair.pin;
            button_driver_gpios_intr_enabled(false);
            evt_flag = true;
        }
        while (evt_flag) {
            bool value = gpio_get_level(io_num);
            switch (button_state) {
            case BUTTON_IDLE:
                button_state = (value == GPIO_INPUT_LEVEL_ON) ? BUTTON_PRESS_DETECTED : BUTTON_IDLE;
                break;
            case BUTTON_PRESS_DETECTED:
                button_state = (value == GPIO_INPUT_LEVEL_ON) ? BUTTON_PRESS_DETECTED : BUTTON_RELEASE_DETECTED;
                break;
            case BUTTON_RELEASE_DETECTED:
                button_state = BUTTON_IDLE;
                /* callback to button_handler */
                (*func_ptr)(&button_func_pair);
                break;
            default:
                break;
            }
            if (button_state == BUTTON_IDLE) {
                button_driver_gpios_intr_enabled(true);
                evt_flag = false;
                break;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

static bool button_driver_gpio_init(button_func_pair_t *button_func_pair, uint8_t button_num) {
    gpio_config_t io_conf = {};
    button_func_pair = button_func_pair;
    button_num = button_num;
    uint64_t pin_bit_mask = 0;

    // set up button func pair pin mask
    for (int i = 0; i < button_num; ++i) {
        pin_bit_mask |= (1ULL << (button_func_pair + i)->pin);
    }
    // interrupt of falling edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = pin_bit_mask;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(button_func_pair_t));
    if ( gpio_evt_queue == 0) {
        ESP_LOGE(TAG, "Queue was not created and must not be used");
        return false;
    }
    // start gpio task 
    xTaskCreate(button_driver_button_detected, "button_detected", 16384, NULL, 10, NULL);
    // install gpio isr service 
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    for (int i = 0; i < button_num; ++i) {
        gpio_isr_handler_add((button_func_pair + i)->pin, gpio_isr_handler, (void *) (button_func_pair + i));
    }
    return true;
}

bool button_driver_init(button_func_pair_t *button_func_pair, uint8_t button_num, esp_button_callback_t cb)
{
    if (!button_driver_gpio_init(button_func_pair, button_num)) {
        printf("Failed to initialize button driver\n");
        return false;
    }
    func_ptr = cb;
    return true;
}