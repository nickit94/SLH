#include "cellular_automaton.h"

#define ca_cur_step   ca->animation.current_step
#define ca_indx_color ca->animation.index_color
#define ca_multiple   ca->animation.multiplicity
#define ca_time_color ca->animation.time_change_one_color
#define ca_time_step  ca->animation.time_one_ca_step
#define ca_palette    ca->animation.palette
#define ca_indx_tran  ca->animation.index_transition
#define ca_next_color ca->animation.palette.array_colors[ca->animation.index_color + 1]
#define ca_cur_color  ca->animation.palette.array_colors[ca->animation.index_color + 0]
#define ca_prev_color ca->animation.palette.array_colors[ca->animation.index_color - 1]
#define ca_last_color ca->animation.palette.array_colors[ca->animation.palette.number_colors - 1]

const uint8_t array_wolfram_rule[MAX_WOLFRAM_RULE] =
{
	3, 9, 11, 13, 15, 25, 26, 28, 30, 41, 45, 60, 61, 62, 75, 89, 93, 99, 101, 105, 107, 111, 115, 135, 137, 149, 169, 181, 188
};

const uint8_t array_wolfram_rule_symmetric[MAX_WOLFRAM_RULE_SYMMETRIC] =
{
	5, 18, 22, 50, 54, 55, 73, 77, 91, 94, 105, 109, 114, 122, 123, 126, 147, 150, 163, 182, 201
};

/* private: */
/*--------------------------------- Celluar automation (ca) ----------------------------------------*/
inline int gen_ca_tor(int8_t x)
{
	return (x < 0) ? (x + COUNT_CELL) : (x % COUNT_CELL);
}
uint8_t gen_ca_wolfram_code(uint8_t rule, uint8_t left, uint8_t center, uint8_t right)
{
	uint8_t cur_state = ((left)   ? (1 << 2) : 0) |
						((center) ? (1 << 1) : 0) |
						((right)  ? (1 << 0) : 0);

	switch(cur_state)
	{
	case 0: return (rule >> 0) & 0x01; // b000
	case 1: return (rule >> 1) & 0x01; // b001
	case 2: return (rule >> 2) & 0x01; // b010
	case 3: return (rule >> 3) & 0x01; // b011
	case 4: return (rule >> 4) & 0x01; // b100
	case 5: return (rule >> 5) & 0x01; // b101
	case 6: return (rule >> 6) & 0x01; // b111
	case 7: return (rule >> 7) & 0x01; // b111
	}

	return 0;
}
void gen_ca_next_step(uint8_t rule, uint8_t* array_states)
{
	uint8_t array_next_states[COUNT_CELL];

	/* Получение следующего шага */
	for (uint8_t i = 0; i < COUNT_CELL; i++)
	{
		array_next_states[i] = gen_ca_wolfram_code(rule, array_states[gen_ca_tor(i - 1)], array_states[gen_ca_tor(i)], array_states[gen_ca_tor(i + 1)]);
	}

	/* Копирование */
	for (uint8_t i = 0; i < COUNT_CELL; i++)
	{
		array_states[i] = array_next_states[i];
	}
}
/*--------------------------------------------------------------------------------------------------*/

void ca_random_init()
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;   // Тактирование порта B

	GPIOB->CRL &= ~GPIO_CRL_MODE3;   // Очистить разряды MODE
	GPIOB->CRL &= ~GPIO_CRL_CNF3;    // Очистить разряды CNF
	GPIOB->CRL |=  GPIO_CRL_CNF3_0;  // Дискретный вход, Hi-Z
}

uint16_t ca_random_number()
{
	static uint32_t seed = 0;

	for (uint8_t i = 0; i < 32; i++)
	{
		if (GPIOB->IDR & GPIO_IDR_IDR3) seed |= (uint32_t)(1 << i);        //если на выводе "1"
		else 							seed &= ~((uint32_t)(1 << i));

		HAL_Delay(1);
	}

    for (uint8_t i = 0; i < ((HAL_GetTick() + 1) % 17); i++)
    	seed = (8253729 * seed + 2396403);

    return (uint16_t)(seed % 0xFFFF);
}
void ca_update_palette(celluar_automaton_t* ca)
{
	if (swTimerCheck(&ca->animation.timer_palette))
	{
		/* Смена цвета */
		switch(ca_palette.palette_rule)
		{
		case palette_rule_replacement:
			if (++ca_indx_tran == ca_multiple)
			{
				ca_indx_tran = 0;
				if (++ca_indx_color == ca_palette.number_colors) ca_indx_color = 0;
			}

			ca->current_color = ca_cur_color;
		break;

		case palette_rule_accumulation:
		break;

		case palette_rule_gradient:
			if (++ca_indx_tran == GRADIENT_STEPS)
			{
				ca_indx_tran = 0;
				if (++ca_indx_color == ca_palette.number_colors) ca_indx_color = 0;

				if (ca_indx_color != ca_palette.number_colors - 1)
					color_get_gradient(ca_cur_color, ca_next_color, ca->animation.array_gradient);
				else
					color_get_gradient(ca_cur_color, ca_palette.array_colors[0], ca->animation.array_gradient);
			}

			ca->current_color = ca->animation.array_gradient[ca_indx_tran];
			led_buffer_change_color(ca->current_color);
		break;

		case palette_rule_rainbow:
			ca->animation.hsv_rainbow.h = (ca->animation.hsv_rainbow.h + RAINBOW_STEP * 2) % 255;
			ca->current_color = hsv2rgb(ca->animation.hsv_rainbow);
		break;

		case palette_rule_at_once:
			if (++ca_indx_color == ca_palette.number_colors) ca_indx_color = 0;

			ca->current_color = ca_cur_color;
			led_buffer_change_color(ca->current_color);
		break;
		}
	}
}

/* public: */
void ca_create(celluar_automaton_t* ca, start_state_ca_t start_state_ca, shift_direction_t direction,
		palette_t palette, uint8_t multiplicity, uint16_t time_change_one_color, uint16_t time_one_rand_step)
{
	ca->direction = direction;
	ca->type = type_object_ca;
	constr_init_animation(&ca->animation, palette, multiplicity, time_change_one_color, time_one_rand_step);

	/* Start position */
	uint16_t rnd = ca_random_number();

	if (start_state_ca == start_state_ca_random)
	{
		ca->current_rule = array_wolfram_rule[ca_random_number() % MAX_WOLFRAM_RULE];
		for (uint8_t i = 0; i < COUNT_CELL; i++) ca->array_cells[i] = (rnd >> i) & 0x01;
	}
	if (start_state_ca == start_state_ca_center)
	{
		ca->current_rule = array_wolfram_rule_symmetric[ca_random_number() % MAX_WOLFRAM_RULE_SYMMETRIC];
		for (uint8_t i = 0; i < COUNT_CELL; i++) ca->array_cells[i] = (i == COUNT_CELL / 2) ? 1 : 0;
	}

}
void ca_animating(celluar_automaton_t* ca)
{
	switch(ca->animation.current_state)
	{
	/* Инициализация */
	case 0:
		ca->is_full = 0;
		ca->current_color = ca->animation.palette.array_colors[0];
		ca->animation.current_step = 0;
		ca->animation.index_color = 0;
		ca->animation.index_transition = 0;

		/* Direction */
		if (ca->direction == shift_direction_down)  ca->animation.current_step = 0;
		if (ca->direction == shift_direction_up) 	ca->animation.current_step = MAX_Y - 1;

		/* Rule color */
		if (ca_palette.palette_rule == palette_rule_gradient) 	color_get_gradient(ca_palette.array_colors[0], ca_palette.array_colors[1], ca->animation.array_gradient);
		if (ca_palette.palette_rule == palette_rule_rainbow)  	ca->animation.hsv_rainbow = rgb2hsv((rgb_t)COLOR_RED);

		/* Timers */
		if (ca_palette.palette_rule == palette_rule_replacement)	swTimerSet(&ca->animation.timer_palette, 0, ca_time_color / ca_multiple);
		if (ca_palette.palette_rule == palette_rule_gradient) 		swTimerSet(&ca->animation.timer_palette, 0, ca_time_color / GRADIENT_STEPS);
		if (ca_palette.palette_rule == palette_rule_rainbow) 		swTimerSet(&ca->animation.timer_palette, 0, ca_time_color / RAINBOW_DELAY);
		if (ca_palette.palette_rule == palette_rule_at_once) 		swTimerSet(&ca->animation.timer_palette, 0, ca_time_color);

		swTimerSet(&ca->animation.timer_random, 0, ca_time_step);
		ca->animation.current_state = 1;
	break;

	/* Заполнение */
	case 1:
		for (uint8_t i = 0; i < MAX_X; i++)
		{
			if (ca->direction == shift_direction_up && ca->array_cells[i])
				draw_pixel(i, (ca->is_full) ? (mas_count_leds_in_channel[i] - 1) : ca_cur_step, ca->current_color);

			if (ca->direction == shift_direction_down && ca->array_cells[i])
				draw_pixel(i, ca_cur_step, ca->current_color);
		}

		gen_ca_next_step(ca->current_rule, ca->array_cells);
		apa106_start_update();

		ca->animation.current_state = 2;
	break;

	/* Сдвиг */
	case 2:
		if (swTimerCheck(&ca->animation.timer_random))
		{
			/* Заполнение */
			if (!ca->is_full)									// Первая стадия- накопление
			{
				if (ca->direction == shift_direction_up)
				{
					if (--ca_cur_step == 0)
					{
						ca->is_full = 1;
						ca_cur_step = 39;
					}
				}
				if (ca->direction == shift_direction_down)
				{
					if (++ca_cur_step == 40)
					{
						ca->is_full = 1;
						ca_cur_step = 0;
					}
				}
			}
			else led_buffer_shift(ca->direction, shift_mode_erase, 1);	// Вторая стадия - сдвиги

			ca->animation.current_state = 1;
		}
	break;
	}

	/* Вызов КА для обновления палитры */
	ca_update_palette(ca);
}


