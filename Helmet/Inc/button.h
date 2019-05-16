#ifndef BUTTON_H_
#define BUTTON_H_

#include "main.h"

#define TIME_DEBOUNCE 		30
#define TIME_COUNT_PRESS 	3000

extern button_t but1;

void button_init();
void button_fsm();

uint8_t button_check(button_t* button, uint16_t time_press);
uint16_t button_get_time_pressed(button_t* button);
uint16_t button_get_count_press(button_t* button);
uint8_t button_get_state(button_t* button);


#endif /* BUTTON_H_ */
