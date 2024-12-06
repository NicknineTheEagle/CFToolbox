// CFManager.h: interface for the CCFManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CFMANAGER_H__122364D2_BE88_4ABD_9124_0CB21882AF93__INCLUDED_)
#define AFX_CFMANAGER_H__122364D2_BE88_4ABD_9124_0CB21882AF93__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "resource.h"
#include "cdr.h"
#include "gcf.h"
#include "HashMap.h"
#include "Task.h"
#include "CFDescriptor.h"
#include "AppDescriptor.h"
#include "Bandwidth.h"
#include "TabBandwidth.h"
#include "SteamNetwork.h"
#include "CSClient.h"
#include "tabfiles.h"
#include "plugins.h"

class CCFManager  
{
public:
	void executePlugin(CPlugin * plugin, CCFDescriptor * desc);
	void executePlugin(CPlugin * plugin, CAppDescriptor * desc );
	void download(DWORD cfId, char* ticket, int ticketLen,bool correct);
	void downloadNew(DWORD cfId, char* ticket, int ticketLen, int version = -1);
	bool deleteFile(DWORD cfId);
	void createMiniCF(DWORD id, char * outPath);
	void cancelTask(DWORD id);
	void validate(DWORD cfId);
	void correct(DWORD cfId);
	void openFile(DWORD id);
	void apply_Update(DWORD id, char * update);
	void makeUpdate(DWORD id,char * archive, char * outPath, bool correct);
	void makeArchive(DWORD id, char * outPath, bool correct);
	CCFManager();
	virtual ~CCFManager();

	void refreshApps();
	int refreshContent();
	HashMapDword * files;
	HashMapDword * apps;
	HashMapDword * cdrAvailableFiles;
private :
	void buildApps(ContentDescriptionRecord cdr);
	void addFile(ContentDescriptionRecord cdr, CDRApp app,char * fileName, GCF *gcf);
	void addApp(ContentDescriptionRecord cdr,CDRApp app, HashMapDword * files,DWORD * appGcfIds,char * appGcfOptional, int appGcfIdsCount);
	void purgeFiles();
	void scanGcf(ContentDescriptionRecord cdr, CDRApp app, char * fileName, char * fullPath);

};

void initCFManager();
CCFManager * getCFManager();

class CCFDownloader : public CTask
{
	CCFDescriptor * descriptor;
	char * ticket;

	int ticketLen;
	bool correct;	
	bool isNewFile;
	int appVersion;
	
	public : 
	char contentServerURL[1000];
	DWORD remainingTime;	
	CCFDownloader(CCFDescriptor * _descriptor, int version, char * _ticket, int _ticketLen, bool _correct,bool _isNewFile ) : CTask ()
	{
		*contentServerURL=0;
		descriptor=_descriptor;
		descriptor->runningTask=this;
		cfId=descriptor->fileId;
		appVersion=version;
		strcpy(descriptor->information,"");
		ticket=(char*)malloc(_ticketLen);
		memcpy(ticket,_ticket,_ticketLen);
		ticketLen=_ticketLen;
		correct=_correct;
		isNewFile=_isNewFile;
		remainingTime=0;
	}
	~CCFDownloader()
	{
	 
 
	}
	 virtual DWORD Run( LPVOID /* arg */ )
	{ 
		 if (descriptor->isMoreThan4GB)
		{
			strcpy(descriptor->information,"4GB+ files not supported");
			descriptor->update();
			descriptor->runningTask=0;
			notifyTaskEnd();
			return 0;
		}
		if (pleaseStop)
			strcpy(descriptor->information,"Download paused");
		else
		{
			ConfigManager conf;
			bool ok=true;
			if (_getDiskFreeSpace(conf.getSteamAppsPath())<1024) 
			{
				// less than 1 Gb space alert before downloading.
				if (MessageBox(NULL,"Less than 1GB free space on the GCF / NCF Storage path. Proceed anyway ?","CF Toolbox",MB_ICONQUESTION|MB_YESNO)==IDNO)
					ok=false;
				if (isNewFile && !ok)
				{
					getCFManager()->files->remove(descriptor->fileId);
					getTabFiles()->removeFile(cfId);
				}
			}
			
			if (ok)
			{
 				registerDownloaderBandwidthMonitor(cfId);
				getTabBandwidth()->addDownloader(descriptor->fileName,this);

				if (correct)
				{
					strcpy( taskName,"Downloading (pre-validating)");
					char root[1000];
					strcpy(root,descriptor->path);
					root[strlen(root)-(strlen(descriptor->fileName)+1)]=0;

					GCF * gcf=parseGCF(descriptor->path);
				
					if ((int)gcf>1) 
					{
						validate(root,descriptor->commonPath,gcf,true,descriptor->fileName,DEBUG_LEVEL_DEBUG,taskName,&taskProgress,&pleaseStop);
						deleteGCF(gcf);
					}
					descriptor->update();
				}


				ConfigManager conf;
				SteamNetwork network;
 				strcpy( taskName,"Downloading");
				int res=downloadApp(network.getConfigServer(),network.getConfigServerPort(),network.getContentDescriptionRecord(),cfId,appVersion,conf.getSteamAppsPath(),ticket,ticketLen,descriptor->fileName, 0,contentServerURL,taskName,&taskProgress,&remainingTime,&pleaseStop,conf.isSecuredGCFUpdates(),conf.isShowBandwidthUsage(),conf.isValidateDownloads());
				
				switch (res)
				{
					case 0 : strcpy(descriptor->information,"Download error");break;
					case 1 : strcpy(descriptor->information,"Download finished");break;
					case 2 : strcpy(descriptor->information,"Download paused");break;
				}
				descriptor->update();
				descriptor->runningTask=0;
				getTabBandwidth()->removeDownloader(descriptor->fileName,this);
				unregisterDownloaderBandwidthMonitor(cfId);
				
				if (isNewFile && !res)
				{
					// check if file was created 
					GCF * gcf=parseGCF(descriptor->path);
					if (((int)gcf)<=1)
					{ // no gcf or invalid
						char errorMsg[1000];
						sprintf(errorMsg,"File not available or not owned : %s",descriptor->fileName);
						MessageBox(NULL,errorMsg,"CF Toolbox",MB_OK|MB_ICONERROR);
						getCFManager()->files->remove(descriptor->fileId);
						getTabFiles()->removeFile(cfId);
					}
					else deleteGCF(gcf);
				}
			}
		}

		descriptor->runningTask=0;
		
		notifyTaskEnd();
 
		return 0;
	}
};

#endif // !defined(AFX_CFMANAGER_H__122364D2_BE88_4ABD_9124_0CB21882AF93__INCLUDED_)
