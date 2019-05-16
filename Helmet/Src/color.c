#include "color.h"




//const rgb_t COLOR_BLACK  = {0,         0,         0};
//rgb_t color_red = {MAX_COLOR, 0,         0};
//const rgb_t COLOR_GREEN  = {0,         MAX_COLOR, 0};
//const rgb_t COLOR_BLUE   = {0,         0,         MAX_COLOR};
//const rgb_t COLOR_YELLOW = {MAX_COLOR, MAX_COLOR, 0};
//const rgb_t COLOR_PURPLE = {MAX_COLOR, 0,         MAX_COLOR};
//const rgb_t COLOR_CYAN   = {0,         MAX_COLOR, MAX_COLOR};
//const rgb_t COLOR_WHITE  = {MAX_COLOR, MAX_COLOR, MAX_COLOR};
//
//const rgb_t COLOR_ORANGE = {84, 27, 0};
//const rgb_t COLOR_LIGHT_GREEN = {48, 84, 0};
//const rgb_t COLOR_PINK = {84, 0, 42};
//const rgb_t COLOR_LIGNT_BLUE = {0, 31, 84};





uint8_t _abs(int16_t num)
{
	return (num < 0) ? ((uint8_t)((-1) * (num))) : ((uint8_t)num);
}
uint8_t _max(uint8_t a, uint8_t b)
{
	return (a > b) ? a : b;
}
uint8_t _min(uint8_t a, uint8_t b)
{
	return (a < b) ? a : b;
}
uint8_t _round(uint8_t a, uint8_t b)
{
	return (uint8_t)(((uint16_t)a * 10 / b) + 9) / 10;
}

inline rgb_t _rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (rgb_t) {r, g, b};
}
inline hsv_t _hsv(uint8_t h, uint8_t s, uint8_t v)
{
    return (hsv_t) {h, s, v};
}

rgb_t hsv2rgb(hsv_t hsv)
{
    rgb_t rgb;
    uint8_t region, remainder, p, q, t;

    if (hsv.s == 0) return _rgb(hsv.v, hsv.v, hsv.v);

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:  rgb.r = hsv.v; rgb.g = t;     rgb.b = p;     break;
    case 1:  rgb.r = q;     rgb.g = hsv.v; rgb.b = p;     break;
    case 2:  rgb.r = p;     rgb.g = hsv.v; rgb.b = t;     break;
    case 3:  rgb.r = p;     rgb.g = q;     rgb.b = hsv.v; break;
    case 4:  rgb.r = t;     rgb.g = p;     rgb.b = hsv.v; break;
    default: rgb.r = hsv.v; rgb.g = p;     rgb.b = q;     break;
    }

    return rgb;
}
hsv_t rgb2hsv(rgb_t rgb)
{
    hsv_t hsv;

    uint8_t min = (rgb.r < rgb.g) ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    uint8_t max = (rgb.r > rgb.g) ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    if (max == 0) return _hsv(0, 0, 0);

    hsv.v = max;
    hsv.s = 255 * (int16_t)(max - min) / hsv.v;

    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if      (max == rgb.r) 	hsv.h = 0 + 43 * (rgb.g - rgb.b) / (max - min);
    else if (max == rgb.g)  hsv.h = 85 + 43 * (rgb.b - rgb.r) / (max - min);
    else 					hsv.h = 171 + 43 * (rgb.r - rgb.g) / (max - min);

    return hsv;
}

void color_get_gradient(rgb_t color1, rgb_t color2, rgb_t* mas_color)
{
	int16_t dr = (int16_t)(color2.r - color1.r) * 100 / (GRADIENT_STEPS - 1);
	int16_t dg = (int16_t)(color2.g - color1.g) * 100 / (GRADIENT_STEPS - 1);
	int16_t db = (int16_t)(color2.b - color1.b) * 100 / (GRADIENT_STEPS - 1);

	uint16_t new_r = (uint16_t)color1.r * 100;
	uint16_t new_g = (uint16_t)color1.g * 100;
	uint16_t new_b = (uint16_t)color1.b * 100;

	for (uint8_t i = 0; i < GRADIENT_STEPS; i++)
	{
		mas_color[i] = _rgb(new_r / 100, new_g / 100, new_b / 100);

		new_r += dr;
		new_g += dg;
		new_b += db;
	}
}

rgb_t color_reduce(rgb_t rgb, uint8_t num)
{
	return _rgb(rgb.r / num, rgb.g / num, rgb.b / num);
}

/*

void draw_line_animate2(uint8_t x0, uint8_t y0, uint8_t lenght, palette_t palette)
{
	switch(state_line_animate)
	{
	 init + start pos
	case 0:
		cur_step = y0;
		cur_accumulation_index = y0;
		cur_color_index = 0;
		cur_gradient_index = 0;

		switch((uint8_t)palette.palette_rule)
		{
		case Replacement:
			for (uint8_t i = 0; i < _round(lenght, palette.multiplicity); i++)
			{
				draw_v_line(x0, y0 + (i * palette.multiplicity), 1, palette.colors[0]);
			}

			swTimerSet(&st_animate, 0, palette.time_change_one_color / palette.multiplicity);
		break;

		case Accumulation:
			for (uint8_t i = 0; i < _round(lenght, palette.multiplicity); i++)
			{
				draw_v_line(x0, y0 + (i * palette.multiplicity), 1, palette.colors[0]);
			}

			swTimerSet(&st_animate, 0, (palette.time_change_one_color - ACCUMULATION_DELAY) / num_step_accumulation(palette.multiplicity));
		break;

		case Gradient:
			draw_v_line(x0, y0, lenght, palette.colors[0]);
			swTimerSet(&st_animate, 0, palette.time_change_one_color / GRADIENT_STEPS);
			get_gradient(palette.colors[0], palette.colors[1], mas_gradient);
		break;

		case Rainbow:
			hsv_rainbow = rgb2hsv(COLOR_RED);
			draw_v_line(x0, y0, 1, COLOR_RED);
			swTimerSet(&st_animate, 0, palette.time_change_one_color / RAINBOW_DELAY);
		break;

		case AtOnce:
			draw_v_line(x0, y0, lenght, palette.colors[0]);
			swTimerSet(&st_animate, 0, palette.time_change_one_color);
		break;
		}

		state_line_animate = 1;
		apa106_start_update();
	break;

	 next frame
	case 1:
		switch((uint8_t)palette.palette_rule)
		{
		case Replacement:
			for (uint8_t i = 0; i < _round(lenght, palette.multiplicity); i++)
			{
				draw_v_line(x0, y0 + (i * palette.multiplicity), (cur_step + 1), palette.colors[cur_color_index]);	// Рисуем новое
			}
		break;

		case Accumulation:
			for (uint8_t i = 0; i < _round(lenght, palette.multiplicity); i++)
			{
				draw_v_line(x0, y0 + (i * palette.multiplicity), palette.multiplicity, (cur_color_index) ? palette.colors[cur_color_index - 1] : COLOR_BLACK);	// Стираем прошлым цветом или черным
				draw_v_line(x0, y0 + (i * palette.multiplicity) + cur_accumulation_index - cur_step, 1, palette.colors[cur_color_index]);			// Рисуем текущую точку
				draw_v_line(x0, y0 + (i * palette.multiplicity) + palette.multiplicity - cur_step, cur_step, palette.colors[cur_color_index]);					// Рисуем точки, которые уже дошли до конца

				if (cur_accumulation_index == (palette.multiplicity - 1) && cur_step == (palette.multiplicity - 1)) st_animate.time = ACCUMULATION_DELAY;		// Задержка перед новым цветом
			}
		break;

		case Gradient:
			draw_v_line(x0, y0, lenght, mas_gradient[cur_gradient_index]);
		break;

		case Rainbow:
			hsv_h_prev = hsv_rainbow.h;

			for (uint8_t i = 0; i < lenght; i++)
			{
				draw_pixel(x0, y0 + i, hsv2rgb(hsv_rainbow));
				hsv_rainbow.h = (hsv_rainbow.h + (255 / lenght)) % 255;
			}

			hsv_rainbow.h = hsv_h_prev;
		break;

		case AtOnce:
			draw_v_line(x0, y0, lenght, palette.colors[cur_color_index]);
		break;
		}

		state_line_animate = 2;
		apa106_start_update();
	break;

	 delay
	case 2:
		if (swTimerCheck(&st_animate))
		{
			switch((uint8_t)palette.palette_rule)
			{
			case Replacement:
				if (++cur_step == palette.multiplicity)
				{
					cur_step = 0;
					if (++cur_color_index == palette.number_colors) cur_color_index = 0;
				}
			break;

			case Accumulation:
				if (++cur_accumulation_index == palette.multiplicity)
				{
					cur_accumulation_index = 0;
					if (++cur_step == palette.multiplicity)
					{
						cur_step = 0;
						if (++cur_color_index == palette.number_colors) cur_color_index = 0;
					}
				}
			break;

			case Gradient:
				if (++cur_gradient_index == GRADIENT_STEPS)
				{
					cur_gradient_index = 0;
					if (++cur_color_index == palette.number_colors) 	cur_color_index = 0;
					if (cur_color_index != palette.number_colors - 1) 	get_gradient(palette.colors[cur_color_index], palette.colors[cur_color_index + 1], mas_gradient);
					else												get_gradient(palette.colors[cur_color_index], palette.colors[0],            	   mas_gradient);
				}
			break;

			case Rainbow:
				hsv_rainbow.h = (hsv_rainbow.h + RAINBOW_STEP) % 255;
			break;

			case AtOnce:
				if (++cur_color_index == palette.number_colors) cur_color_index = 0;
			break;
			}

			state_line_animate = 1;
		}
	break;
	}
}
*/









