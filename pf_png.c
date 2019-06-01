#define _POSIX_C_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <png.h>

#include "pf_png.h"

static png_structp png_ptr;
static png_infop info_ptr;
static png_bytepp row_pointers;
static png_uint_32 width, height;
static FILE *fp;

int pf_png_open(const char *path) {
	fp = fopen(path, "rb");
	if (!fp) {
		return 2;
	}

	/* a PNG header should be 8 bytes long */
	uint8_t header[8] = { 0 };
	if (fread(header, 1, 8, fp) != 8) {
		return 3;
	}
	if (png_sig_cmp(header, 0, 8)) {
		return 4;
	}

	png_ptr = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		NULL,
		NULL,
		NULL
		);

	if (!png_ptr) {
		return 5;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr,
					(png_infopp)NULL, (png_infopp)NULL);
		return 6;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return 7;
	}

	rewind(fp);
	png_init_io(png_ptr, fp);

	return 0;
}

int pf_png_read(void) {
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return -1;
	}
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_SCALE_16 | PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_PACKING, NULL);

	row_pointers = png_get_rows(png_ptr, info_ptr);
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	fclose(fp);
	fp = NULL;

	return 0;
}

int pf_png_close(void) {
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return -1;
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return 0;
}

uint32_t pf_png_get_rgb(uint32_t x, uint32_t y) {
	png_bytep px = &(row_pointers[y][x * 4]);
	png_byte r = px[0];
	png_byte g = px[1];
	png_byte b = px[2];
	return r << 16 | g << 8 | b;
}
