#ifndef _LARGEFILE_H_
#define _LARGEFILE_H_
#include <windows.h>

// new in 1.0.12 : read buffer


#define I64BUFSIZE 100000

typedef struct 
{
	HANDLE handle;
	BYTE buffer[I64BUFSIZE];
	DWORD size;
	DWORD position;
} BUFFEREDHANDLE_STRUCT;


typedef DWORD BUFFEREDHANDLE;
 
// new in 1.0.7 : large file support (4GB maxi)
BUFFEREDHANDLE fopen_64(char * fileName,char * mode);
void fclose_64(BUFFEREDHANDLE handle);
DWORD fseek_64(BUFFEREDHANDLE handle, _int64 offset, int origin );
DWORD fread_64(void *,DWORD size,DWORD count,BUFFEREDHANDLE handle);
DWORD fwrite_64(void *,DWORD size,DWORD count,BUFFEREDHANDLE handle);
void rewind_64(BUFFEREDHANDLE handle);
void fflush_64(BUFFEREDHANDLE handle);
DWORD ftell_64(BUFFEREDHANDLE handle);

ULONGLONG fileSize64(char * path);


#endif