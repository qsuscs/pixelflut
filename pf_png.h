#pragma once
#include <stdint.h>

int pf_png_open(const char *path);
int pf_png_read(void);
uint32_t pf_png_height(void);
uint32_t pf_png_width(void);
uint32_t pf_png_get_rgb(uint32_t x, uint32_t y);
int pf_png_close(void);
