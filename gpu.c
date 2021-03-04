
#include "gpu.h"
#include "bios.h"

#define GPU_GP1_RESET 0x00
#define GPU_GP1_DISPLAY_ENABLE 0x03
#define GPU_GP1_DISPLAY_MODE 0x08
#define GPU_GP0_FILL_VRAM 0x02

volatile uint32_t * GPU_STAT = (volatile uint32_t *) 0x1F801814;

bool gpu_is_pal(void) {
	return (*GPU_STAT & 8) != 0;
}

void gpu_reset(void) {
	SendGP1Command(GPU_GP1_RESET << 24);
}

void gpu_display_mode(uint32_t mode) {
	SendGP1Command(GPU_GP1_DISPLAY_MODE << 24 | mode);
}

void gpu_display(bool enable) {
	if (enable) {
		SendGP1Command(GPU_GP1_DISPLAY_ENABLE << 24 | 0);
	} else {
		SendGP1Command(GPU_GP1_DISPLAY_ENABLE << 24 | 1);
	}
}

void gpu_fill_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t rgb) {
	uint8_t r = rgb >> 16;
	uint8_t g = rgb >> 8;
	uint8_t b = rgb >> 0;

	uint32_t buf[3];
	buf[0] = GPU_GP0_FILL_VRAM << 24 | (uint32_t) b << 16 | (uint32_t) g << 8 | (uint32_t) r;
	buf[1] = (uint32_t) y << 16 | x;
	buf[2] = (uint32_t) height << 16 | width;
	GPU_cwp(buf, 3);
}

void gpu_draw_tex_rect(const struct gpu_tex_rect * rect) {
	uint32_t buf[4];

	if (rect->raw_tex) {
		buf[0] = 0x65000000;
	} else {
		uint8_t r = rect->color >> 16;
		uint8_t g = rect->color >> 8;
		uint8_t b = rect->color >> 0;

		buf[0] = 0x65000000 | (uint32_t) b << 16 | (uint32_t) g << 8 | (uint32_t) r;
	}

	if (rect->semi_transp) {
		buf[0] |= 0x02000000;
	}

	buf[1] = (uint32_t) rect->draw_y << 16 | rect->draw_x;
	buf[2] = (uint32_t) rect->clut_y << 22 | (rect->clut_x / 16) << 16 | rect->tex_y << 8 | rect->tex_x;
	buf[3] = (uint32_t) rect->height << 16 | rect->width;

	GPU_cwp(buf, 4);
}

void gpu_set_drawing_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	GPU_cw(0xE3000000 | y << 10 | x);
	GPU_cw(0xE4000000 | (y + height) << 10 | (x + width));
}

void gpu_flush_cache(void) {
	GPU_cw(0x01000000);
}