#include "draw.h"

#define SWAP(A, B) { A ^= B; B = A ^ B; A ^= B; }

/* Отрисовка примитивов */
void draw_pixel(uint8_t x0, uint8_t y0, rgb_t rgb)
{
	if (x0 > MAX_X || y0 > MAX_Y) return;

	led_buffer[x0][y0] = rgb;

	//apa106_set_pixel(x0, y0, rgb.r, rgb.g, rgb.b);
}
void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, rgb_t rgb)
{
	if (x0 > MAX_X || y0 > MAX_Y || x1 > MAX_X || y1 > MAX_Y) return;

	uint8_t dx = _abs(x1 - x0);
	uint8_t dy = _abs(y1 - y0);
	uint8_t step = (dy > dx) ? 1 : 0;

	if (step)
	{
		SWAP(x0, y0);
		SWAP(x1, y1);
		SWAP(dx, dy);
	}

	if (x0 > x1)
	{
		SWAP(x0, x1);
		SWAP(y0, y1);
	}

	int8_t y_step = (y0 < y1) ? 1 : -1;
	int8_t err = dx >> 1;

	for ( ; x0 <= x1; x0++)
	{
		if (step)	draw_pixel(y0, x0, rgb);
		else		draw_pixel(x0, y0, rgb);

		err -= dy;
		if (err < 0)
		{
			y0 += y_step;
			err += dx;
		}
	}
}
void draw_v_line(int8_t x0, int8_t y0, int8_t length, rgb_t rgb)
{
	if (x0 > MAX_X || y0 > MAX_Y) return;

	if (length < 0)
	{
		for (uint8_t i = y0; i > (y0 + length + 1); i--)
		{
			draw_pixel(x0, y0 - i, rgb);
		}
	}
	else
	{
		for (uint8_t i = 0; i < length; ++i)
		{
			draw_pixel(x0, i + y0, rgb);
		}
	}
}
void draw_h_line(int8_t x0, int8_t y0, uint8_t length, rgb_t rgb)
{
	if (x0 > MAX_X || y0 > MAX_Y) return;

	for (uint8_t i = 0; i < length; ++i)
	{
		draw_pixel(i + x0, y0, rgb);
	}

}
void draw_rectangle(int8_t x0, int8_t y0, int8_t x1, int8_t y1, rgb_t rgb)
{
	uint8_t length = x1 - x0 + 1;
	uint8_t height = y1 - y0;

	draw_h_line(x0, y0, length, rgb);
	draw_h_line(x0, y1, length, rgb);
	draw_v_line(x0, y0, height, rgb);
	draw_v_line(x1, y0, height, rgb);
}
void draw_rectangle_fill(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, rgb_t rgb)
{
	uint8_t length = x1 - x0 + 1;
	uint8_t height = y1 - y0;

	for (int16_t x = 0; x < length; ++x)
	{
		for (int16_t y = 0; y <= height; ++y)
		{
			draw_pixel(x0 + x, y + y0, rgb);
		}
	}
}
void draw_triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb_t rgb)
{
	draw_line(x1, y1, x2, y2, rgb);
	draw_line(x2, y2, x3, y3, rgb);
	draw_line(x3, y3, x1, y1, rgb);
}
void draw_triangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb_t rgb)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
	curpixel = 0;

	deltax = _abs(x2 - x1);
	deltay = _abs(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1)	{	xinc1 = 1;	xinc2 = 1;	}
	else 			{	xinc1 = -1;	xinc2 = -1;	}

	if (y2 >= y1) 	{	yinc1 = 1;	yinc2 = 1;	}
	else 			{	yinc1 = -1;	yinc2 = -1;	}

	if (deltax >= deltay)
	{
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	}
	else
	{
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		draw_line(x, y, x3, y3, rgb);

		num += numadd;
		if (num >= den)
		{
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}
