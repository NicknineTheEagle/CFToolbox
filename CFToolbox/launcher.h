#ifndef _LAUNCHER_H_
#define _LAUNCHER_H_
#include "CFDescriptor.h"
#include "AppDescriptor.h"
#include "SocketTools.h"
int cutCommandLine(char * commandLine, char ** params);
bool buildAppCommandLine(char * commandLine, char * commandLineTemplate,DWORD appId,CAppDescriptor * desc=0);
bool buildFileCommandLine(char * commandLine, char * commandLineTemplate,DWORD fileId,CCFDescriptor * desc=0);

void launch(DWORD id,int mode, bool isApp=false);
void launch(CCFDescriptor * descriptor,int mode);
void launch(CAppDescriptor * descriptor,int mode);
#endif;