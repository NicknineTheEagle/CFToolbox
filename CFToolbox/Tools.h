#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "stdafx.h"
#include "gcf.h"

bool BrowseForFolder(HWND hOwner, char * title, CString& folderpath);
bool BrowsForFile(char filesFilter[] /* "All Files (*.*)|*.*||" */ ,bool isOpen,char * title, CString &m_path);
char * renderSize(signed __int64 size, char * buffer, bool isMoreThan4GB);
char * renderTime(DWORD time, char * buffer);
HICON getIconFromGCFTGA(GCF * gcf, char * iconpath);
int isRunningProcess(char * exe);


#endif