// Plugins.h: interface for the CPlugins class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUGINS_H__C5DB5156_8DB6_44B1_B3E0_BB2C2F3EA60D__INCLUDED_)
#define AFX_PLUGINS_H__C5DB5156_8DB6_44B1_B3E0_BB2C2F3EA60D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HashMap.h"
#include "dictionary.h"
#include "CFDescriptor.h"
#include "AppDescriptor.h"

  class CPlugin
{
public:
	char * name;
	char * description;
	char * version;
	char * menuCategory;
	char * menuItem;
	bool asynchronous;
	bool updatesFiles;
	bool closeCFToolbox;
	bool runOnlyOnceForMultipleSelection;

	char * gcfCommandLine;
	char * ncfCommandLine;
	char * gameCommandLine;
	char * mediaCommandLine;
	char * toolCommandLine;
	/*
	char * notInstalledGcfCommandLine;
	char * notInstalledNcfCommandLine;
	char * notInstalledGameCommandLine;
	char * notInstalledMediaCommandLine;
	char * notInstalledToolCommandLine;
*/
	CPlugin (dictionary * dic);
	~CPlugin();
	char * getCommandLine(CCFDescriptor * descriptor);
	char * getCommandLine(CAppDescriptor * descriptor);
};


class CPlugins  
{
public:
	CPlugins();
	virtual ~CPlugins();
	void buildMenu(CMenu * menu,UINT where,bool gcf, bool ncf, bool game, bool media, bool tool/*, bool installed*/);
	CPlugin * getPlugin(DWORD command);
private:
	HashMapDword * plugins;
	int loadPlugin(char * path,DWORD pid);
	int loadPlugins(char * folder);
};
CPlugins * getPlugins();
#endif // !defined(AFX_PLUGINS_H__C5DB5156_8DB6_44B1_B3E0_BB2C2F3EA60D__INCLUDED_)
