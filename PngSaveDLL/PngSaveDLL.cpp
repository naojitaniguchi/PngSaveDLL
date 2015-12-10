// PngSaveDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PngSaveDLL.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

int x, y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;


// added for 16bit depth color
png_uint_16p * row_pointers_16bit;
png_structp png_ptr_16bit;
png_infop info_ptr_16bit;
png_bytep * row_pointers_2x;

void png_save_uint_16(png_bytep buf, unsigned int i)
{
	buf[0] = (png_byte)((i >> 8) & 0xff);
	buf[1] = (png_byte)(i & 0xff);
}

void generate_16bit_color(void) {

	width = 1920;
	height = 1920;
	/* initialize stuff */
	png_ptr_16bit = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr_16bit)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr_16bit = png_create_info_struct(png_ptr_16bit);
	if (!info_ptr_16bit)
		abort_("[write_png_file] png_create_info_struct failed");

	png_set_IHDR(png_ptr_16bit, info_ptr_16bit, width, height,
		16, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


	int line_length = png_get_rowbytes(png_ptr_16bit, info_ptr_16bit);

	row_pointers_2x = (png_bytep *)malloc(sizeof(png_uint_16p) * height);
	for (y = 0; y<height; y++)
		row_pointers_2x[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr_16bit, info_ptr_16bit));

	float x_rate = 65535 / width;
	float y_rate = 65535 / height;

	unsigned short x_short;
	for (y = 0; y<height; y++) {
		png_byte* row = row_pointers_2x[y];
		for (x = 0; x < width; x++) {
			// r
			png_byte* ptr = &(row[x * 8]);
			png_save_uint_16(ptr, (unsigned int)((float)x * x_rate));

			// g
			ptr = &(row[x * 8 + 2]);
			png_save_uint_16(ptr, (unsigned int)((float)y * y_rate));

			// b
			ptr = &(row[x * 8 + 4]);
			png_save_uint_16(ptr, 0);

			// a
			ptr = &(row[x * 8 + 6]);
			png_save_uint_16(ptr, 65535);
		}
	}

}

void write_png_file_generated_16bit(char* file_name)
{

	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		abort_("[write_png_file] File %s could not be opened for writing", file_name);


	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[write_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing header");

	png_set_IHDR(png_ptr, info_ptr, width, height,
		16, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// png_set_swap(png_ptr);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, row_pointers_2x);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	for (y = 0; y<height; y++)
		free(row_pointers_2x[y]);
	free(row_pointers_2x);

	fclose(fp);
}

void generate_16bit_png_data(int image_width, int image_hight, unsigned short *image_buf) {

	/* initialize stuff */
	png_ptr_16bit = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr_16bit)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr_16bit = png_create_info_struct(png_ptr_16bit);
	if (!info_ptr_16bit)
		abort_("[write_png_file] png_create_info_struct failed");

	png_set_IHDR(png_ptr_16bit, info_ptr_16bit, image_width, image_hight,
		16, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


	row_pointers_2x = (png_bytep *)malloc(sizeof(png_uint_16p) * height);
	for (y = 0; y<height; y++)
		row_pointers_2x[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr_16bit, info_ptr_16bit));

	unsigned short x_short;
	for (y = 0; y<height; y++) {
		png_byte* row = row_pointers_2x[y];
		for (x = 0; x < width; x++) {
			int pixel_pos = y * (image_width * 4) + x * 4;

			// r
			unsigned short color = image_buf[pixel_pos];
			png_byte* ptr = &(row[x * 8]);
			png_save_uint_16(ptr, color);

			// g
			color = image_buf[pixel_pos+1];
			ptr = &(row[x * 8 + 2]);
			png_save_uint_16(ptr, color);

			// b
			color = image_buf[pixel_pos + 2];
			ptr = &(row[x * 8 + 4]);
			png_save_uint_16(ptr, color);

			// a
			color = image_buf[pixel_pos + 3];
			ptr = &(row[x * 8 + 6]);
			png_save_uint_16(ptr, color);
		}
	}

}

void write_png_file_16bit(char* file_name)
{

	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		abort_("[write_png_file] File %s could not be opened for writing", file_name);


	png_init_io(png_ptr_16bit, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png_ptr_16bit)))
		abort_("[write_png_file] Error during writing header");

	png_write_info(png_ptr_16bit, info_ptr_16bit);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr_16bit)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr_16bit, row_pointers_2x);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr_16bit)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr_16bit, NULL);

	/* cleanup heap allocation */
	for (y = 0; y<height; y++)
		free(row_pointers_2x[y]);
	free(row_pointers_2x);

	fclose(fp);
}

// This is an example of an exported variable
PNGSAVEDLL_API int nPngSaveDLL=0;

// This is an example of an exported function.
PNGSAVEDLL_API int fnPngSaveDLL(void)
{
    return 42;
}

int counter = 0;
PNGSAVEDLL_API int CountUp(void)
{
	return counter++;
}

PNGSAVEDLL_API int SaveTestPng(void)
{
	generate_16bit_color();
	write_png_file_generated_16bit("out.png");

	return 1;
}

PNGSAVEDLL_API int Save16BitPng(int image_width, int image_height, unsigned short *image_buffer, char *path)
{
	generate_16bit_png_data(image_width, image_height, image_buffer);
	write_png_file_16bit(path);

	return 1;
}

// This is the constructor of a class that has been exported.
// see PngSaveDLL.h for the class definition
CPngSaveDLL::CPngSaveDLL()
{
    return;
}
