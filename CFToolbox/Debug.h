#ifndef _debug_h_
#define _debug_h_


#define GLOBAL_DEBUG_LEVEL
#define _DEBUG_AFX_MODE_

#define DEBUG_LEVEL_EVERYTHING 0
#define DEBUG_LEVEL_DEBUG 1
#define DEBUG_LEVEL_INFO 2
#define DEBUG_LEVEL_ERROR 3
#define DEBUG_LEVEL_TODO 4
#define DEBUG_LEVEL_DISABLED 5

#ifdef _DEBUG_AFX_MODE_


#ifdef GLOBAL_DEBUG_LEVEL
void setGlobalLogLevel(int l);
#endif

#include "stdafx.h"
int initDebug(char * newLogFile,CEdit * control);
#else
//#include "windows.h"
int initDebug(char * newLogFile,int logStdout);
#endif
char * getDateTime(char * buff);

void closeDebug();
void closeDebugDialog();
void printDebug(int level, char * who, char * what, int desiredLevel);

void printDebug(int level, char * who, char * what, ULONGLONG value, int desiredLevel);

void printDebug(int level, char * who, char * what, char * str, int desiredLevel);

void printDebug(int level, char * who, char * what, char * data,int len, int desiredLevel);

#endif
