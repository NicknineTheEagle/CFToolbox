// ConfigManager.h: interface for the ConfigManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGMANAGER_H__AF48ACF2_DCFB_40E7_A0A0_3EA441CC1379__INCLUDED_)
#define AFX_CONFIGMANAGER_H__AF48ACF2_DCFB_40E7_A0A0_3EA441CC1379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ConfigManager  
{
public:
	char * getOnFileDoubleClick();
	void setOnFileDoubleClick(char * action);
	char * getOnAppDoubleClick();
	void setOnAppDoubleClick(char * action);
	
	void setCorrectBeforeArchiveUpdateCreation(bool validate);
	bool isCorrectBeforeArchiveUpdateCreation();
	void setValidateDownloads(bool validate);
	bool isValidateDownloads();
	void setOutputPath(char * path);
	char * getOutputPath();
	void setMaxTask(int max);
	int getMaxTasks();
 
	void setDebugLevel(int level);
	int getDebugLevel();
	bool isAutomaticCDRUpdate();
	void setAutomaticCDRUpdate(bool automatic);
	ConfigManager();
	virtual ~ConfigManager();
	char * getLanguage();
	void setLanguage(char * language);
	char * getSteamAppsPath();
	void setSteamAppsPath(char * path);
	char * getAppCommandLineTemplate();
	void setAppCommandLineTemplate(char * path);
	char * getFileCommandLineTemplate();
	void setFileCommandLineTemplate(char * path);
	bool isWaitForFileCommandLineEnd();
	void setWaitForFileCommandLineEnd(bool wait);
	bool isWaitForAppCommandLineEnd();
	void setWaitForAppCommandLineEnd(bool wait);
	bool isSharingSteamFolder();
	void setSharingSteamFolder(bool sharing);
	bool isSecuredGCFUpdates();
	void setSecuredGCFUpdates(bool secured);
	bool isAccurateNcfCompletion();
	void setAccurateNcfCompletion(bool accurate);
	bool isDisableInstall();
	void setDisableInstall(bool disable);
	bool isShowBandwidthUsage();
	void setShowBandwidthUsage(bool show);
	bool isResetMiniCF();
	void setResetMiniCF(bool reset);
};

#endif // !defined(AFX_CONFIGMANAGER_H__AF48ACF2_DCFB_40E7_A0A0_3EA441CC1379__INCLUDED_)
