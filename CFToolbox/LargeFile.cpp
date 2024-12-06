#include "LargeFile.h"
#include <windows.h>
#include <sys/stat.h>


/*

#define DEBUG_LEVEL_EVERYTHING 0
#define DEBUG_LEVEL_DEBUG 1
#define DEBUG_LEVEL_INFO 2
#define DEBUG_LEVEL_ERROR 3
#define DEBUG_LEVEL_TODO 4
#define DEBUG_LEVEL_DISABLED 5

void printDebug(int level, char * who, char * what, int desiredLevel);

void printDebug(int level, char * who, char * what, ULONGLONG value, int desiredLevel);

void printDebug(int level, char * who, char * what, char * str, int desiredLevel);

void printDebug(int level, char * who, char * what, char * data,int len, int desiredLevel);

*/


BUFFEREDHANDLE fopen_64(char * fileName,char * mode)
{
//	printDebug(DEBUG_LEVEL_DEBUG,"io","Opening",fileName,DEBUG_LEVEL_DEBUG);
//	printDebug(DEBUG_LEVEL_DEBUG,"io"," > mode",mode,DEBUG_LEVEL_DEBUG);
	DWORD access=0;
	DWORD creat=OPEN_EXISTING;
	if (strchr(mode,'w') || strchr(mode,'+')) 
	{
		access=access|GENERIC_WRITE;
		creat=OPEN_ALWAYS;
	}
	if (strchr(mode,'r')) access=access|=GENERIC_READ;

	HANDLE fileHandle=INVALID_HANDLE_VALUE;
	fileHandle=CreateFile(fileName,access,FILE_SHARE_READ | FILE_SHARE_WRITE ,NULL,creat,0,NULL);
	if (fileHandle==INVALID_HANDLE_VALUE) 
	{
//		printDebug(DEBUG_LEVEL_DEBUG,"io","  > ERROR",GetLastError(),DEBUG_LEVEL_DEBUG);
		return 0;	
	}
//	printDebug(DEBUG_LEVEL_DEBUG,"io","  > OK",(int)fileHandle,DEBUG_LEVEL_DEBUG);

	// return fileHandle;

	BUFFEREDHANDLE_STRUCT * bHandle=(BUFFEREDHANDLE_STRUCT* )malloc(sizeof(BUFFEREDHANDLE_STRUCT));
	memset(bHandle,0,sizeof(BUFFEREDHANDLE_STRUCT));
	bHandle->handle=fileHandle;
	
	return (DWORD)bHandle;
}

void fclose_64(BUFFEREDHANDLE _bHandle)
{
	BUFFEREDHANDLE_STRUCT * bHandle=(BUFFEREDHANDLE_STRUCT*)_bHandle;

//	printDebug(DEBUG_LEVEL_DEBUG,"io","Closing",(int)handle,DEBUG_LEVEL_DEBUG);
	CloseHandle(bHandle->handle);
	free(bHandle);
}

DWORD fseek_64(BUFFEREDHANDLE _bHandle, _int64 offset, int origin )
{
	BUFFEREDHANDLE_STRUCT * bHandle=(BUFFEREDHANDLE_STRUCT*)_bHandle;
	switch (origin) 
	{
	case SEEK_END:origin=FILE_END;break;
	case SEEK_CUR:offset-=(bHandle->size-bHandle->position);origin=FILE_CURRENT;break;
	case SEEK_SET:origin=FILE_BEGIN;break;
	default : return 0;
	}

	LARGE_INTEGER li;
	li.QuadPart=offset;
	li.LowPart=SetFilePointer(bHandle->handle,li.LowPart,&li.HighPart,origin);
	bHandle->size=0;
	bHandle->position=0;
	return (DWORD)li.QuadPart;
}

DWORD fread_64(void *_b ,DWORD size,DWORD count,BUFFEREDHANDLE _bHandle)
{
	BUFFEREDHANDLE_STRUCT * bHandle=(BUFFEREDHANDLE_STRUCT*)_bHandle;
	BYTE * b=(BYTE*)_b;

	DWORD avail=bHandle->size-bHandle->position;
	
	if (!avail)
	{
		DWORD readSize=0;
		bool ok=(ReadFile(bHandle->handle,bHandle->buffer,I64BUFSIZE,&readSize,NULL)!=0);
		if (!ok) return 0;

		bHandle->size=readSize;
		bHandle->position=0;
		avail=bHandle->size-bHandle->position;
	}

	DWORD availCount=avail/size;

	if (availCount)
	{
		DWORD toCopy=(count>availCount ? availCount : count);
		DWORD copied=toCopy*size;

		memcpy(b,bHandle->buffer+bHandle->position,copied);
		bHandle->position+=copied;
		

		if (toCopy<count) copied+=fread_64(b+copied,size,count-toCopy,_bHandle);

		return copied;
	}
	else
	{
		DWORD copied=avail;

		memcpy(b,bHandle->buffer+bHandle->position,avail);
		bHandle->position=0;
		bHandle->size=0;
		DWORD rest=size*count-avail;

		DWORD readSize=0;
		bool ok=(ReadFile(bHandle->handle,b+copied,rest,&readSize,NULL)!=0);
		if (ok) copied+=readSize;

		return copied;
	}
}

DWORD fwrite_64(void *b,DWORD size,DWORD count,BUFFEREDHANDLE _bHandle)
{
	BUFFEREDHANDLE_STRUCT * bHandle=(BUFFEREDHANDLE_STRUCT*)_bHandle;
	if (bHandle->size-bHandle->position) fseek_64(_bHandle,-(bHandle->size-bHandle->position),SEEK_CUR);

	DWORD res=0;
	bool ok=(WriteFile(bHandle->handle,b,size*count,&res,NULL)!=0);
	if (!ok) return 0;
	return res;
}

void rewind_64(BUFFEREDHANDLE handle)
{
	fseek_64(handle,0,SEEK_SET);
}

void fflush_64(BUFFEREDHANDLE _bHandle)
{
	BUFFEREDHANDLE_STRUCT * bHandle=(BUFFEREDHANDLE_STRUCT*)_bHandle;
	FlushFileBuffers(bHandle->handle);
}

DWORD ftell_64(BUFFEREDHANDLE handle)
{

	return fseek_64(handle,0,SEEK_CUR);
}

ULONGLONG fileSize64(char * path)
{
	struct _stati64 stats;
	if (_stati64( path,&stats))
	{
		return 0;
	}
	else
	{
		return stats.st_size;
	}
}
