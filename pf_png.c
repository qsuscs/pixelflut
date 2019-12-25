#include <stdint.h>
#include <stdlib.h>

#include <png.h>

#include "pf_png.h"

static png_image image;
static uint32_t height, width;
static uint8_t *buffer = NULL;

int pf_png_open(const char *path)
{
	image.version = PNG_IMAGE_VERSION;
	image.opaque = NULL;
	int ret = png_image_begin_read_from_file(&image, path);
	height = image.height;
	width = image.width;
	return !ret;
}

int pf_png_read(void)
{
	image.format = PNG_FORMAT_RGB;
	buffer = calloc(PNG_IMAGE_SIZE(image), 1);
	if (!buffer) {
		pf_png_close();
		return -1;
	}
	int ret = png_image_finish_read(&image, NULL, buffer, 0, NULL);
	return !ret;
}

uint32_t pf_png_height(void)
{
	return height;
}

uint32_t pf_png_width(void)
{
	return width;
}

uint32_t pf_png_get_rgb(uint32_t x, uint32_t y)
{
	int stride = PNG_IMAGE_ROW_STRIDE(image) *
		     PNG_IMAGE_PIXEL_COMPONENT_SIZE(image.format);
	uint8_t *row = buffer + stride * y;
	uint8_t *px = row + PNG_IMAGE_PIXEL_SIZE(image.format);
	uint32_t ret = (*px << 16) | (*(px + 1) << 8) | (*(px + 2));
	return ret;
}

int pf_png_close(void)
{
	png_image_free(&image);
	free(buffer);
	buffer = NULL;
	return 0;
}
