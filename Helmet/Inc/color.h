#ifndef COLOR_H_
#define COLOR_H_

#include <main.h>

#define MAX_COLOR (APA106_MAX_BRIGHT / 3)

#define COLOR_BLACK  {0,         0,         0}
#define COLOR_RED    {MAX_COLOR, 0,         0}
#define COLOR_GREEN  {0,         MAX_COLOR, 0}
#define COLOR_BLUE   {0,         0,         MAX_COLOR}
#define COLOR_YELLOW {MAX_COLOR, MAX_COLOR, 0}
#define COLOR_PURPLE {MAX_COLOR, 0,         MAX_COLOR}
#define COLOR_CYAN   {0,         MAX_COLOR, MAX_COLOR}
#define COLOR_WHITE  {MAX_COLOR, MAX_COLOR, MAX_COLOR}

#define COLOR_ORANGE {84, 27, 0}
#define COLOR_LIGHT_GREEN {48, 84, 0}
#define COLOR_PINK {84, 0, 42}
#define COLOR_LIGNT_BLUE {0, 31, 84}


uint8_t _abs(int16_t num);
uint8_t _max(uint8_t a, uint8_t b);
uint8_t _min(uint8_t a, uint8_t b);
uint8_t _round(uint8_t a, uint8_t b);

rgb_t hsv2rgb(hsv_t hsv);
hsv_t rgb2hsv(rgb_t rgb);

void color_get_gradient(rgb_t color1, rgb_t color2, rgb_t* mas_color);
rgb_t color_reduce(rgb_t rgb, uint8_t num);

#endif /* COLOR_H_ */
