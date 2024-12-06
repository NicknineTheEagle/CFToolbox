// AppDescriptor.h: interface for the CAppDescriptor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APPDESCRIPTOR_H__4394057D_E947_4C40_BF74_55460891D38C__INCLUDED_)
#define AFX_APPDESCRIPTOR_H__4394057D_E947_4C40_BF74_55460891D38C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "cdr.h"
#include "HashMap.h"
#include "Task.h"
class CAppDescriptor  
{
	
public:
	void update();
	int iconId;
	DWORD appId;
	CAppDescriptor(ContentDescriptionRecord cdr,CDRApp app, HashMapDword * files,DWORD * _appGcfIds,char * _appGcfOptional,int _appGcfIdsCount);
	virtual ~CAppDescriptor();
	CTask * runningTask;
	char information[500];
	double completion;
	ULONGLONG realSize;
	char * appName;
	char * commonPath;
	char * manual;
	char * editor;
	DWORD * appGcfIds;
	char * appGcfOptional;
	int appGcfIdsCount;
	char ** appGcfNames;
	DWORD * appGcfSizes;
	char ** propertiesNames;
	char ** propertiesValues;
	int nbProperties;
	CAppDescriptor();
	char type[100];

};

#endif // !defined(AFX_APPDESCRIPTOR_H__4394057D_E947_4C40_BF74_55460891D38C__INCLUDED_)
