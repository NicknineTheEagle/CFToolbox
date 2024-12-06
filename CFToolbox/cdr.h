/*
cdr / blob mapping
*/

#ifndef _CDR_H_
#define _CDR_H_

#include "Registry.h"

typedef struct 
{
	RegistryVector * vector;
	
	DWORD version;
} ContentDescriptionRecord;

typedef struct 
{
	RegistryNode * node;

	DWORD appId; // 1
	char * name; // 2
	char * installDirName; // 3
	DWORD minCacheFileSizeMb; // 4
	DWORD maxCacheFileSizeMb; // 5

	DWORD onFirstLaunch; // 8 
	DWORD isBandwidthGreedy; // 9
	
	DWORD currentVersionId; // 11
	DWORD trickleVersionId; // 13

	char * betaVersionPassword; // 15
	DWORD betaVersionId; // 16
	char * legacyInstallDirName; // 17
	DWORD useFileSystemDvr; // 19
	DWORD manifestOnlyApp; // 20
	DWORD appOfManifestOnlyCache; // 21
} CDRApp;

typedef struct
{
	RegistryNode * node;

	char * description; // 1
	char * commandLine; // 2
	DWORD iconIndex; // 3
	DWORD noDesktopShortcut; // 4
	DWORD noStartMenuShortcut; // 5
	DWORD longRunningUnattended; // 6
} CDRAppLaunchOption;

typedef struct
{
	RegistryNode * node;

	char * description; // 1
	DWORD versionId; // 2 -- also the main blob key
	DWORD isNotAvailable; // 3
	RegistryVector * launchOptionIds; // 4
	char * depotEncryptionKey; // 5
	DWORD isEncryptionKeyAvailable; // 6
	DWORD isRebased; // 7
	DWORD isLongVersionRoll; // 8
} CDRAppVersion;

typedef struct
{
	RegistryNode * node;

	DWORD appId; // 1
	char * mountName; // 2
	DWORD isOptional; // 3
} CDRAppFilesystem;


ContentDescriptionRecord getCDR(RegistryVector * node);
int getCDRNbApps(ContentDescriptionRecord cdr);

CDRApp getCDRAppAt(ContentDescriptionRecord cdr,int index);
CDRApp getCDRApp(ContentDescriptionRecord cdr,int appId);
int getCDRAppNbLaunchOptions(CDRApp app);
int getCDRAppNbVersions(CDRApp app);
int getCDRAppNbFilesystem(CDRApp app);

CDRAppLaunchOption getCDRAppLaunchOption(CDRApp app, int index);
CDRAppVersion getCDRAppVersion(CDRApp app, int index);
CDRAppFilesystem getCDRAppFilesystem(CDRApp app, int index);
RegistryVector * getCDRAppUserDefinedData(CDRApp app);

CDRApp getCDRLocalisedApp(ContentDescriptionRecord cdr, CDRApp app, char* language);

// subscriptions part is never used so is not implemented

int isFile(CDRApp app);
int isApp(CDRApp app);

#endif