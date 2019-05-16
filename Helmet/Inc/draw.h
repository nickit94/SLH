#ifndef DRAW_H_
#define DRAW_H_

#include <main.h>

void draw_pixel(uint8_t x0, uint8_t y0, rgb_t rgb);
void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, rgb_t rgb);
void draw_v_line(int8_t x0, int8_t y0, int8_t length, rgb_t rgb);
void draw_h_line(int8_t x0, int8_t y0, uint8_t length, rgb_t rgb);
void draw_rectangle(int8_t x0, int8_t y0, int8_t x1, int8_t y1, rgb_t rgb);
void draw_rectangle_fill(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, rgb_t rgb);
void draw_triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb_t rgb);
void draw_triangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb_t rgb);

#endif
