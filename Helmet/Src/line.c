#include "line.h"

#define line_cur_step   line->animation.current_step
#define line_indx_color line->animation.index_color
#define line_multiple   line->animation.multiplicity
#define line_time_color line->animation.time_change_one_color
#define line_time_step  line->animation.time_one_line_step
#define line_palette    line->animation.palette
#define line_indx_tran  line->animation.index_transition
#define line_next_color line->animation.palette.array_colors[line->animation.index_color + 1]
#define line_cur_color  line->animation.palette.array_colors[line->animation.index_color + 0]
#define line_prev_color line->animation.palette.array_colors[line->animation.index_color - 1]
#define line_last_color line->animation.palette.array_colors[line->animation.palette.number_colors - 1]

/* private: */
void line_replacement(line_t* line)
{

	switch(line->animation.current_state)
	{
	case 0:
		line_cur_step = line->y;
		line_indx_color = 0;

		swTimerSet(&line->animation.timer_palette, 0, line_time_color / line_multiple);
		line->animation.current_state = 1;
	break;

	case 1:
		for (uint8_t i = 0; i < _round(line->length, line_multiple); i++)
			draw_v_line(line->x, line->y + (i * line_multiple), (line_cur_step + 1), line_palette.array_colors[line_indx_color]);

		container_start_update();
		line->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&line->animation.timer_palette))
		{
			if (++line_cur_step == line_multiple)
			{
				line_cur_step = 0;
				if (++line_indx_color == line_palette.number_colors) line_indx_color = 0;
			}

			line->animation.current_state = 1;
		}
	break;
	}
}
void line_accumulation(line_t* line)
{
	switch(line->animation.current_state)
	{
	case 0:
		line_indx_color = 0;
		line_cur_step = 0;
		line_indx_tran = 0;

		swTimerSet(&line->animation.timer_palette, 0, (line_time_color - ACCUMULATION_DELAY) / num_step_accumulation(line_multiple));
		line->animation.current_state = 1;
	break;

	case 1:
		for (uint8_t i = 0; i < _round(line->length, line_multiple); i++)
		{
			/* Стираем прошлым цветом или черным */
			draw_v_line(line->x, line->y + (i * line_multiple), line_multiple, (line_indx_color) ? line_prev_color : line_last_color);

			/* Рисуем текущую точку */
			draw_v_line(line->x, line->y + (i * line_multiple) + line_indx_tran, 1, line_cur_color);

			/* Рисуем точки, которые уже дошли до конца */
			draw_v_line(line->x, line->y + (i * line_multiple) + line_multiple - line_cur_step, line_cur_step, line_cur_color);
		}

		/* Задержка перед новым цветом */
		if (line_indx_tran == (line_multiple - line_cur_step - 1) && line_cur_step == (line_multiple - 1))
			line->animation.timer_palette.time = ACCUMULATION_DELAY;

		line->animation.current_state = 2;
		container_start_update();
	break;

	case 2:
		if (swTimerCheck(&line->animation.timer_palette))
		{
			if (++line_indx_tran == line_multiple - line_cur_step)
			{
				line_indx_tran = 0;
				if (++line_cur_step == line_multiple)
				{
					line_cur_step = 0;
					if (++line_indx_color == line_palette.number_colors) line_indx_color = 0;
				}
			}
			line->animation.current_state = 1;
		}
	break;
	}
}
void line_gradient(line_t* line)
{
	switch (line->animation.current_state)
	{
	case 0:
		line_cur_step = line->y;
		line_indx_color = 0;
		line_indx_tran = 0;

		swTimerSet(&line->animation.timer_palette, 0, line_time_color / GRADIENT_STEPS);
		color_get_gradient(line_palette.array_colors[0], line_palette.array_colors[1], line->animation.array_gradient);
		line->animation.current_state = 1;
	break;

	case 1:
		draw_v_line(line->x, line->y, line->length, line->animation.array_gradient[line_indx_tran]);
		container_start_update();
		line->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&line->animation.timer_palette))
		{
			if (++line_indx_tran == GRADIENT_STEPS)
			{
				line_indx_tran = 0;
				if (++line_indx_color == line_palette.number_colors) line->animation.index_color = 0;

				if (line_indx_color != line_palette.number_colors - 1)
					color_get_gradient(line_cur_color, line_next_color, line->animation.array_gradient);
				else
					color_get_gradient(line_cur_color, line_palette.array_colors[0], line->animation.array_gradient);
			}

			line->animation.current_state = 1;
		}
	break;
	}
}
void line_rainbow(line_t* line)
{
	switch(line->animation.current_state)
	{
	case 0:
		line->animation.hsv_rainbow = color_rgb2hsv((rgb_t)COLOR_RED);
		swTimerSet(&line->animation.timer_palette, 0, line_time_color / RAINBOW_DELAY);
		line->animation.current_state = 1;
	break;

	case 1:
		for (uint8_t i = 0; i < line->length; i++)
		{
			draw_pixel(line->x, line->y + i, color_hsv2rgb(line->animation.hsv_rainbow));
			line->animation.hsv_rainbow.h = (line->animation.hsv_rainbow.h + (255 / line->length)) % 255;
		}

		container_start_update();
		line->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&line->animation.timer_palette))
		{
			line->animation.hsv_rainbow.h = (line->animation.hsv_rainbow.h + RAINBOW_STEP) % 255;
			line->animation.current_state = 1;
		}
	break;
	}
}
void line_at_once(line_t* line)
{
	switch(line->animation.current_state)
	{
	case 0:
		line_indx_color = 0;
		swTimerSet(&line->animation.timer_palette, 0, line_time_color);
		line->animation.current_state = 1;
	break;

	case 1:
		draw_v_line(line->x, line->y, line->length, line_cur_color);
		container_start_update();
		line->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&line->animation.timer_palette))
		{
			if (++line_indx_color == line_palette.number_colors) line_indx_color = 0;
			line->animation.current_state = 1;
		}
	break;
	}
}

/* public: */
void line_create(line_t* line, uint8_t x, uint8_t y, uint8_t length,
		palette_t palette, uint8_t multiplicity, uint16_t time_change_one_color)
{
	line->x = x;
	line->y = y;
	line->length = (length == LINE_TO_END) ? (mas_count_leds_in_channel[x] - y) : length;
	line->type = type_object_line;
	constr_init_animation(&line->animation, palette, multiplicity, time_change_one_color, 0);
}
void line_animating(line_t* line)
{
	switch((uint8_t)line_palette.palette_rule)
	{
	case palette_rule_replacement: line_replacement(line);
		break;
	case palette_rule_accumulation: line_accumulation(line);
		break;
	case palette_rule_gradient: line_gradient(line);
		break;
	case palette_rule_rainbow: line_rainbow(line);
		break;
	case palette_rule_at_once: line_at_once(line);
		break;
	}
}
