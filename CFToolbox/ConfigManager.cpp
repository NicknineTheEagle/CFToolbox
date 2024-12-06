// ConfigManager.cpp: implementation of the ConfigManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFToolbox.h"
#include "ConfigManager.h"
#include "ServerRegistry.h"
#include "debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CONFIG "Configuration"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
char * defaultOnDoubleClick="";
char * defaultSteamApps="steamapps";
char * defaultLanguage="english";
bool defaultAutomaticCDRUpdate=true;
bool defaultCorrectBeforeArchiveUpdateCreation=false;
bool defaultValidateDownloads=false;
int defaultDebugLevel=DEBUG_LEVEL_DISABLED;
int defaultMaxTasks=2;
char * defaultCommandLineTemplate="";
int defaultCommandLineMode=true;
int defaultSharingSteamFolder=true;
int defaultSecuredGCFUpdates=true;
int defaultAccurateNcfCompletion=true;
int defaultDisableInstall=true;
int defaultShowBandwidthUsage=false;
int defaultResetMiniCF=true;

ConfigManager::ConfigManager()
{

}

ConfigManager::~ConfigManager()
{

}

char * ConfigManager::getSteamAppsPath()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultSteamApps;
	
	RegistryValue * value=key->getValue("SteamAppsPath");

	if (!value) return defaultSteamApps;

	return value->getString();
}
void ConfigManager::setSteamAppsPath(char * path)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("SteamAppsPath");
	
	RegistryValue * value=new RegistryValue();
	value->setName("SteamAppsPath");
	value->setString(path);
	
	key->addValue(value);

	saveServerRegistry();
	printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Steam apps modified",path,0);
}
 
char * ConfigManager::getOnFileDoubleClick()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultOnDoubleClick;
	
	RegistryValue * value=key->getValue("OnFileDoubleClick");

	if (!value) return defaultOnDoubleClick;

	return value->getString();
}
void ConfigManager::setOnFileDoubleClick(char * action)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("OnFileDoubleClick");
	
	RegistryValue * value=new RegistryValue();
	value->setName("OnFileDoubleClick");
	value->setString(action);
	
	key->addValue(value);

	saveServerRegistry();
//	printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Steam apps modified",path,0);
}

char * ConfigManager::getOnAppDoubleClick()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultOnDoubleClick;
	
	RegistryValue * value=key->getValue("OnAppDoubleClick");

	if (!value) return defaultOnDoubleClick;

	return value->getString();
}
void ConfigManager::setOnAppDoubleClick(char * action)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("OnAppDoubleClick");
	
	RegistryValue * value=new RegistryValue();
	value->setName("OnAppDoubleClick");
	value->setString(action);
	
	key->addValue(value);

	saveServerRegistry();
//	printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Steam apps modified",path,0);
}

char * ConfigManager::getAppCommandLineTemplate()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultCommandLineTemplate;
	
	RegistryValue * value=key->getValue("AppCommandLineTemplate");

	if (!value) return defaultCommandLineTemplate;

	return value->getString();
}
void ConfigManager::setAppCommandLineTemplate(char * path)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("AppCommandLineTemplate");
	
	RegistryValue * value=new RegistryValue();
	value->setName("AppCommandLineTemplate");
	value->setString(path);
	
	key->addValue(value);

	saveServerRegistry();
	printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Game/Media/Tool execution command line template modified",path,0);
}

char * ConfigManager::getFileCommandLineTemplate()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultCommandLineTemplate;
	
	RegistryValue * value=key->getValue("FileCommandLineTemplate");

	if (!value) return defaultCommandLineTemplate;

	return value->getString();
}
void ConfigManager::setFileCommandLineTemplate(char * path)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("FileCommandLineTemplate");
	
	RegistryValue * value=new RegistryValue();
	value->setName("FileCommandLineTemplate");
	value->setString(path);
	
	key->addValue(value);

	saveServerRegistry();
	printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"GCF/NCF command line template modified",path,0);
}

char * ConfigManager::getLanguage()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultLanguage;
	
	RegistryValue * value=key->getValue("SteamAppsLanguage");

	if (!value) return defaultLanguage;

	return value->getString();
}
void ConfigManager::setLanguage(char * language)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("SteamAppsLanguage");
	
	RegistryValue * value=new RegistryValue();
	value->setName("SteamAppsLanguage");
	value->setString(language);
	
	key->addValue(value);

	saveServerRegistry();
	printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Steam apps language modified",language,0);
}
void ConfigManager::setAutomaticCDRUpdate(bool automatic)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("automaticCDRUpdate");
	
	RegistryValue * value=new RegistryValue();
	value->setName("automaticCDRUpdate");
	value->setDWORD(automatic);
	
	key->addValue(value);

	saveServerRegistry();
	if (automatic) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"ContentDescriptionRecord automatic update enabled",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"ContentDescriptionRecord automatic update disabled",0);
}

bool ConfigManager::isAutomaticCDRUpdate()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultAutomaticCDRUpdate;
	
	RegistryValue * value=key->getValue("automaticCDRUpdate");

	if (!value) return defaultAutomaticCDRUpdate;

	return (bool)value->getDWORD();
}

int ConfigManager::getDebugLevel()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultDebugLevel;
	
	RegistryValue * value=key->getValue("debugLevel");

	if (!value) return defaultDebugLevel;

	return value->getDWORD();
}

void ConfigManager::setDebugLevel(int level)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("debugLevel");
	
	RegistryValue * value=new RegistryValue();
	value->setName("debugLevel");
	value->setDWORD(level);
	
	key->addValue(value);
	saveServerRegistry();
}

 

int ConfigManager::getMaxTasks()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultMaxTasks;
	
	RegistryValue * value=key->getValue("maximumActiveTasks");

	if (!value) return defaultMaxTasks;

	return value->getDWORD();
}

void ConfigManager::setMaxTask(int max)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("maximumActiveTasks");
	
	RegistryValue * value=new RegistryValue();
	value->setName("maximumActiveTasks");
	value->setDWORD(max);
	
	key->addValue(value);
	saveServerRegistry();
}

char * ConfigManager::getOutputPath()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return getSteamAppsPath();
	
	RegistryValue * value=key->getValue("outputPath");

	if (!value) return getSteamAppsPath();

	return value->getString();
}

void ConfigManager::setOutputPath(char *path)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("outputPath");
	
	RegistryValue * value=new RegistryValue();
	value->setName("outputPath");
	value->setString(path);
	
	key->addValue(value);

	saveServerRegistry();
	printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"output path modified",path,0);
}
 

bool ConfigManager::isCorrectBeforeArchiveUpdateCreation()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultCorrectBeforeArchiveUpdateCreation;
	
	RegistryValue * value=key->getValue("correctBeforeArchiveOrUpdateCreation");

	if (!value) return defaultCorrectBeforeArchiveUpdateCreation;

	return (bool)value->getDWORD();
}

void ConfigManager::setCorrectBeforeArchiveUpdateCreation(bool validate)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("correctBeforeArchiveOrUpdateCreation");
	
	RegistryValue * value=new RegistryValue();
	value->setName("correctBeforeArchiveOrUpdateCreation");
	value->setDWORD(validate);
	
	key->addValue(value);

	saveServerRegistry();
	if (validate) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Validation before archive / update creation enabled",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Validation before archive / update creation disabled",0);
}
void ConfigManager::setWaitForFileCommandLineEnd(bool wait)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("waitForFileCommandLineEnd");
	
	RegistryValue * value=new RegistryValue();
	value->setName("waitForFileCommandLineEnd");
	value->setDWORD(wait);
	
	key->addValue(value);

	saveServerRegistry();
	if (wait) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"GCF/NCF command line mode set to sequential execution",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"GCF/NCF command line mode set to parallel execution",0);
}

bool ConfigManager::isWaitForFileCommandLineEnd()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultCommandLineMode;
	
	RegistryValue * value=key->getValue("waitForFileCommandLineEnd");

	if (!value) return defaultCommandLineMode;

	return (bool)value->getDWORD();
}
void ConfigManager::setWaitForAppCommandLineEnd(bool wait)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("waitForAppCommandLineEnd");
	
	RegistryValue * value=new RegistryValue();
	value->setName("waitForAppCommandLineEnd");
	value->setDWORD(wait);
	
	key->addValue(value);

	saveServerRegistry();
	if (wait) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Game/Media/Tool command line mode set to sequential execution",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Game/Media/Tool command line mode set to parallel execution",0);
}

bool ConfigManager::isWaitForAppCommandLineEnd()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultCommandLineMode;
	
	RegistryValue * value=key->getValue("waitForAppCommandLineEnd");

	if (!value) return defaultCommandLineMode;

	return (bool)value->getDWORD();
}
void ConfigManager::setSharingSteamFolder(bool sharing)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("sharedSteamApps");
	
	RegistryValue * value=new RegistryValue();
	value->setName("sharedSteamApps");
	value->setDWORD(sharing);
	
	key->addValue(value);

	saveServerRegistry();
	if (sharing) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"CF Toolbox and Steam are sharing the steamapps folder",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"CF Toolbox and Steam using independent steamapps folder",0);
}

bool ConfigManager::isSharingSteamFolder()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultSharingSteamFolder;
	
	RegistryValue * value=key->getValue("sharedSteamApps");

	if (!value) return defaultSharingSteamFolder;

	return (bool)value->getDWORD();
}
void ConfigManager::setSecuredGCFUpdates(bool secured)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("securedCFUpdates");
	
	RegistryValue * value=new RegistryValue();
	value->setName("securedCFUpdates");
	value->setDWORD(secured);
	
	key->addValue(value);

	saveServerRegistry();
	if (secured) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"GCF/NCF update secure mode enabled",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"GCF/NCF update secure mode disabled",0);
}

bool ConfigManager::isSecuredGCFUpdates()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultSecuredGCFUpdates;
	
	RegistryValue * value=key->getValue("securedCFUpdates");

	if (!value) return defaultSecuredGCFUpdates;

	return (bool)value->getDWORD();
}

void ConfigManager::setAccurateNcfCompletion(bool accurate)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("accurateNcfCompletion");
	
	RegistryValue * value=new RegistryValue();
	value->setName("accurateNcfCompletion");
	value->setDWORD(accurate);
	
	key->addValue(value);

	saveServerRegistry();
	if (accurate) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"NCF accurate completion enabled",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"NCF accurate completion disabled",0);
}

bool ConfigManager::isAccurateNcfCompletion()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultAccurateNcfCompletion;
	
	RegistryValue * value=key->getValue("accurateNcfCompletion");

	if (!value) return defaultSecuredGCFUpdates;

	return (bool)value->getDWORD();
}

void ConfigManager::setDisableInstall(bool disable)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("accurateNcfCompletion");
	
	RegistryValue * value=new RegistryValue();
	value->setName("disableInstall");
	value->setDWORD(disable);
	
	key->addValue(value);

	saveServerRegistry();
	if (disable) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"GCF/NCF install disabled",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"GCF/NCF install enabled",0);
}

bool ConfigManager::isDisableInstall()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultDisableInstall;
	
	RegistryValue * value=key->getValue("disableInstall");

	if (!value) return defaultDisableInstall;

	return (bool)value->getDWORD();
}

void ConfigManager::setShowBandwidthUsage(bool show)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("showBandwidthUsage");
	
	RegistryValue * value=new RegistryValue();
	value->setName("showBandwidthUsage");
	value->setDWORD(show);
	
	key->addValue(value);

	saveServerRegistry();
	if (show) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Bandwidth monitor configured to display real bandwidth usage",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Bandwidth monitor configired to display files download speed",0);
}

bool ConfigManager::isShowBandwidthUsage()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultShowBandwidthUsage;
	
	RegistryValue * value=key->getValue("showBandwidthUsage");

	if (!value) return defaultShowBandwidthUsage;

	return (bool)value->getDWORD();
}
  
void ConfigManager::setResetMiniCF(bool reset)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("resetMiniCF");
	
	RegistryValue * value=new RegistryValue();
	value->setName("resetMiniCF");
	value->setDWORD(reset);
	
	key->addValue(value);

	saveServerRegistry();
	if (reset) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Reseting headers of miniGCFs",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Not reseting headers of miniGCFs",0);
}

bool ConfigManager::isResetMiniCF()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultResetMiniCF;
	
	RegistryValue * value=key->getValue("resetMiniCF");

	if (!value) return defaultResetMiniCF;

	return (bool)value->getDWORD();
}
  
bool ConfigManager::isValidateDownloads()
{
	RegistryKey * key=getServerRegistry()->getKey("/TopKey/Config");
		
	if (!key) return defaultValidateDownloads;
	
	RegistryValue * value=key->getValue("validateDownloadStream");

	if (!value) return defaultValidateDownloads;

	return (bool)value->getDWORD();
}

void ConfigManager::setValidateDownloads(bool validate)
{
	RegistryKey * key=getServerRegistry()->setKey("/TopKey/Config");
	key->removeValue("validateDownloadStream");
	
	RegistryValue * value=new RegistryValue();
	value->setName("validateDownloadStream");
	value->setDWORD(validate);
	
	key->addValue(value);

	saveServerRegistry();
	if (validate) printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Download streams validation enabled",0);
	else printDebug(DEBUG_LEVEL_DEBUG,CONFIG,"Download streams validation disabled",0);
}