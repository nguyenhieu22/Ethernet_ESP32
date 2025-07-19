#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <stdio.h>
#include <stdint.h>
#include "driver/gpio.h"

#define GPIO_INPUT_IO_TOGGLE_BUTTON  GPIO_NUM_9

#define GPIO_INPUT_LEVEL_ON     0
#define ESP_INTR_FLAG_DEFAULT   0

#define PAIR_SIZE(TYPE_STR_PAIR) (sizeof(TYPE_STR_PAIR) / sizeof(TYPE_STR_PAIR[0]))
typedef enum {
    BUTTON_IDLE,
    BUTTON_PRESS_ARMED,
    BUTTON_PRESS_DETECTED,
    BUTTON_PRESSED,
    BUTTON_RELEASE_DETECTED,
} button_state_t;

typedef enum {
    BUTTON_ON_CONTROL,
    BUTTON_OFF_CONTROL,
    BUTTON_ONOFF_TOGGLE_CONTROL,
    BUTTON_LEVEL_UP_CONTROL,
    BUTTON_LEVEL_DOWN_CONTROL,
    BUTTON_LEVEL_CYCLE_CONTROL,
    BUTTON_COLOR_CONTROL,
} button_func_t;

typedef struct {
    uint32_t pin;
    button_func_t func;
} button_func_pair_t;

typedef void (*esp_button_callback_t)(button_func_pair_t *param);

bool button_driver_init(button_func_pair_t *button_func_pair, uint8_t button_num, esp_button_callback_t cb);

#endif