#include "led_buffer.h"

#define asvb array_size_virtual_bufs

const uint8_t mas_count_leds_in_channel[APA106_NUM_CHANNELS] = {
		APA106_CH0_NUM_PIXEL, APA106_CH1_NUM_PIXEL, APA106_CH2_NUM_PIXEL,
		APA106_CH3_NUM_PIXEL, APA106_CH4_NUM_PIXEL, APA106_CH5_NUM_PIXEL,
		APA106_CH6_NUM_PIXEL, APA106_CH7_NUM_PIXEL, APA106_CH8_NUM_PIXEL,
		APA106_CH9_NUM_PIXEL, APA106_CH10_NUM_PIXEL, APA106_CH11_NUM_PIXEL,
		APA106_CH12_NUM_PIXEL
};

const uint8_t array_size_virtual_bufs[MAX_X_VIRTUAL] =
{
	22, 18, 18, 11, 15, 18, 			// 6 виртуальных каналов слева (отсчет снизу-вверх)
	13, 12, 12, 12, 12, 12, 13, 		// 7 физических каналов посередине (отсчет слева-направо)
	18, 15, 11, 17, 18, 22,				// 6 виртуальных каналов справа (отсчет сверху-вниз)
};

static rgb_t backup_x[MAX_X];
static rgb_t backup_y[MAX_Y];

	   rgb_t led_buffer[MAX_X][MAX_Y];

/* private: */

void led_buffer_shift_one_pos(shift_direction_t direction, shift_mode_t mode)
{
	switch((uint8_t)direction)
	{
	/* К лбу */
	case shift_direction_up:

		/* Бэкап выдавливаемой строки */
		for (uint8_t x = 0; x < MAX_X; x++) backup_x[x] = led_buffer[x][0];

		/* Сдвиг */
		for (uint8_t y = 1; y < MAX_Y; y++)
		{
			for (uint8_t x = 0; x < MAX_X; x++)
			{
				if ((y - 1) > mas_count_leds_in_channel[x]) continue;
				led_buffer[x][y - 1] = led_buffer[x][y];
			}
		}

		/* Применение режима сдвига - стирание или тор */
		if (mode == shift_mode_erase)
		{
			for (uint8_t x = 0; x < MAX_X; x++) led_buffer[x][mas_count_leds_in_channel[x] - 1] = (rgb_t)COLOR_BLACK;
		}
		if (mode == shift_mode_tor)
		{
			for (uint8_t x = 0; x < MAX_X; x++) led_buffer[x][mas_count_leds_in_channel[x] - 1] = backup_x[x];
		}

	break;

	/* К затылку */
	case shift_direction_down:

		/* Бэкап выдавливаемой строки */
		for (uint8_t x = 0; x < MAX_X; x++) backup_x[x] = led_buffer[x][mas_count_leds_in_channel[x] - 1];

		/* Сдвиг */
		for (uint8_t y = 0; y < MAX_Y - 1; y++)
		{
			for (uint8_t x = 0; x < MAX_X; x++)
			{
				if ((y + 2) > mas_count_leds_in_channel[x]) continue;
				led_buffer[x][mas_count_leds_in_channel[x] - (1 + y)] = led_buffer[x][mas_count_leds_in_channel[x] - (2 + y)];
			}
		}

		/* Применение режима сдвига - стирание или тор */
		if (mode == shift_mode_erase)
		{
			for (uint8_t x = 0; x < MAX_X; x++) led_buffer[x][0] = (rgb_t)COLOR_BLACK;
		}
		if (mode == shift_mode_tor)
		{
			for (uint8_t x = 0; x < MAX_X; x++) led_buffer[x][0] = backup_x[x];
		}
	break;

	case shift_direction_left:

		/* ---------- Бэкап выдавливаемой строки ---------- */
		for (uint8_t y = 0; y < MAX_Y_VIRTUAL; y++) backup_y[y] = led_buffer[2][asvb[5] + y];

		/* ---------- Копирование виртуальных каналов слева ---------- */
		for (uint8_t y = 0; y < asvb[0]; y++)
			led_buffer[2][asvb[5] + y] = (y < asvb[1]) ? led_buffer[1][asvb[4] + y] : (rgb_t)COLOR_BLACK; 	// 2.2 <- 1.2

		for (uint8_t y = 0; y < asvb[2]; y++)
			led_buffer[1][asvb[4] + y] = led_buffer[0][asvb[3] + y]; 								// 1.2 <- 0.2

		for (uint8_t y = 0; y < asvb[2]; y++)
			led_buffer[0][asvb[3] + y] = (y < asvb[3]) ? led_buffer[0][0 + y] : (rgb_t)COLOR_BLACK; 		// 0.2 <- 0.1

		/* ---------- Копирование физических каналов по центру ---------- */
		// 0.1 - 11.1
		for (uint8_t x = 1; x < MAX_X; x++)
		{
			if (asvb[3 + x - 1] < asvb[3 + x])														// Просто перенос
			{
				for (uint8_t y = 0; y < _min(asvb[3 + x - 1], asvb[3 + x]); y++)
					led_buffer[x - 1][y] = led_buffer[x][y];
			}
			else																					// Перенос с затиркой
			{
				for (uint8_t y = 0; y < _max(asvb[3 + x - 1], asvb[3 + x]); y++)
					led_buffer[x - 1][y] = (y < asvb[3 + x]) ? led_buffer[x][y] : (rgb_t)COLOR_BLACK;
			}
		}

		/* ---------- Копирование виртуальных каналов справа ---------- */

		for (uint8_t y = 0; y < asvb[15]; y++)
			led_buffer[12][0 + y] = led_buffer[12][asvb[15] + y]; 									// 12.1 <- 12.2

		for (uint8_t y = 0; y < asvb[16]; y++)
			led_buffer[12][asvb[15] + y] = led_buffer[11][asvb[14] + y]; 							// 12.2 <- 11.2

		for (uint8_t y = 0; y < asvb[17]; y++)
			led_buffer[11][asvb[14] + y] = led_buffer[10][asvb[13] + y]; 							// 11.2 <- 10.2


		/* ---------- Применение режима сдвига - стирание или тор ---------- */
		if (mode == shift_mode_erase)
		{
			for (uint8_t y = 0; y < asvb[18]; y++) led_buffer[MAX_X - 3][asvb[13] + y] = (rgb_t)COLOR_BLACK;
		}
		if (mode == shift_mode_tor)
		{
			for (uint8_t y = 0; y < asvb[18]; y++) led_buffer[MAX_X - 3][asvb[13] + y] = backup_y[y];
		}

	break;

	case shift_direction_right:

		/* ---------- Бэкап выдавливаемой строки ---------- */
		for (uint8_t y = 0; y < MAX_Y_VIRTUAL; y++) backup_y[y] = led_buffer[12][asvb[13] + y];

		/* ---------- Копирование виртуальных каналов справа ---------- */
		for (uint8_t y = 0; y < asvb[18]; y++)
			led_buffer[10][asvb[13] + y] = (y < asvb[17]) ? led_buffer[11][asvb[14] + y] : (rgb_t)COLOR_BLACK;	// 10.2 <- 11.2

		for (uint8_t y = 0; y < asvb[17]; y++)
			led_buffer[11][asvb[14] + y] = (y < asvb[16]) ? led_buffer[12][asvb[15] + y] : (rgb_t)COLOR_BLACK;	// 11.2 <- 12.2

		for (uint8_t y = 0; y < asvb[16]; y++)
			led_buffer[12][asvb[15] + y] = (y < asvb[15]) ? led_buffer[12][0 + y] : (rgb_t)COLOR_BLACK;		// 12.2 <- 12.1


		/* ---------- Копирование физических каналов по центру ---------- */
		// 12.1 - 1.1
		for (uint8_t x = 0; x < MAX_X - 1; x++)
		{
			if (asvb[(MAX_X_VIRTUAL - 1) - (3 + x)] < asvb[(MAX_X_VIRTUAL - 1) - (4 + x)])				// Просто перенос
			{
				for (uint8_t y = 0; y < _min(asvb[(MAX_X_VIRTUAL - 1) - (3 + x)], asvb[(MAX_X_VIRTUAL - 1) - (4 + x)]); y++)
					led_buffer[(MAX_X - 1) - x][y] = led_buffer[(MAX_X - 1) - (x + 1)][y];
			}
			else																						// Перенос с затиркой
			{
				for (uint8_t y = 0; y < _max(asvb[(MAX_X_VIRTUAL - 1) - (3 + x)], asvb[(MAX_X_VIRTUAL - 1) - (4 + x)]); y++)
					led_buffer[(MAX_X - 1) - x][y] = (y < asvb[(MAX_X_VIRTUAL - 1) - (4 + x)]) ? led_buffer[(MAX_X - 1) - (x + 1)][y] : (rgb_t)COLOR_BLACK;
			}
		}

		/* ---------- Копирование виртуальных каналов слева ---------- */
		for (uint8_t y = 0; y < asvb[3]; y++)
			led_buffer[0][0 + y] = led_buffer[0][asvb[3] + y]; 											// 0.1 <- 0.2

		for (uint8_t y = 0; y < asvb[2]; y++)
			led_buffer[0][asvb[3] + y] = led_buffer[1][asvb[4] + y]; 									// 0.2 <- 1.2

		for (uint8_t y = 0; y < asvb[1]; y++)
			led_buffer[1][asvb[4] + y] = led_buffer[2][asvb[5] + y]; 									// 1.2 <- 2.2

		/* ---------- Применение режима сдвига - стирание или тор ---------- */
		if (mode == shift_mode_erase)
		{
			for (uint8_t y = 0; y < asvb[0]; y++) led_buffer[2][asvb[5] + y] = (rgb_t)COLOR_BLACK;
		}
		if (mode == shift_mode_tor)
		{
			for (uint8_t y = 0; y < asvb[0]; y++) led_buffer[2][asvb[5] + y] = backup_y[y];
		}

	break;
	}
}

/* public: */
void led_buffer_init()
{
	for (uint8_t x = 0; x < MAX_X; x++)
	{
		for (uint8_t y = 0; y < MAX_Y; y++)
		{
			led_buffer[x][y] = (rgb_t)COLOR_BLACK;
		}
	}
}

void led_buffer_prepare_for_apa106(uint8_t led_channel)
{
	for (uint8_t y = 0; y < mas_count_leds_in_channel[led_channel]; y++)
	{
		apa106_set_pixel(led_channel, y, led_buffer[led_channel][y].r, led_buffer[led_channel][y].g, led_buffer[led_channel][y].b);
	}
}

void led_buffer_change_color(rgb_t new_color)
{
	for (uint8_t x = 0; x < MAX_X; x++)
	{
		for (uint8_t y = 0; y < MAX_Y; y++)
		{
			if (led_buffer[x][y].r || led_buffer[x][y].g || led_buffer[x][y].b) led_buffer[x][y] = new_color;	// Если текущий цвет не черный
		}
	}
}

void led_buffer_shift(shift_direction_t direction, shift_mode_t mode, uint8_t num_shift)
{
	for (uint8_t i = 0; i < num_shift; i++)
	{
		led_buffer_shift_one_pos(direction, mode);
	}
}

void led_buffer_clear(uint8_t x)
{
	for (uint8_t y = 0; y < MAX_Y; y++)
	{
		led_buffer[x][y] = (rgb_t)COLOR_BLACK;
	}
}

void led_buffer_clear_all()
{
	for (uint8_t x = 0; x < MAX_X; x++)
	{
		for (uint8_t y = 0; y < MAX_Y; y++)
		{
			led_buffer[x][y] = (rgb_t)COLOR_BLACK;
		}
	}
}
