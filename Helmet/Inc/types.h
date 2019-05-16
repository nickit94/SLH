#ifndef TYPES_H_
#define TYPES_H_

#include "main.h"

#define RAINBOW_STEP 1
#define GRADIENT_STEPS 11
#define RAINBOW_DELAY 100
#define ACCUMULATION_DELAY 400

/********************************************************/
typedef struct
{
  uint8_t status;
  uint32_t period;
  uint32_t time;
  uint32_t prev_time_system;
} swtimer_t;

/********************************************************/
typedef struct
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
} hsv_t;

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;

/********************************************************/
typedef enum
{
	palette_rule_replacement,	/* Замещение */
	palette_rule_accumulation,	/* Накопление - по одному светодиоду в конец, пока не заполнится */
	palette_rule_gradient,		/* Градиент */
	palette_rule_rainbow,		/* Радуга - массив цветов не используется */
	palette_rule_at_once,		/* Разом, без анимации */
} palette_rule_t;

/* Палитра */
typedef struct
{
	palette_rule_t palette_rule;	/* Правила откраски */
	rgb_t array_colors[20];			/* Массив цветов */
	uint8_t number_colors;			/* Кол-во записанных цветов в массив colors */
} palette_t;

/********************************************************/
typedef enum
{
	shift_direction_up = 1,
	shift_direction_down,
	shift_direction_left,
	shift_direction_right
} shift_direction_t;

typedef enum
{
	shift_mode_tor = 1,
	shift_mode_erase
} shift_mode_t;

/********************************************************/
typedef struct
{
	/* Служебное */
	uint8_t current_state;					/* Текущее состояние КА */
	uint8_t current_step;					/* Текущий шаг (кадр) анимации */
	uint8_t index_color;					/* Индекс текущего цвета */
	uint8_t index_transition;				/* Индекс перехода - для градиента индекс массива градиента, для накопления - текущее положение блока */
	rgb_t array_gradient[GRADIENT_STEPS];	/* Массив цветов для режима градиент */
	hsv_t hsv_rainbow;						/* Цвет в hsv для режима радуги */
	swtimer_t timer_palette;				/* Таймер для задания скорости анимации палитры */
	swtimer_t timer_random;					/* Таймер для задания скорости генерации случайных фигур */

	/* Для пользователя */
	uint8_t multiplicity;					/* Кратность */
	uint16_t time_change_one_color;			/* Скорость смены */
	uint16_t time_one_ca_step;				/* Скорость шага клеточного автомата, если он есть */
	palette_t palette;						/* Палитра для текущей анимации */
} animation_t;

/*******************************************************/
typedef enum
{
	type_object_none = 0,
	type_object_line = 1,
	type_object_rect,
	type_object_ca
} type_object_t;

/**************************************************/
typedef struct
{
  GPIO_TypeDef* port;
  uint16_t pin;
  uint8_t state;
  uint16_t time_press;
  uint8_t count_press;
  uint32_t prev_time_system;
  uint8_t state_fsm;
  swtimer_t timer_debounce;
  swtimer_t timer_count_press;
} button_t;

#endif
