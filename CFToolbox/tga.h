

#ifndef _TGA_H_
#define _TGA_H_
#ifndef BYTE
#include "windows.h"
#endif
typedef struct {
	DWORD width;
	DWORD height;
	BYTE * pixels;
} TGA;
TGA * parseTGA (char * buffer, long length);
#endif // _TGA_H_