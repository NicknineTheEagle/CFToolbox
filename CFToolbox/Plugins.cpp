// Plugins.cpp: implementation of the CPlugins class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Plugins.h"
#include "io.h"
#include "string.h"
#include "stdio.h"
#include "iniparser.h"
#include "debug.h"
#include "resource.h"
#include "TabPreferences.h"
#include "ConfigManager.h"

int addString(CComboBox * combo,char * string, DWORD value)
{
	int index=combo->FindStringExact(0,string);
	if (index!=CB_ERR)
	{
		char error[500];
		sprintf(error,"Conflict for %s",string);
		MessageBox(NULL,error,"CF Toolbox",MB_ICONERROR);
		return index;
	}
	combo->AddString(string);
	 
	index=combo->FindStringExact(0,string);
	combo->SetItemData(index,value);
	return index;
}

char * copyStr(char * from)
{
	if (!from) return 0;
	if (!strlen(from)) return 0;
	char * v=(char*)malloc(strlen(from)+1);
	strcpy(v,from);
	return v;
}

	CPlugin::CPlugin (dictionary * dic)
	{
		name=copyStr(iniparser_getstr(dic, "cftoolbox/plugin:name"));
		description=copyStr(iniparser_getstr(dic, "cftoolbox/plugin:description"));
		version=copyStr(iniparser_getstr(dic, "cftoolbox/plugin:version"));
		menuCategory=copyStr(iniparser_getstr(dic, "cftoolbox/plugin:menuCategory"));
		menuItem=copyStr(iniparser_getstr(dic, "cftoolbox/plugin:menuItem"));

		gcfCommandLine=copyStr(iniparser_getstr(dic, "gcf:commandLine"));
		ncfCommandLine=copyStr(iniparser_getstr(dic, "ncf:commandLine"));
		gameCommandLine=copyStr(iniparser_getstr(dic, "game:commandLine"));
		mediaCommandLine=copyStr(iniparser_getstr(dic, "media:commandLine"));
		toolCommandLine=copyStr(iniparser_getstr(dic, "tool:commandLine"));
		/*notInstalledGcfCommandLine=copyStr(iniparser_getstr(dic, "gcf/notInstalled:commandLine"));
		notInstalledNcfCommandLine=copyStr(iniparser_getstr(dic, "ncf/notInstalled:commandLine"));
		notInstalledGameCommandLine=copyStr(iniparser_getstr(dic, "game/notInstalled:commandLine"));
		notInstalledMediaCommandLine=copyStr(iniparser_getstr(dic, "media/notInstalled:commandLine"));
		notInstalledToolCommandLine=copyStr(iniparser_getstr(dic, "tool/notInstalled:commandLine"));
		*/
		asynchronous=iniparser_getboolean(dic, "cftoolbox/plugin:asynchronous",false);
		updatesFiles=iniparser_getboolean(dic, "cftoolbox/plugin:updatesFiles",false);
		closeCFToolbox=iniparser_getboolean(dic, "cftoolbox/plugin:closeCFToolbox",false);
		runOnlyOnceForMultipleSelection=iniparser_getboolean(dic, "cftoolbox/plugin:runOnlyOnceForMultipleSelection",false);

		if (!menuItem) menuItem=copyStr(name);
		printDebug(DEBUG_LEVEL_DEBUG,"CF Toolbox","Loaded plugin",name,0);
	}
	CPlugin::~CPlugin()
	{
		if (name) free(name);
		if (description) free(description);
		if (version) free(version);
		if (menuItem) free(menuItem);
		
		
		if (gcfCommandLine) free(gcfCommandLine);
		if (ncfCommandLine) free(ncfCommandLine);
		if (gameCommandLine) free(gameCommandLine);
		if (mediaCommandLine) free(mediaCommandLine);
		if (toolCommandLine) free(toolCommandLine);
		/*
		if (notInstalledGcfCommandLine) free(notInstalledGcfCommandLine);
		if (notInstalledNcfCommandLine) free(notInstalledNcfCommandLine);
		if (notInstalledGameCommandLine) free(notInstalledGameCommandLine);
		if (notInstalledMediaCommandLine) free(notInstalledMediaCommandLine);
		if (notInstalledToolCommandLine) free(notInstalledToolCommandLine);
		*/
	}


char * CPlugin::getCommandLine(CCFDescriptor * descriptor)
{
	if (!descriptor) return 0;
	if (descriptor->isNcf) return ncfCommandLine;
	return gcfCommandLine;
}

char * CPlugin::getCommandLine(CAppDescriptor * descriptor)
{
	if (!descriptor) return 0;
	if (!stricmp(descriptor->type,"tool")) return toolCommandLine;
	if (!stricmp(descriptor->type,"movie")) return mediaCommandLine;
	return gameCommandLine;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlugins * thePlugins=0;

CPlugins * getPlugins()
{
	if (!thePlugins) thePlugins=new CPlugins();
	return thePlugins;
}

CPlugins::CPlugins()
{
	plugins=new HashMapDword(HASHMAP_DELETE);
 
	loadPlugins("plugins");
}

CPlugins::~CPlugins()
{
 
	delete plugins;	
}

int CPlugins::loadPlugins(char * folder)
{

	CComboBox * filesDbl=&(getTabPreferences()->m_filesDoubleClick);
	CComboBox * appsDbl=&(getTabPreferences()->m_appsDoubleClick);

	addString(filesDbl,"Open file",0);
	filesDbl->SetCurSel(0);
	addString(appsDbl,"Execute batch command",0);
	appsDbl->SetCurSel(0);

	DWORD pid=0x1001;
	int nbLoaded=0;
	char path[1000];
	char filter[1000];
	sprintf(filter,"%s\\*.ini",folder);
	_finddata_t fileData;
	long id=_findfirst(filter,&fileData);
	if (id>=0)
	{
		do
		{
			
			sprintf(path,"%s\\%s",folder,fileData.name);
			nbLoaded+=loadPlugin(path,pid+nbLoaded);
			
		} while (!_findnext(id,&fileData));
		_findclose(id);
	}
	return nbLoaded;
}

int getInsertPos(HMENU menu,char * item)
{
	int nb=GetMenuItemCount(menu);
	char str[200];
	int pos;
	for (pos=0;pos<nb;pos++)
	{
		GetMenuString(menu,pos,str,200,MF_BYPOSITION);
		if (stricmp(str,item)>=0) return pos;
	}
	return pos;
}


 

HMENU getCategoryMenu(HMENU menu,char * category )
{
	if (!category || !strlen(category)) return menu;


	int nb=GetMenuItemCount(menu);
 
	char name[200];
	MENUITEMINFO info;
	memset(&info,0,sizeof(MENUITEMINFO));
	info.cbSize=sizeof(MENUITEMINFO);
	info.fMask=MIIM_DATA|MIIM_SUBMENU|MIIM_TYPE;
	info.dwTypeData=name;
	info.cch=200;

	for (int pos=0;pos<nb;pos++)
	{
		if (GetMenuItemInfo(menu,pos,true,&info))
		{
		//	printDebug(DEBUG_LEVEL_TODO,"compare",category,info.dwTypeData,0);
			if (info.hSubMenu)
			{
				GetMenuString(menu,pos,name,200,MF_BYPOSITION);
				if (!stricmp(name,category))
				{	
					return info.hSubMenu;
				}
			}
		}		
	}
	HMENU catM=CreatePopupMenu ();
 
	InsertMenu(menu,getInsertPos(menu,category),MF_BYCOMMAND|MF_POPUP,(UINT)catM,category);
	return catM;

}

int CPlugins::loadPlugin(char *path, DWORD id)
{
	ConfigManager conf;
	dictionary * d ;
	
    d = iniparser_load(path);

	CComboBox * filesDbl=&(getTabPreferences()->m_filesDoubleClick);
	CComboBox * appsDbl=&(getTabPreferences()->m_appsDoubleClick);

    char * name=iniparser_getstr(d, "cftoolbox/plugin:name");
	char plgpath[300];
	char action[300];
	if (name && strlen(name))
	{
 			CPlugin * plugin=new CPlugin(d);
			plugins->put(id,plugin);

			if (plugin->menuCategory)
			{
				sprintf(plgpath,"plugin : %s / %s",plugin->menuCategory,plugin->menuItem);
				sprintf(action,"%s/%s",plugin->menuCategory,plugin->menuItem);
			}
			else
			{
				sprintf(plgpath,"plugin : %s",plugin->menuItem);
				sprintf(action,"%s",plugin->menuItem);
			}
 
		if (plugin->gcfCommandLine || plugin->ncfCommandLine)
		{
			int fileIndex=addString(filesDbl,plgpath,id);
			if (!strcmp(action,conf.getOnFileDoubleClick()))
			{
				filesDbl->SetCurSel(fileIndex);	
			}
		}
		if (plugin->gameCommandLine || plugin->mediaCommandLine || plugin->toolCommandLine)
		{
			int appIndex=addString(appsDbl,plgpath,id);
			if (!strcmp(action,conf.getOnAppDoubleClick()))
			{
				appsDbl->SetCurSel(appIndex);	
			}
		}

	}
    iniparser_freedict(d);
	
	return 1;
}

void CPlugins::buildMenu(CMenu * menu,UINT  where,bool gcf, bool ncf, bool game, bool media, bool tool/*, bool installed*/)
{
	int size=plugins->size();

	if (size)
	{
 		HMENU plugs=CreatePopupMenu ();
 
		
		DWORD * ids=(DWORD*)malloc(size*4);
		plugins->getDWordKeys(ids);
		int added=0;
		for (int ind=0;ind<size;ind++)
		{
			
			CPlugin * plugin=(CPlugin *)plugins->get(ids[ind]);
 

		

			if ((gcf || ncf) && (plugin->gcfCommandLine || plugin->ncfCommandLine))
			{
			 
				HMENU catMenu=getCategoryMenu(plugs,plugin->menuCategory);
				int pos=getInsertPos(catMenu,plugin->menuItem);
 				if (
					 (!gcf || plugin->gcfCommandLine) &&
					 (!ncf || plugin->ncfCommandLine)
 				   )
					InsertMenu(catMenu,pos,MF_BYPOSITION|MF_ENABLED   ,(UINT)ids[ind],plugin->menuItem);
				else
					InsertMenu(catMenu,pos,MF_BYPOSITION|MF_GRAYED ,(UINT)ids[ind],plugin->menuItem);
				added++;
			}
			if ((game || media || tool)  && (plugin->gameCommandLine || plugin->mediaCommandLine || plugin->toolCommandLine))
			{
				HMENU catMenu=getCategoryMenu(plugs,plugin->menuCategory);
				int pos=getInsertPos(catMenu,plugin->menuItem);
 				if (
 					 (!game || plugin->gameCommandLine) &&
					 (!media || plugin->mediaCommandLine) &&
					 (!tool || plugin->toolCommandLine)
				   )
					InsertMenu(catMenu,pos,MF_BYPOSITION|MF_ENABLED   ,(UINT)ids[ind],plugin->menuItem);
				else
					InsertMenu(catMenu,pos,MF_BYPOSITION|MF_GRAYED ,(UINT)ids[ind],plugin->menuItem);
				added++;
			}
 		
		}
 
		if (added)
		{
			menu->InsertMenu(where,MF_BYCOMMAND|MF_POPUP,(UINT)plugs,"Plugins");
			menu->InsertMenu(where,MF_BYCOMMAND|MF_SEPARATOR);
		}
		else
			DestroyMenu(plugs);
		free(ids);
	}
}

CPlugin * CPlugins::getPlugin(DWORD command)
{
	return(CPlugin*) plugins->get(command);
}