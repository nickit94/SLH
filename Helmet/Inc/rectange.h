#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include "main.h"

typedef struct
{
	type_object_t type;

	uint8_t x0;
	uint8_t y0;
	uint8_t radius;
	animation_t animation;
} rectangle_t;

void rect_animating(rectangle_t* rect);
void rect_create(rectangle_t* rect, uint8_t x0, uint8_t y0, uint8_t radius,
		palette_t palette, uint8_t multiplicity, uint16_t time_change_one_color);

#endif /* RECTANGLE_H_ */
