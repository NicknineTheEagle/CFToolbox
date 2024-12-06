#include "cdr.h"

ContentDescriptionRecord getCDR(RegistryVector * vector)
{
	ContentDescriptionRecord cdr;
	cdr.vector=vector;
	cdr.version=vector->getNode((DWORD)0)->getDwordDescription();
	return cdr;
}

int getCDRNbApps(ContentDescriptionRecord cdr)
{
	return cdr.vector->getNode(1)->getVectorValue()->getNbNodes();
}

DWORD getSafeNodeDWordValue(RegistryVector * v,DWORD id)
{
	RegistryNode * node=v->getNode(id);
	if (!node) return 0;
	return node->getDWordValue();
}
DWORD getSafeNodeByteValue(RegistryVector * v,DWORD id)
{
	RegistryNode * node=v->getNode(id);
	if (!node) return 0;
	return node->getByteValue();
}
char * getSafeNodeStringValue(RegistryVector * v,DWORD id)
{
	RegistryNode * node=v->getNode(id);
	if (!node) return 0;
	return node->getStringValue();
}
void populateCDRApp(CDRApp * app)
{
	RegistryVector * v=app->node->getVectorValue();

	app->appId=getSafeNodeDWordValue(v,1);
	app->name=getSafeNodeStringValue(v,2);

	app->installDirName=getSafeNodeStringValue(v,3);
	app->minCacheFileSizeMb=getSafeNodeDWordValue(v,4);
	app->maxCacheFileSizeMb=getSafeNodeDWordValue(v,5);

	app->onFirstLaunch=getSafeNodeDWordValue(v,8);
	app->isBandwidthGreedy=getSafeNodeByteValue(v,9);
	
	app->currentVersionId=getSafeNodeDWordValue(v,11);
	app->trickleVersionId=getSafeNodeDWordValue(v,13);

	app->betaVersionPassword=getSafeNodeStringValue(v,15);
	app->betaVersionId=getSafeNodeDWordValue(v,16);
	app->legacyInstallDirName=getSafeNodeStringValue(v,17);
	app->useFileSystemDvr=getSafeNodeByteValue(v,19);
	app->manifestOnlyApp=getSafeNodeByteValue(v,20);
	app->appOfManifestOnlyCache=getSafeNodeDWordValue(v,21);
}

CDRApp getCDRAppAt(ContentDescriptionRecord cdr, int index)
{
	CDRApp app;
 	memset(&app,0,sizeof(CDRApp));
	app.node=cdr.vector->getNode(1)->getVectorValue()->getNodeAt(index);
	if (app.node) populateCDRApp(&app);
	return app;
}

CDRApp getCDRApp(ContentDescriptionRecord cdr,int appId)
{
	CDRApp app;
 	memset(&app,0,sizeof(CDRApp));
	app.node=cdr.vector->getNode(1)->getVectorValue()->getNode(appId);
	if (app.node) populateCDRApp(&app);
	return app;
}

int getCDRAppNbLaunchOptions(CDRApp app)
{
	RegistryNode * node=app.node->getVectorValue()->getNode(6);
	if (!node) return 0;
	return node->getVectorValue()->getNbNodes();
}

CDRAppLaunchOption getCDRAppLaunchOption(CDRApp app, int index)
{
	CDRAppLaunchOption option;
	memset(&option,0,sizeof(CDRAppLaunchOption));
	option.node=app.node->getVectorValue()->getNode(6)->getVectorValue()->getNodeAt(index);
	if (!option.node) return option;

	RegistryVector * v=option.node->getVectorValue();
	
	option.description=getSafeNodeStringValue(v,1);
	option.commandLine=getSafeNodeStringValue(v,2);
	option.iconIndex=getSafeNodeDWordValue(v,3);
	option.noDesktopShortcut=getSafeNodeByteValue(v,4);
	option.noStartMenuShortcut=getSafeNodeByteValue(v,5);
	option.longRunningUnattended=getSafeNodeByteValue(v,6);

	return option;
}

int getCDRAppNbVersions(CDRApp app)
{
	RegistryNode * node=app.node->getVectorValue()->getNode(10);
	if (!node) return 0;
	return node->getVectorValue()->getNbNodes();
}

CDRAppVersion getCDRAppVersion(CDRApp app, int index)
{
	CDRAppVersion version;
	memset(&version,0,sizeof(CDRAppVersion));
	version.node=app.node->getVectorValue()->getNode(10)->getVectorValue()->getNodeAt(index);
	if (!version.node) return version;

	RegistryVector * v=version.node->getVectorValue();
	
	version.description=getSafeNodeStringValue(v,1);
	version.versionId=getSafeNodeDWordValue(v,2);
	version.isNotAvailable=getSafeNodeByteValue(v,3);
	if (v->getNode(4)) version.launchOptionIds=v->getNode(4)->getVectorValue();
	version.depotEncryptionKey=getSafeNodeStringValue(v,5);
	version.isEncryptionKeyAvailable=getSafeNodeByteValue(v,6);
	version.isRebased=getSafeNodeByteValue(v,7);
	version.isLongVersionRoll=getSafeNodeByteValue(v,8);

	return version;
}

int getCDRAppNbFilesystem(CDRApp app)
{
	RegistryNode * node=app.node->getVectorValue()->getNode(12);
	if (!node) return 0;
	return node->getVectorValue()->getNbNodes();
}

CDRAppFilesystem getCDRAppFilesystem(CDRApp app, int index)
{
	CDRAppFilesystem filesystem;
	memset(&filesystem,0,sizeof(CDRAppFilesystem));
	filesystem.node=app.node->getVectorValue()->getNode(12)->getVectorValue()->getNodeAt(index);
	if (!filesystem.node) return filesystem;

	RegistryVector * v=filesystem.node->getVectorValue();
	
	filesystem.appId=getSafeNodeDWordValue(v,1);
	filesystem.mountName=getSafeNodeStringValue(v,2);
	filesystem.isOptional=getSafeNodeByteValue(v,3);

	return filesystem;
}

RegistryVector * getCDRAppUserDefinedData(CDRApp app)
{
	RegistryNode * node=app.node->getVectorValue()->getNode(14);
	if (!node) return 0;
	return node->getVectorValue();
}

CDRApp getCDRLocalisedApp(ContentDescriptionRecord cdr, CDRApp app, char* appLanguage)
{
	char language[200]; 
	strcpy(language,appLanguage);
	strlwr(language);
	RegistryVector* userData=getCDRAppUserDefinedData(app);
	if (userData)
	{
		RegistryNode * availableLanguages=userData->getNode("languages");
		if (availableLanguages)
		{
			char * available=availableLanguages->getStringValue();
			if (!strstr(available,language))
			{
				strcpy(language,"english");
			}
		}
	}

	char searchFor[1000];
	sprintf(searchFor,"%s %s",app.name,language);
	
	int nb=getCDRNbApps(cdr);
 	// too slow
 //	for (int ind=0;ind<nb;ind++)
//	{
//		CDRApp locale=getCDRAppAt(cdr,ind);
//		if (!stricmp(searchFor,locale.name)) return locale;
//	}
	RegistryNode * node=app.node;
	while (node->getNextNode())
	{
		node=node->getNextNode();
		if (!stricmp(searchFor,node->getVectorValue()->getNode(2)->getStringValue())) return getCDRApp(cdr,node->getDwordDescription());
	}
 	CDRApp  none;
	none.node=0;
	return none;
}

/*
#include "debug.h"

CDRApp getCDRLocalisedApp(ContentDescriptionRecord cdr, CDRApp app, char* language)
{
	char lName[1000];
	char nodeAppName[1000];
	char lLang[100];

	strcpy(lName,app.installDirName);
	strlwr(lName);

	strcpy(lLang,language);
	strlwr(lLang);
	//sprintf(searchFor,"%s_%s",app.installDirName,language);
	unsigned int searchLen=strlen(app.installDirName)+1+strlen(language);

	int nb=getCDRNbApps(cdr);
 	// too slow
 //	for (int ind=0;ind<nb;ind++)
//	{
//		CDRApp locale=getCDRAppAt(cdr,ind);
//		if (!stricmp(searchFor,locale.name)) return locale;
//	}
	RegistryNode * node=app.node;
	while (node->getNextNode())
	{
		 
		node=node->getNextNode();
		char * iN=node->getVectorValue()->getNode(3)->getStringValue();
		if (strlen(iN)==searchLen)
		{
			
			strcpy(nodeAppName,iN);
			strlwr(nodeAppName);
			if (strstr(nodeAppName,lName) &&
				strstr(nodeAppName,lLang))
			return getCDRApp(cdr,node->getDwordDescription());
		}
	}
 	CDRApp  none;
	none.node=0;
	return none;
}
*/


int isFile(CDRApp app)
{
 	if (!app.node) return 0;
	if (app.manifestOnlyApp && app.appOfManifestOnlyCache!=app.appId) return 0;
	int nb=getCDRAppNbFilesystem(app);
	if (!nb) return 1;
	for (int ind=0;ind<nb;ind++)
	{
		CDRAppFilesystem fs=getCDRAppFilesystem(app,ind);
		if (fs.appId==app.appId) return 1;
	}
	if (getCDRAppUserDefinedData(app)==0) return 1;
	return 0;
 
}
int isApp(CDRApp app)
{
	if (!app.node) return 0;
	return !isFile(app); // not implemented
}