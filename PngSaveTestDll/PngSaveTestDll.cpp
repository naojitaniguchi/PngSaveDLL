// PngSaveTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// #include <unistd.h>
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
png_bytep * row_pointers;

// added for 16bit depth color
png_uint_16p * row_pointers_16bit;
png_structp png_ptr_16bit;
png_infop info_ptr_16bit;
png_bytep * row_pointers_2x;

void read_png_file(char* file_name)
{
	char header[8];    // 8 is the maximum size that can be checked

					   /* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("[read_png_file] File %s could not be opened for reading", file_name);
	fread(header, 1, 8, fp);
	if (png_sig_cmp((png_const_bytep)header, 0, 8))
		abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);


	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[read_png_file] png_create_read_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);


	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during read_image");

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for (y = 0; y<height; y++)
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));

	png_read_image(png_ptr, row_pointers);

	fclose(fp);
}


void write_png_file(char* file_name)
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
		bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, row_pointers);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	for (y = 0; y<height; y++)
		free(row_pointers[y]);
	free(row_pointers);

	fclose(fp);
}


void write_png_file_16bit(char* file_name)
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
		16, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, (png_bytep*)row_pointers_16bit);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	for (y = 0; y<height; y++)
		free(row_pointers[y]);
	free(row_pointers);

	fclose(fp);
}

void write_png_file_16bit_rgb(char* file_name)
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
		16, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, (png_bytep*)row_pointers_16bit);
	// png_save_uint_16 (png_bytep buf, unsigned int i);

	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	for (y = 0; y<height; y++)
		free(row_pointers[y]);
	free(row_pointers);

	fclose(fp);
}

void write_png_file_generated_16bit(char* file_name)
{
	width = 1920;
	height = 1920;

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

void process_file(void)
{
	if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
		abort_("[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
			"(lacks the alpha channel)");

	if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
		abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
			PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));

	for (y = 0; y<height; y++) {
		png_byte* row = row_pointers[y];
		for (x = 0; x<width; x++) {
			png_byte* ptr = &(row[x * 4]);
			printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
				x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

			/* set red value to 0 and green value to the blue one */
			ptr[0] = 0;
			ptr[1] = ptr[2];
		}
	}
}

void set_16bit_color(void) {

	/* initialize stuff */
	png_ptr_16bit = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr_16bit)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr_16bit = png_create_info_struct(png_ptr_16bit);
	if (!info_ptr_16bit)
		abort_("[write_png_file] png_create_info_struct failed");

	png_set_IHDR(png_ptr_16bit, info_ptr_16bit, width, height,
		16, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	row_pointers_16bit = (png_uint_16p *)malloc(sizeof(png_uint_16p) * height);
	for (y = 0; y<height; y++)
		row_pointers_16bit[y] = (png_uint_16 *)malloc(png_get_rowbytes(png_ptr_16bit, info_ptr_16bit));

	for (y = 0; y<height; y++) {
		png_byte* row_8bit = row_pointers[y];
		png_uint_16* row_16bit = row_pointers_16bit[y];
		for (x = 0; x < width; x++) {
			png_byte* ptr_8bit = &(row_8bit[x * 4]);
			png_uint_16* ptr_16bit = &(row_16bit[x * 4]);
			printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
				x, y, ptr_8bit[0], ptr_8bit[1], ptr_8bit[2], ptr_8bit[3]);

			for (int i = 0; i < 4; i++) {
				png_byte color_8bit = ptr_8bit[i];
				png_uint_16 color_16bit = (png_uint_16)color_8bit;
				ptr_16bit[i] = color_16bit;
			}
		}
	}

}

void set_16bit_color_rgb(void) {

	/* initialize stuff */
	png_ptr_16bit = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr_16bit)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr_16bit = png_create_info_struct(png_ptr_16bit);
	if (!info_ptr_16bit)
		abort_("[write_png_file] png_create_info_struct failed");

	png_set_IHDR(png_ptr_16bit, info_ptr_16bit, width, height,
		16, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	row_pointers_16bit = (png_uint_16p *)malloc(sizeof(png_uint_16p) * height);
	for (y = 0; y<height; y++)
		row_pointers_16bit[y] = (png_uint_16 *)malloc(png_get_rowbytes(png_ptr_16bit, info_ptr_16bit));

	for (y = 0; y<height; y++) {
		png_byte* row_8bit = row_pointers[y];
		png_uint_16* row_16bit = row_pointers_16bit[y];
		for (x = 0; x < width; x++) {
			png_byte* ptr_8bit = &(row_8bit[x * 4]);
			png_uint_16* ptr_16bit = &(row_16bit[x * 3]);
			printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
				x, y, ptr_8bit[0], ptr_8bit[1], ptr_8bit[2], ptr_8bit[3]);

			for (int i = 0; i < 3; i++) {
				png_byte color_8bit = ptr_8bit[i];
				png_uint_16 color_16bit = (png_uint_16)color_8bit;
				ptr_16bit[i] = color_16bit;
			}
		}
	}

}

void png_save_uint_16(png_bytep buf, unsigned int i)
{
	// buf[0] = (png_byte)((i >> 8) & 0xff);
	// buf[1] = (png_byte)(i & 0xff);

	buf[0] = 0xff;
	buf[1] = 0xff;
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

	unsigned short x_short;
	for (y = 0; y<height; y++) {
		png_byte* row = row_pointers_2x[y];
		for (x = 0; x < width; x++) {
			// r
			png_byte* ptr = &(row[x * 8]);
			png_save_uint_16(ptr, 1920);

			// g
			ptr = &(row[x * 8 + 2]);
			png_save_uint_16(ptr, 1920);

			// b
			ptr = &(row[x * 8 + 4]);
			png_save_uint_16(ptr, 0);

			// a
			ptr = &(row[x * 8 + 6]);
			png_save_uint_16(ptr, 65535);
		}
	}

}

int main(int argc, char **argv)
{
	//if (argc != 3)
	//	abort_("Usage: program_name <file_in> <file_out>");

	//read_png_file(argv[1]);

	//set_16bit_color();
	//write_png_file_16bit(argv[2]);

	generate_16bit_color();
	write_png_file_generated_16bit("out.png");

	//process_file();
	//write_png_file(argv[2]);

	return 0;
}

