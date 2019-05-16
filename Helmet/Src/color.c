#include "color.h"

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

rgb_t color_hsv2rgb(hsv_t hsv)
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

hsv_t color_rgb2hsv(rgb_t rgb)
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



