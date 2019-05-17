#ifndef LINE_H_
#define LINE_H_

#include <main.h>

#define LINE_TO_END	200

typedef enum
{
	line_orient_horizontal = 1,
	line_orient_vertical,
} line_orient_t;

typedef struct
{
	type_object_t type;

	uint8_t x;
	uint8_t y;
	uint8_t length;
	animation_t animation;
} line_t;


void line_animating(line_t* line);
void line_create(line_t* line, uint8_t x, uint8_t y, uint8_t length,
		palette_t palette, uint8_t multiplicity, uint16_t time_change_one_color);

#endif /* LINE_H_ */
