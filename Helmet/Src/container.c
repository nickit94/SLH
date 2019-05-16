#include "container.h"

/* Основная палитра */
#define MAIN_COLORS { COLOR_YELLOW, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_PINK, COLOR_PURPLE, COLOR_ORANGE, COLOR_CYAN, COLOR_LIGHT_GREEN, COLOR_WHITE }

/* ---------------------------  for all objects: ---------------------------  */

uint8_t num_step_accumulation(uint8_t num_led)
{
	uint8_t sum = 0;

	for (uint8_t i = 1; i < num_led; i++)
		sum += i;

	return num_led * num_led - sum;
}

void constr_init_animation(animation_t* _animation, palette_t __palette, uint8_t _multiplicity, uint16_t _time_change_one_color, uint16_t _time_one_rand_step)
{
	_animation->palette = __palette;
	_animation->time_change_one_color = _time_change_one_color;
	_animation->multiplicity = _multiplicity;
	_animation->time_one_ca_step = _time_one_rand_step;

	_animation->current_state = 0;
	_animation->current_step = 0;
	_animation->index_color = 0;
	_animation->index_transition = 0;
}

/* --------------------------- container ---------------------------  */

void* container[MAX_CONTAINER][MAX_OBJECT];
static uint8_t cur_container = 0;
static uint8_t array_container_last_obj[MAX_CONTAINER] = {0};
static uint8_t fl_start_update = 0;

palette_t palette_replacement = {palette_rule_replacement, MAIN_COLORS, 10};
palette_t palette_accumulation = {palette_rule_accumulation, MAIN_COLORS, 10};
palette_t palette_gradient = {palette_rule_gradient, MAIN_COLORS, 10};
palette_t palette_rainbow = {palette_rule_rainbow, {}, 0};
palette_t palette_at_once = {palette_rule_at_once, MAIN_COLORS, 10};


/* private container: */
void container_start_update()
{
	fl_start_update = 1;
}

type_object_t get_type_object(void* object)
{
	return *((uint8_t*) object);
}

uint8_t container_check_null()
{
	for (uint8_t i = 0; i < MAX_OBJECT; i++)
	{
		if (get_type_object(container[cur_container][i]) != type_object_none) return 0;
	}

	return 1;
}

uint8_t container_add(uint8_t num_container, void* object)
{
	if (num_container >= MAX_CONTAINER) return 1;
	if (array_container_last_obj[num_container] >= MAX_OBJECT) return 1;

	uint8_t index = array_container_last_obj[num_container];
	container[num_container][index] = object;
	array_container_last_obj[num_container]++;

	return 0;
}

/* create animate: */
void container_set_anim_1()
{
	static rectangle_t rect1;
	rect_create(&rect1, 6, 6, 7, palette_rainbow, 0, 1000);
	container_add(0, (void*)&rect1);
}
void container_set_anim_2()
{
	static line_t line1, line2, line3, line4, line5, line6;
	static rectangle_t rect1, rect2;

	line_create(&line1, 0,  13, 16, palette_rainbow, 0, 3000);
	line_create(&line2, 12, 13, 15, palette_rainbow, 0, 3000);
	line_create(&line3, 1,  13, 20, palette_rainbow, 0, 4000);
	line_create(&line4, 11, 13, 20, palette_rainbow, 0, 4000);
	line_create(&line5, 2,  13, 27, palette_rainbow, 0, 5000);
	line_create(&line6, 10, 13, 27, palette_rainbow, 0, 5000);

	rect_create(&rect1, 0, 6, 7, palette_rainbow, 0, 800);
	rect_create(&rect2, 12, 6, 7, palette_rainbow, 0, 800);

	container_add(1, (void*)&rect1);
	container_add(1, (void*)&rect2);

	container_add(1, (void*)&line1);
	container_add(1, (void*)&line2);
	container_add(1, (void*)&line3);
	container_add(1, (void*)&line4);
	container_add(1, (void*)&line5);
	container_add(1, (void*)&line6);
}
void container_set_anim_3()
{
	static celluar_automaton_t ca;
	ca_create(&ca, start_state_ca_random, shift_direction_down, palette_rainbow, 0, 5000, 100);
	container_add(2, (void*)&ca);
}
void container_set_anim_4() {}
void container_set_anim_5() {}
void container_set_anim_6() {}
void container_set_anim_7() {}
void container_set_anim_8() {}

/* public: */
void container_next()
{
	led_buffer_clear_all();

	/* Увеличивать номер текущего контейнера, пока тот не перестанет быть пустым */
	do cur_container = (cur_container == (MAX_CONTAINER - 1)) ? 0 : (cur_container + 1);
	while (container_check_null());
}

void container_prev()
{
	led_buffer_clear_all();

	do cur_container = (cur_container == 0) ? (MAX_CONTAINER - 1) : (cur_container - 1);
	while (container_check_null());
}

void container_init()
{
	for (uint8_t i = 0; i < MAX_CONTAINER; i++)
		for (uint8_t j = 0; j < MAX_OBJECT; j++)
			container[i][j] = NULL;


	container_set_anim_1();
	container_set_anim_2();
	container_set_anim_3();
}

void container_run()
{
	for (uint8_t i = 0; i < MAX_OBJECT; i++)
	{
		switch(get_type_object(container[cur_container][i]))
		{
			case type_object_none:
				return;
			case type_object_line: line_animating((line_t*)container[cur_container][i]);
				break;
			case type_object_rect: rect_animating((rectangle_t*)container[cur_container][i]);
				break;
			case type_object_ca: ca_animating((celluar_automaton_t*)container[cur_container][i]);
				break;
		}
	}

	if (fl_start_update)
	{
		fl_start_update = 0;
		apa106_start_update();
	}
}
