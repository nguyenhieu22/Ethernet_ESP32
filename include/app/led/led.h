#ifndef _LED_H_
#define _LED_H_
#include <stdint.h>

void led_init(void);
void led_strip_set(uint8_t red, uint8_t green, uint8_t blue);

#endif