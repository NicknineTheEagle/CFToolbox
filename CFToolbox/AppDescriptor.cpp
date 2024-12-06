// AppDescriptor.cpp: implementation of the CAppDescriptor class.
//
//////////////////////////////////////////////////////////////////////
#include "CFManager.h"
#include "stdafx.h"
#include "CFToolbox.h"
#include "AppDescriptor.h"
#include "tga.h"
#include "tools.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#include "CFDescriptor.h"
#include "AppIcons.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include "debug.h"

CAppDescriptor::CAppDescriptor(ContentDescriptionRecord cdr,CDRApp app, HashMapDword * files,DWORD * _appGcfIds,char * _appGcfOptional, int _appGcfIdsCount)
{
	iconId=0; // no Icon
	runningTask=0;
	appName=0;
	editor=0;
	manual=0;
	appGcfNames=0;
	appGcfSizes=0;
	strcpy(information,"");
	nbProperties=0;
	propertiesNames=0;
	propertiesValues=0;
	appGcfOptional=0;
	*type=0;
	commonPath=0;
 	appGcfIdsCount=_appGcfIdsCount;
 	appGcfIds=(DWORD*)malloc(4*appGcfIdsCount);
	appGcfOptional=(char*)malloc(appGcfIdsCount);
	
	 for (int ind=0;ind<appGcfIdsCount;ind++)
	 {
		appGcfOptional[ind]=_appGcfOptional[ind];
		appGcfIds[ind]=_appGcfIds[ind];
	 }

  	RegistryVector * data=getCDRAppUserDefinedData(app);
	if(data)
	{
		RegistryNode * iconNode=data->getNode(0x6E6F6369); // icon in hexa
		if (iconNode && strlen(iconNode->getStringValue()))
		{
 			iconId=getAppIcons()->getIconId(iconNode->getStringValue());
		}
		RegistryNode * editorNode=data->getNode("developer");
		if (editorNode)
		{
			editor=(char*)malloc(strlen(editorNode->getStringValue())+1);
			strcpy(editor,editorNode->getStringValue());
		}
		RegistryNode * manualNode=data->getNode("GameManualURL");
		if (manualNode)
		{
			manual=(char*)malloc(strlen(manualNode->getStringValue())+1);
			strcpy(manual,manualNode->getStringValue());
		}
		RegistryNode * stateNode=data->getNode("state");
		if (stateNode && strlen(stateNode->getStringValue()) && stricmp("eStateAvailable",stateNode->getStringValue()) && stricmp("eStateJustReleased",stateNode->getStringValue()))
		{
			if (stricmp(stateNode->getStringValue(),"eState")) strcpy(type,stateNode->getStringValue()+strlen("eState"));
		 
		}
		RegistryNode * mediaType=data->getNode("MediaFileType");
		if (mediaType)
		{
			strcpy(type,mediaType->getStringValue());
		}
		int nb=data->getNbNodes();
		propertiesNames=(char**)malloc(sizeof(char*)*nb);
		propertiesValues=(char**)malloc(sizeof(char*)*nb);
		for (int ind=0;ind<nb;ind++)
		{
			RegistryNode * node=data->getNodeAt(ind);
			if (node->getDescriptionType()==NODE_DESC_TYPE_STRING)
			{
				propertiesNames[nbProperties]=(char*)malloc(strlen(node->getStringDescription())+1);
				strcpy(propertiesNames[nbProperties],node->getStringDescription());
			 
				propertiesValues[nbProperties]=(char*)malloc(strlen(node->getStringValue())+1);
					strcpy(propertiesValues[nbProperties],node->getStringValue());
				nbProperties++;
			}
		}
	}

	//	if (!strlen(type)) strcpy(type,"Game");

	appName=(char*)malloc(strlen(app.name)+1);
	strcpy(appName,app.name);
 	appId=app.appId;

	completion=0;
	realSize=0;
	appGcfNames=(char**)malloc(sizeof(char*)*appGcfIdsCount);
	appGcfSizes=(DWORD*)malloc(4*appGcfIdsCount);
	for (  ind=0;ind<appGcfIdsCount;ind++)
	{
		CCFDescriptor * desc=(CCFDescriptor *)files->get(appGcfIds[ind]);
		
		if (desc)
		{
			appGcfSizes[ind]=desc->realSize;
			if (desc->cdrVersion==desc->fileVersion) completion+=desc->completion*(float)desc->realSize;
			realSize+=desc->realSize;
			appGcfNames[ind]=(char*)malloc(strlen(desc->fileName)+1+(appGcfOptional[ind] ? 11 : 0));
			strcpy(appGcfNames[ind],desc->fileName);
			if (!commonPath && desc->isNcf)
			{
				commonPath=(char*)malloc(strlen(desc->commonPath)+1);
				strcpy(commonPath,desc->commonPath);
			}
		}
		else
		{
 			// not found search in cdr for missing size
			CDRApp cf=getCDRApp(cdr,appGcfIds[ind]);
			appGcfSizes[ind]=(cf.maxCacheFileSizeMb*1024*1024);
			if (!appGcfOptional[ind])
			{
				realSize+=appGcfSizes[ind];
			}
			appGcfNames[ind]=(char*)malloc((cf.installDirName ? strlen(cf.installDirName) : 0)+1+4+(appGcfOptional[ind] ? 11 : 0));
			if (cf.installDirName) strcpy(appGcfNames[ind],cf.installDirName);
			else
				strcpy(appGcfNames[ind],"");
			strcat(appGcfNames[ind],(cf.manifestOnlyApp || cf.appOfManifestOnlyCache ? ".ncf" : ".gcf"));
			
			strlwr(appGcfNames[ind]);
			if (!commonPath && app.manifestOnlyApp)
			{
				commonPath=(char*)malloc(strlen(app.installDirName)+1);
				strcpy(commonPath,app.installDirName );
			}
		}
 		if (appGcfOptional[ind]) strcat(appGcfNames[ind]," (optional)");

	}

	completion/=(double)(signed __int64)realSize;
 	
}

CAppDescriptor::~CAppDescriptor()
{
	if (commonPath) free(commonPath);
	if (appName) free(appName);
	if (editor) free(editor);
	if (appGcfIds) free(appGcfIds);
	
	if (manual) free(manual);
	if (appGcfNames) 
	{
		for (int ind=0;ind<appGcfIdsCount;ind++)
		{
			free(appGcfNames[ind]);
		}
		free(appGcfNames);
	}
		
	for (int ind=0;ind<nbProperties;ind++)
	{
		free(propertiesNames[ind]);
		free(propertiesValues[ind]);
	}
	free(propertiesNames);
	free(propertiesValues);
	if (appGcfOptional) free(appGcfOptional);
}

void CAppDescriptor::update()
{
	

}
