#ifndef LED_BUFFER_H_
#define LED_BUFFER_H_

#include <main.h>

#define MAX_X         13
#define MAX_Y         40
#define MAX_X_VIRTUAL 19
#define MAX_Y_VIRTUAL 22

extern rgb_t led_buffer[MAX_X][MAX_Y];
extern const uint8_t mas_count_leds_in_channel[];

void led_buffer_init();
void led_buffer_prepare_for_apa106(uint8_t led_channel);
void led_buffer_change_color(rgb_t new_color);
void led_buffer_shift(shift_direction_t direction, shift_mode_t mode, uint8_t num_shift);
void led_buffer_clear(uint8_t x);
void led_buffer_clear_all();

#endif /* LED_BUFFER_H_ */
