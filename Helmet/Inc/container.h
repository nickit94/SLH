#ifndef CONSTRUCTOR_H_
#define CONSTRUCTOR_H_

#include "main.h"

#define MAX_OBJECT 13
#define MAX_CONTAINER 10




uint8_t num_step_accumulation(uint8_t num_led);
void constr_init_animation(animation_t* _animation, palette_t __palette, uint8_t _multiplicity, uint16_t _time_change_one_color, uint16_t _time_one_rand_step);

void container_start_update();
void container_next();
void container_prev();
void container_init();
void container_run();

#endif
