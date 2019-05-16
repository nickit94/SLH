#include "rectange.h"

#define rect_cur_step   rect->animation.current_step
#define rect_indx_color rect->animation.index_color
#define rect_multiple   rect->animation.multiplicity
#define rect_time_color rect->animation.time_change_one_color
#define rect_time_step  rect->animation.time_one_ca_step
#define rect_palette    rect->animation.palette
#define rect_indx_tran  rect->animation.index_transition
#define rect_next_color rect->animation.palette.array_colors[rect->animation.index_color + 1]
#define rect_cur_color  rect->animation.palette.array_colors[rect->animation.index_color + 0]
#define rect_prev_color rect->animation.palette.array_colors[rect->animation.index_color - 1]
#define rect_last_color rect->animation.palette.array_colors[rect->animation.palette.number_colors - 1]

/* private: */
void rect_replacement(rectangle_t* rect)
{
	switch(rect->animation.current_state)
	{
	case 0:
		rect_cur_step = 0;
		rect_indx_color = 0;

		swTimerSet(&rect->animation.timer_palette, 0, rect_time_color / rect_multiple);
		rect->animation.current_state = 1;
	break;

	case 1:
		for (uint8_t i = 0; i < _round(rect->radius, rect_multiple); i++)
		{
			draw_rectangle(rect->x0 - rect_cur_step - (i * rect_multiple), rect->y0 - rect_cur_step - (i * rect_multiple),
					rect->x0 + rect_cur_step + (i * rect_multiple), rect->y0 + rect_cur_step + (i * rect_multiple), rect_cur_color);
		}

		//apa106_start_update();
		container_start_update();
		rect->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&rect->animation.timer_palette))
		{
			if (++rect_cur_step == rect_multiple)
			{
				rect_cur_step = 0;
				if (++rect_indx_color == rect_palette.number_colors) rect_indx_color = 0;
			}

			rect->animation.current_state = 1;
		}
	break;
	}
}
void rect_accumulation(rectangle_t* rect)
{
	switch(rect->animation.current_state)
	{
	case 0:
		rect_indx_color = 0;
		rect_cur_step = 0;
		rect_indx_tran = 0;

		swTimerSet(&rect->animation.timer_palette, 0, (rect_time_color - ACCUMULATION_DELAY) / num_step_accumulation(rect_multiple));
		rect->animation.current_state = 1;
	break;

	case 1:
		/* Стираем прошлым цветом */
		draw_rectangle_fill(rect->x0 - (rect->radius - 1), rect->y0 - (rect->radius - 1), rect->x0 + (rect->radius - 1), rect->y0 + (rect->radius - 1),
				(rect_indx_color) ? rect_prev_color : rect_last_color);

		for (uint8_t i = 0, rad = 0; i < _round(rect->radius, rect_multiple); i++)
		{
			/* Текущая "точка" */
			rad = (rect_indx_tran) + (i * rect_multiple);
			draw_rectangle(rect->x0 - rad, rect->y0 - rad, rect->x0 + rad, rect->y0 + rad, rect_cur_color);

			/* Уже накопленные "точки" */
			for (uint8_t j = (rect_multiple - rect_cur_step); j < rect_multiple; j++)
			{
				rad = j + (i * rect_multiple);
				draw_rectangle(rect->x0 - rad, rect->y0 - rad, rect->x0 + rad, rect->y0 + rad, rect_cur_color);
			}
		}

		/* Задержка перед новым цветом */
		if (rect_indx_tran == (rect_multiple - 1) && rect_cur_step == (rect_multiple - 1))
			rect->animation.timer_palette.time = ACCUMULATION_DELAY;

		rect->animation.current_state = 2;
		//apa106_start_update();
		container_start_update();
	break;

	case 2:
		if (swTimerCheck(&rect->animation.timer_palette))
		{
			if (++rect_indx_tran == rect_multiple - rect_cur_step)
			{
				rect_indx_tran = 0;
				if (++rect_cur_step == rect_multiple)
				{
					rect_cur_step = 0;
					if (++rect_indx_color == rect_palette.number_colors) rect_indx_color = 0;
				}
			}

			rect->animation.current_state = 1;
		}
	break;
	}
}
void rect_gradient(rectangle_t* rect)
{
	switch(rect->animation.current_state)
	{
	case 0:
		rect_cur_step = 0;
		rect_indx_color = 0;
		rect_indx_tran = 0;

		swTimerSet(&rect->animation.timer_palette, 0, rect_time_color / GRADIENT_STEPS);
		color_get_gradient(rect_palette.array_colors[0], rect_palette.array_colors[1], rect->animation.array_gradient);

		rect->animation.current_state = 1;
	break;

	case 1:
		draw_rectangle_fill(rect->x0 - (rect->radius - 1), rect->y0 - (rect->radius - 1), rect->x0 + (rect->radius - 1), rect->y0 + (rect->radius - 1),
				rect->animation.array_gradient[rect_indx_tran]);
		//apa106_start_update();
		container_start_update();
		rect->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&rect->animation.timer_palette))
		{
			if (++rect_indx_tran == GRADIENT_STEPS)
			{
				rect_indx_tran = 0;
				if (++rect_indx_color == rect_palette.number_colors) rect_indx_color = 0;

				if (rect_indx_color != rect_palette.number_colors - 1)
					color_get_gradient(rect_cur_color, rect_next_color, rect->animation.array_gradient);
				else
					color_get_gradient(rect_cur_color, rect_palette.array_colors[0], rect->animation.array_gradient);
			}

			rect->animation.current_state = 1;
		}
	break;
	}
}
void rect_rainbow(rectangle_t* rect)
{
	switch(rect->animation.current_state)
	{
	case 0:
		rect->animation.hsv_rainbow = color_rgb2hsv((rgb_t)COLOR_RED);
		swTimerSet(&rect->animation.timer_palette, 0, rect_time_color / RAINBOW_DELAY);
		rect->animation.current_state = 1;
	break;

	case 1:
		for (uint8_t i = 0; i < rect->radius; i++)
		{
			draw_rectangle(rect->x0 - i, rect->y0 - i, rect->x0 + i, rect->y0 + i, color_hsv2rgb(rect->animation.hsv_rainbow));
			rect->animation.hsv_rainbow.h = (rect->animation.hsv_rainbow.h + (255 / rect->radius)) % 255;
		}

		//apa106_start_update();
		container_start_update();
		rect->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&rect->animation.timer_palette))
		{
			rect->animation.hsv_rainbow.h = (rect->animation.hsv_rainbow.h + RAINBOW_STEP) % 255;
			rect->animation.current_state = 1;
		}
	break;
	}
}
void rect_at_once(rectangle_t* rect)
{
	switch(rect->animation.current_state)
	{
	case 0:
		rect_indx_color = 0;
		swTimerSet(&rect->animation.timer_palette, 0, rect_time_color);

		rect->animation.current_state = 1;
	break;

	case 1:
		draw_rectangle_fill(rect->x0 - rect->radius, rect->y0 - rect->radius, rect->x0 + rect->radius, rect->y0 + rect->radius, rect_cur_color);
		//apa106_start_update();
		container_start_update();
		rect->animation.current_state = 2;
	break;

	case 2:
		if (swTimerCheck(&rect->animation.timer_palette))
		{
			if (++rect_indx_color == rect_palette.number_colors) rect_indx_color = 0;
			rect->animation.current_state = 1;
		}
	break;
	}
}

/* public: */
void rect_create(rectangle_t* rect, uint8_t x0, uint8_t y0, uint8_t radius,
		palette_t palette, uint8_t multiplicity, uint16_t time_change_one_color)
{
	rect->x0 = x0;
	rect->y0 = y0;
	rect->radius = radius;
	rect->type = type_object_rect;

	constr_init_animation(&rect->animation, palette, multiplicity, time_change_one_color, 0);
}
void rect_animating(rectangle_t* rect)
{
	switch((uint8_t)rect_palette.palette_rule)
	{
	case palette_rule_replacement: rect_replacement(rect);
		break;
	case palette_rule_accumulation: rect_accumulation(rect);
		break;
	case palette_rule_gradient: rect_gradient(rect);
		break;
	case palette_rule_rainbow: rect_rainbow(rect);
		break;
	case palette_rule_at_once: rect_at_once(rect);
		break;
	}
}
