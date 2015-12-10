// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PNGSAVEDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PNGSAVEDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PNGSAVEDLL_EXPORTS
#define PNGSAVEDLL_API __declspec(dllexport)
#else
#define PNGSAVEDLL_API __declspec(dllimport)
#endif

// This class is exported from the PngSaveDLL.dll
class PNGSAVEDLL_API CPngSaveDLL {
public:
	CPngSaveDLL(void);
	// TODO: add your methods here.
};

extern PNGSAVEDLL_API int nPngSaveDLL;

PNGSAVEDLL_API int fnPngSaveDLL(void);

extern "C" {
	PNGSAVEDLL_API int CountUp(void);
	PNGSAVEDLL_API int SaveTestPng(void);
	PNGSAVEDLL_API int Save16BitPng(int image_width, int image_height, unsigned short *image_buffer, char *path);
}
