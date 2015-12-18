// PngSaveDLLDx11.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PngSaveDLLDx11.h"
#include <D3D11.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <algorithm>

#include "Unity/IUnityGraphics.h"
#include "Unity/IUnityGraphicsD3D11.h"

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

// for log file
FILE *log_fp;

// --------------------------------------------------------------------------
// UnitySetInterfaces



static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;
static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;
static void* g_TexturePointer = NULL;

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

// --------------------------------------------------------------------------
// GraphicsDeviceEvent

static void DoEventGraphicsDeviceD3D11(UnityGfxDeviceEventType eventType);

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	UnityGfxRenderer currentDeviceType = s_DeviceType;

	switch (eventType)
	{
	case kUnityGfxDeviceEventInitialize:
	{
		s_DeviceType = s_Graphics->GetRenderer();
		currentDeviceType = s_DeviceType;
		break;
	}

	case kUnityGfxDeviceEventShutdown:
	{
		s_DeviceType = kUnityGfxRendererNull;
		g_TexturePointer = NULL;
		break;
	}

	case kUnityGfxDeviceEventBeforeReset:
	{
		break;
	}

	case kUnityGfxDeviceEventAfterReset:
	{
		break;
	}
	};

	DoEventGraphicsDeviceD3D11(eventType);
}

static ID3D11Device* g_D3D11Device = NULL;

static void DoEventGraphicsDeviceD3D11(UnityGfxDeviceEventType eventType)
{
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		IUnityGraphicsD3D11* d3d11 = s_UnityInterfaces->Get<IUnityGraphicsD3D11>();
		g_D3D11Device = d3d11->GetDevice();

		//EnsureD3D11ResourcesAreCreated();
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		//ReleaseD3D11Resources();
	}
}


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

void generate_16bit_png_data(int image_width, int image_hight, char *image_buf) {

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


	row_pointers_2x = (png_bytep *)malloc(sizeof(png_bytep) * image_hight);
	for (int y = 0; y<image_hight; y++)
		row_pointers_2x[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr_16bit, info_ptr_16bit));

	for (int y = 0; y<image_hight; y++) {
		png_byte* row = row_pointers_2x[y];
		for (int x = 0; x < image_width; x++) {
			int pixel_pos = y * (image_width * 8) + x * 8;
			//int pixel_pos = y * (image_width * 4) + x * 4;

			for (int i = 0; i < 4; i++) {
				png_byte* ptr = &(row[x * 8 + i * 2]);
				ptr[0] = image_buf[pixel_pos + i * 2];
				ptr[1] = image_buf[pixel_pos + i * 2 + 1];
			}
		}
	}

}

void generate_16bit_png_data_from_float(int image_width, int image_hight, float *image_buf) {

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


	row_pointers_2x = (png_bytep *)malloc(sizeof(png_bytep) * image_hight);
	for (int y = 0; y<image_hight; y++)
		row_pointers_2x[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr_16bit, info_ptr_16bit));

	for (int y = 0; y<image_hight; y++) {
		png_byte* row = row_pointers_2x[y];
		for (int x = 0; x < image_width; x++) {
			int pixel_pos = y * (image_width * 4) + x * 4;

			for (int i = 0; i < 4; i++) {
				png_byte* ptr = &(row[x * 8 + i * 2]);
				unsigned int v = (unsigned int)image_buf[pixel_pos + i];
				if (i == 3) {
					if (image_buf[pixel_pos + i] > 0.5f) {
						v = 65535;
					}
				}
				png_save_uint_16(ptr, v);
			}
		}
	}

}

int dump_buf(char *filename, int image_width, int image_height, char *image_buffer)
{
	FILE *fp;
	int i;

	if ((fp = fopen(filename, "w")) == NULL) {
		printf("ファイルオープンエラー\n");
		exit(EXIT_FAILURE);
	}

	int length = image_width * image_height * 8;
	for (int i = 0; i < length; i++) {
		fprintf(fp, "%d\n", image_buffer[i]);
	}

	/* ファイルクローズ */
	fclose(fp);


	return 0;
}

int dump_png_buf(char *filename, int image_width, int image_height)
{
	FILE *fp;
	int i;

	if ((fp = fopen(filename, "w")) == NULL) {
		printf("ファイルオープンエラー\n");
		exit(EXIT_FAILURE);
	}

	for (int y = 0; y<image_height; y++) {
		png_byte* row = row_pointers_2x[y];
		for (int x = 0; x < image_width * 8; x++) {
			fprintf(fp, "%d\n", row[x]);
		}
	}

	/* ファイルクローズ */
	fclose(fp);


	return 0;
}

int open_log(char *filename)
{
	if ((log_fp = fopen(filename, "w")) == NULL) {
		printf("ファイルオープンエラー\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int close_log()
{

	/* ファイルクローズ */
	fclose(log_fp);

	return 0;
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

	fclose(fp);
}

void cleanHeap(int image_height) {

	/* cleanup heap allocation */
	for (y = 0; y<image_height; y++)
		free(row_pointers_2x[y]);
	free(row_pointers_2x);

}


// This is an example of an exported variable
PNGSAVEDLLDX11_API int nPngSaveDLLDx11=0;

// This is an example of an exported function.
PNGSAVEDLLDX11_API int fnPngSaveDLLDx11(void)
{
    return 42;
}

PNGSAVEDLLDX11_API int Save16BitPng(int image_width, int image_height, char *image_buffer, char *path)
{
	// dump_buf("image_buf_dump.txt", image_width, image_height, image_buffer);

	generate_16bit_png_data(image_width, image_height, image_buffer);

	// dump_png_buf("png_buf_dump.txt", image_width, image_height);

	write_png_file_16bit(path);
	cleanHeap(image_height);

	return 1;
}

float *getFloatBufferFromTexture(ID3D11Resource *pRes) {

	ID3D11Texture2D* m_pTexture = static_cast<ID3D11Texture2D*>(pRes);

	D3D11_TEXTURE2D_DESC desc;
	m_pTexture->GetDesc(&desc);

	fprintf(log_fp, "Width %d\n", desc.Width);
	fprintf(log_fp, "Width %d\n", desc.Height);
	fprintf(log_fp, "MipLevels %d\n", desc.MipLevels);
	fprintf(log_fp, "ArraySize %d\n", desc.ArraySize);
	fprintf(log_fp, "Format %d\n", desc.Format);

	ID3D11Texture2D* pNewTexture = NULL;
	D3D11_TEXTURE2D_DESC description;
	m_pTexture->GetDesc(&description);

	description.BindFlags = 0;

	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	description.Usage = D3D11_USAGE_STAGING;
	description.MipLevels = 1;
	HRESULT hr = g_D3D11Device->CreateTexture2D(&description, NULL, &pNewTexture);

	ID3D11DeviceContext* ctx = NULL;
	g_D3D11Device->GetImmediateContext(&ctx);

	ctx->CopyResource(pNewTexture, m_pTexture);

	D3D11_MAPPED_SUBRESOURCE resource;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	ctx->Map(pNewTexture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

	float *sptr = NULL ;
	float *dptr = NULL ;
	float *dptrTop = NULL;

	// RGBA
	if (desc.Format == DXGI_FORMAT_R32G32B32A32_TYPELESS) {
		sptr = reinterpret_cast<float*>(resource.pData);
		dptr = new float[desc.Width*desc.Height * 4];
		dptrTop = dptr;
		
		for (size_t h = 0; h < desc.Height; ++h)
		{
			memcpy(dptr, sptr, desc.Width * 4 * sizeof(float));
			dptr += desc.Width * 4 ;
			sptr += resource.RowPitch / 4 ;
		}

	}
	else if (desc.Format == DXGI_FORMAT_R32_TYPELESS) {
		sptr = reinterpret_cast<float*>(resource.pData);
		dptr = new float[desc.Width*desc.Height];
		dptrTop = dptr;
		for (size_t h = 0; h < desc.Height; ++h)
		{
			memcpy(dptr, sptr, desc.Width * sizeof(float));
			dptr += desc.Width;
			sptr += resource.RowPitch / 4 ;
		}
	}

	pNewTexture->Release();

	return dptrTop;
}

void	dumpBuffer(int image_width, int image_height, float *worldPtr, float *maskPtr) {
	fprintf(log_fp, "------ worldPtr -------\n");
	for (int i = 0; i < image_width * image_height * 4; i++) {
		fprintf(log_fp, "i %f\n", worldPtr[i]);
	}

	fprintf(log_fp, "------ mask Ptr -------\n");
	for (int i = 0; i < image_width * image_height; i++) {
		fprintf(log_fp, "i %f\n", maskPtr[i]);
	}
}

void	addMaskToWorld(int image_width, int image_height, float *worldPtr, float *maskPtr) {
	for (int i = 0; i < image_width * image_height ; i++) {
		int index = i * 4;
		worldPtr[index + 3] = maskPtr[i];
	}
}

void	setWorldToRange(int image_width, int image_height, float *worldPtr, float xRangeMin, float xRangeMax, float yRangeMin, float yRangeMax, float zRangeMin, float zRangeMax) {
	float xMin = 99999999999.9;
	float xMax = -99999999999.9;
	float yMin = 99999999999.9;
	float yMax = -99999999999.9;
	float zMin = 99999999999.9;
	float zMax = -99999999999.9;

	for (int i = 0; i < image_width * image_height; i++) {
		int index = i * 4;

		// check mask
		if (worldPtr[index + 3] > 0.9f) {
			if (worldPtr[index] < xMin) {
				xMin = worldPtr[index];
			}
			if (worldPtr[index] > xMax) {
				xMax = worldPtr[index];
			}
			if (worldPtr[index + 1] < yMin) {
				yMin = worldPtr[index];
			}
			if (worldPtr[index + 1] > yMax) {
				yMax = worldPtr[index];
			}
			if (worldPtr[index + 2] < zMin) {
				zMin = worldPtr[index];
			}
			if (worldPtr[index + 2] > zMax) {
				zMax = worldPtr[index];
			}
		}
	}

	float xOriginalRange = xMax - xMin;
	float yOriginalRange = yMax - yMin;
	float zOriginalRange = zMax - zMin;

	float xRate = (xRangeMax - xRangeMin) / xOriginalRange;
	float yRate = (yRangeMax - yRangeMin) / yOriginalRange;
	float zRate = (zRangeMax - zRangeMin) / zOriginalRange;

	for (int i = 0; i < image_width * image_height; i++) {
		int index = i * 4;
		float x = ( worldPtr[index] - xMin ) * xRate ;
		worldPtr[index] = x;
		float y = (worldPtr[index+1] - yMin) * yRate;
		worldPtr[index + 1 ] = y;
		float z = (worldPtr[index + 2] - zMin) * zRate;
		worldPtr[index + 2] = z;
	}

}

PNGSAVEDLLDX11_API int Save16BitPngFromDXTexture(int image_width, int image_height, ID3D11Resource *pResWorld, ID3D11Resource *pResMask, char *path,
	float xRangeMin, float xRangeMax, float yRangeMin, float yRangeMax, float zRangeMin, float zRangeMax )
{
	// open_log("log_file.txt");

	// fprintf(log_fp, "Save16BitPngFromDXTexture called\n");

	float *worldPtr = getFloatBufferFromTexture(pResWorld);
	float *maskPtr = getFloatBufferFromTexture(pResMask);

	addMaskToWorld(image_width, image_height, worldPtr, maskPtr);
	setWorldToRange(image_width, image_height, worldPtr, xRangeMin, xRangeMax, yRangeMin, yRangeMax, zRangeMin, zRangeMax);

	// close_log();

	// clamp_data

	generate_16bit_png_data_from_float(image_width, image_height, worldPtr);
	write_png_file_16bit(path);
	cleanHeap(image_height);

	return 1;
}


// This is the constructor of a class that has been exported.
// see PngSaveDLLDx11.h for the class definition
CPngSaveDLLDx11::CPngSaveDLLDx11()
{
    return;
}
