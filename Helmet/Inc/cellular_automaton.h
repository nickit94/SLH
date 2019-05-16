#ifndef CELLULAR_AUTOMATION_H_
#define CELLULAR_AUTOMATION_H_

#include "main.h"

#define COUNT_CELL 13
#define MAX_WOLFRAM_RULE 29
#define MAX_WOLFRAM_RULE_SYMMETRIC 21

typedef enum
{
	start_state_ca_random,
	start_state_ca_center
} start_state_ca_t;

typedef struct
{
	/* Служебное */
	type_object_t type;					// Тип объекта
	uint8_t is_full;					// Флаг полной заполненности клеточным автоматом
	rgb_t current_color;				// Текущий цвет в режииме рандома для нового состояния клеточного автомата
	uint8_t current_rule;				// Текущее правило для клеточного автомата по коду Вольфрама
	uint8_t array_cells[COUNT_CELL];	// Массив под клеточный автомат

	/* Для пользователя */
	shift_direction_t direction;
	animation_t animation;
} celluar_automaton_t;

void ca_random_init();
void ca_animating(celluar_automaton_t* ca);
void ca_create(celluar_automaton_t* ca, start_state_ca_t start_state_ca, shift_direction_t direction,
		palette_t palette, uint8_t multiplicity, uint16_t time_change_one_color, uint16_t time_one_rand_step);

#endif /* CELLULAR_AUTOMATION_H_ */



