// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PNGSAVEDLLDX11_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PNGSAVEDLLDX11_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#include <D3D11.h>
#include "Unity/IUnityInterface.h"

#ifdef PNGSAVEDLLDX11_EXPORTS
#define PNGSAVEDLLDX11_API __declspec(dllexport)
#else
#define PNGSAVEDLLDX11_API __declspec(dllimport)
#endif

// This class is exported from the PngSaveDLLDx11.dll
class PNGSAVEDLLDX11_API CPngSaveDLLDx11 {
public:
	CPngSaveDLLDx11(void);
	// TODO: add your methods here.
};

extern PNGSAVEDLLDX11_API int nPngSaveDLLDx11;

PNGSAVEDLLDX11_API int fnPngSaveDLLDx11(void);

extern "C" {
	PNGSAVEDLLDX11_API int Save16BitPng(int image_width, int image_height, char *image_buffer, char *path);
	PNGSAVEDLLDX11_API int Save16BitPngFromDXTexture(int image_width, int image_height, ID3D11Resource *pResWorld, ID3D11Resource *pResMask, char *path,
		float xRangeMin, float xRangeMax, float yRangeMin, float yRangeMax, float zRangeMin, float zRangeMax);
	PNGSAVEDLLDX11_API int Save16BitPngFromDXColorTexture(int image_width, int image_height, ID3D11Resource *pResColor, char *path);
}
