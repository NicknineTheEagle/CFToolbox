// CFManager.cpp: implementation of the CCFManager class.
//
//////////////////////////////////////////////////////////////////////
#include <process.h>
#include "socketTools.h"
#include "Tools.h"
#include "direct.h"
#include "stdafx.h"
#include "CFToolbox.h"
#include "CFManager.h"
#include "ConfigManager.h"
#include "Debug.h"
#include "SteamNetwork.h"
#include "Registry.h"
#include "cdr.h"
#include "gcf.h"
#include "CFDescriptor.h"
#include "AppDescriptor.h"
#include "CSClient.h"
#include "Task.h"
#include "ProgressPopup.h"
#include "launcher.h"
#include "CFToolboxDlg.h"
#ifdef _DEBUG

#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#include "Bandwidth.h"
#include "TabBandwidth.h"

#define WINUI_ID 7
#define MEDIAPLAYER_ID 910

CCFManager * cfMngr=0;
 
class CPluginExecutor : public CTask
{
	char * commandLine;
	char * currentFolder;

	CCFDescriptor * file;
	CAppDescriptor * app;
	char * pluginName;
	bool refreshOnExit;
	bool asynchronous;

	public : CPluginExecutor(char * name,char * _commandLine,CCFDescriptor * _descriptor, bool _asynchronous,bool _refreshOnExit,bool _close) : CTask ()
	{
		refreshOnExit=_refreshOnExit;
		asynchronous=_asynchronous;
		close=_close;
		app=0;
		file=_descriptor;
		file->runningTask=this;
		cfId=_descriptor->fileId;
		
		if(_commandLine[2]==':' || _commandLine[2]=='\\')
		{
			// a X:\xxxxx or \xxxx  command line, specifying an absolute path
			commandLine=(char*)malloc(strlen(_commandLine)+1);
			strcpy(commandLine,_commandLine);
			currentFolder=0;
		}
		else
		{
					// relative path
			

			char path[2000];
			_getcwd(path,1000);
			strcat(path,"\\plugins\\");
			
			char * toRemove=path+strlen(path);

			strcat(path,_commandLine);
			/*char * cf=path;
			if (*cf=='"')
			{
				cf++;
				char * closing=strchr(cf,'"');
				if (closing) *closing=0;
			}
			else
			{
				char * closing=strchr(cf,' ');
				if (closing) *closing=0;
			}
			
				char * closing=strrchr(cf,'\\');
			if (closing) *closing=0;

			*/
			toRemove=strchr(toRemove,' ');
			if (toRemove) *toRemove=0;
			toRemove=strrchr(path,'\\');
			*toRemove=0;

		
 			/*commandLine=(char*)malloc(strlen(_commandLine)+1-strlen(toRemove));
			strcpy(commandLine,_commandLine+strlen(toRemove)+1);
			*/
			commandLine=(char*)malloc(strlen(_commandLine)+1+8);
			sprintf(commandLine,"plugins\\%s",_commandLine);

			currentFolder =(char*)malloc(strlen(path)+1);
			strcpy(currentFolder,path);
		}
		pluginName=(char*)malloc(strlen(name)+1);
		strcpy(pluginName,name);

		
		strcpy(file->information,"");
	}
	CPluginExecutor(char * name,char * _commandLine,CAppDescriptor * _descriptor, bool _asynchronous, bool _refreshOnExit,bool _close) : CTask ()
	{
		asynchronous=_asynchronous;
		refreshOnExit=_refreshOnExit;
		close=_close;
		file=0;
		app=_descriptor;
		//file->runningTask=this;
		cfId=_descriptor->appId;
		
		if(_commandLine[2]==':' || _commandLine[2]=='\\')
		{
			// a X:\xxxxx or \xxxx  command line, specifying an absolute path
			commandLine=(char*)malloc(strlen(_commandLine)+1);
			strcpy(commandLine,_commandLine);
			currentFolder=0;
		}
		else
		{
			// relative path
			

			char path[2000];
			_getcwd(path,1000);
			strcat(path,"\\plugins\\");
			
			char * toRemove=path+strlen(path);

			strcat(path,_commandLine);
			/*char * cf=path;
			if (*cf=='"')
			{
				cf++;
				char * closing=strchr(cf,'"');
				if (closing) *closing=0;
			}
			else
			{
				char * closing=strchr(cf,' ');
				if (closing) *closing=0;
			}
			
				char * closing=strrchr(cf,'\\');
			if (closing) *closing=0;

			*/
			toRemove=strchr(toRemove,' ');
			if (toRemove) *toRemove=0;
			toRemove=strrchr(path,'\\');
			*toRemove=0;

		
 			/*commandLine=(char*)malloc(strlen(_commandLine)+1-strlen(toRemove));
			strcpy(commandLine,_commandLine+strlen(toRemove)+1);
			*/
			commandLine=(char*)malloc(strlen(_commandLine)+1+8);
			sprintf(commandLine,"plugins\\%s",_commandLine);

			currentFolder =(char*)malloc(strlen(path)+1);
			strcpy(currentFolder,path);
		}

		pluginName=(char*)malloc(strlen(name)+1);
		strcpy(pluginName,name);

		strcpy(app->information,"");
	}
	~CPluginExecutor()
	{
		 if (commandLine) free(commandLine);
 		 if (currentFolder) free(currentFolder);
 
	}
	 virtual DWORD Run( LPVOID /* arg */ )
	{ 
		bool ok=0;
		if (pleaseStop)
		{
			close=0;
			if (file) sprintf(file->information,"%s interrupted",pluginName);
			if (app) sprintf(app->information,"%s interrupted",pluginName);
		}
		else
		{
			sprintf( taskName,pluginName);
			
			printDebug(DEBUG_LEVEL_DEBUG,"Launcher","Executing",commandLine,0);
 
				//CreateProcess(,commandLine,0,0,false,CREATE_DEFAULT_ERROR_MODE,0,"plugins/browser",0,0)
			STARTUPINFO si;
			PROCESS_INFORMATION pi;

			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			ZeroMemory( &pi, sizeof(pi) );

			ok=CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, currentFolder, &si, &pi); 

			if (!ok) //createArchive(descriptor->path,outPath,"CF Patcher", 0,0,0))
			{
				if (file) sprintf(file->information,"%s launch error",pluginName);
				if (app) sprintf(app->information,"%s launch error",pluginName);
				printDebug(DEBUG_LEVEL_ERROR,"Launcher","Launch error",GetLastError() ,0);
				close=0;
			}
			else
			{
 
				// Wait until child process exits.
					if (!asynchronous) WaitForSingleObject( pi.hProcess, INFINITE );
	
				// Close process and thread handles. 
					CloseHandle( pi.hProcess );
					CloseHandle( pi.hThread );


					if (file) sprintf(file->information,"%s executed",pluginName);
					if (app) sprintf(app->information,"%s executed",pluginName);
					printDebug(DEBUG_LEVEL_DEBUG,"Launcher","Executed",pluginName,0);
			}
		}
 			if (refreshOnExit)
			{
				if (file) file->update();
				if (app) app->update();
			}

		if (file) file->runningTask=0;

		notifyTaskEnd();
		

		return 0;
	}
};

class CMiniCFCreator : public CTask
{
	CCFDescriptor * descriptor;
	char * outPath;
 
	 
	public : CMiniCFCreator(CCFDescriptor * _descriptor, char * _outPath) : CTask ()
	{
	 
		descriptor=_descriptor;
		descriptor->runningTask=this;
		cfId=descriptor->fileId;
		strcpy(descriptor->information,"");
		outPath=(char*)malloc(strlen(_outPath)+1);
		strcpy(outPath,_outPath);
		strcpy(descriptor->information,"");
	}
	~CMiniCFCreator()
	{
		free(outPath);
 
	}
	 virtual DWORD Run( LPVOID /* arg */ )
	{ 
		
		if (pleaseStop)
			strcpy(descriptor->information,"Archive creation interrupted");
		else
		{
			strcpy( taskName,"Creating miniCF");
			GCF * gcf=parseGCF(descriptor->path);
			if ((int)gcf>1) 
			{
					ConfigManager conf;
			if (conf.isResetMiniCF())
			{
				GCF * oldGCF=gcf;
				
				long directoryLength=serializeGCFDirectory(0,gcf);
				char * directory=(char*)malloc(directoryLength);
				serializeGCFDirectory(directory,gcf);
				
				long checksumLength=serializeGCFChecksums(0,gcf);
				char * checksums=(char*)malloc(checksumLength);
				serializeGCFChecksums(checksums,gcf);


				gcf=parseGcf(directory,checksums,oldGCF->cfType,oldGCF->cfVersion);

				deleteGCF(oldGCF);

				 free(directory);
				 free(checksums);
			}
		

			char installDirName[1000];
			strcpy(installDirName,descriptor->fileName);
			*strrchr(installDirName,'.')=0;
			strcat(installDirName,".mini");
			if (createGcfFile(gcf,conf.getOutputPath(),installDirName,descriptor->commonPath,descriptor->fileName,0))
				strcpy(descriptor->information,"MiniGCF creation error");
			else
				strcpy(descriptor->information,"MiniGCF created");
			deleteGCF(gcf);
			}
			else
				strcpy(descriptor->information,"MiniGCF creation error");
			descriptor->runningTask=0;
		}
		descriptor->runningTask=0;

		notifyTaskEnd();
		return 0;
	}
};


class CCFArchiver : public CTask
{
	CCFDescriptor * descriptor;
	char * outPath;
	bool correct;
	 
	public : CCFArchiver(CCFDescriptor * _descriptor, char * _outPath, bool _correct) : CTask ()
	{
		correct=_correct;
		descriptor=_descriptor;
		descriptor->runningTask=this;
		cfId=descriptor->fileId;
		strcpy(descriptor->information,"");
		outPath=(char*)malloc(strlen(_outPath)+1);
		strcpy(outPath,_outPath);
		strcpy(descriptor->information,"");
	}
	~CCFArchiver()
	{
		free(outPath);
 
	}
	 virtual DWORD Run( LPVOID /* arg */ )
	{ 
		
		if (pleaseStop)
			strcpy(descriptor->information,"Archive creation interrupted");
		else
		{
			if (correct)
			{
				strcpy( taskName,"Creating archive (pre-validating)");
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
			strcpy( taskName,"Creating archive");
			if (createArchive(descriptor->path,outPath,"CF Patcher", 0,0,0))
				strcpy(descriptor->information,"Archive creation error");
			else
				strcpy(descriptor->information,"Archive created");
			descriptor->runningTask=0;
		}
		descriptor->runningTask=0;

		notifyTaskEnd();
		return 0;
	}
};

class CCFUpdater : public CTask
{
	CCFDescriptor * descriptor;
	char * update;
	 
	public : CCFUpdater(CCFDescriptor * _descriptor, char * _update) : CTask ()
	{
		descriptor=_descriptor;
		descriptor->runningTask=this;
		cfId=descriptor->fileId;
	 
		update=(char*)malloc(strlen(_update)+1);
		strcpy(update,_update);
		strcpy(descriptor->information,"");
	}
	~CCFUpdater()
	{
		free(update);
 
	}
	 virtual DWORD Run( LPVOID /* arg */ )
	{ 
		ConfigManager conf;
		if (descriptor->isMoreThan4GB)
		{
			strcpy(descriptor->information,"4GB+ files not supported");
			descriptor->update();
			descriptor->runningTask=0;
			notifyTaskEnd();
			return 0;
		}
		if (!pleaseStop)
		{
			bool ok=true;
			if (_getDiskFreeSpace(conf.getSteamAppsPath())<1024) 
			{
				// less than 1 Gb space alert before downloading.
				if (MessageBox(NULL,"Less than 1GB free space on the GCF / NCF Storage path. Proceed anyway ?","CF Toolbox",MB_ICONQUESTION|MB_YESNO)==IDNO)
					ok=false;
			}
			
			if (ok)
			{
				if (applyUpdate(update,descriptor->path,descriptor->commonPath,descriptor->decryptionKey,"CF patcher",0,taskName,&taskProgress,&pleaseStop,conf.isSecuredGCFUpdates()))
					strcpy(descriptor->information,"Error during update");
				else
					strcpy(descriptor->information,"File updated");
			}
		}
		if (pleaseStop)
			strcpy(descriptor->information,"Update interrupted");
		
		descriptor->update();
		descriptor->runningTask=0;
		notifyTaskEnd();
		return 0;
	}
};


class CCFUpdateMaker : public CTask
{
	CCFDescriptor * descriptor;
	char * archive;
	char * outPath;
	bool correct;
	public : CCFUpdateMaker(CCFDescriptor * _descriptor,  char * _archive, char * _outPath,bool _correct) : CTask ()
	{
		correct=_correct;
		descriptor=_descriptor;
		descriptor->runningTask=this;
		cfId=descriptor->fileId;

		archive=(char*)malloc(strlen(_archive)+1);
		strcpy(archive,_archive);
		outPath=(char*)malloc(strlen(_outPath)+1);
		strcpy(outPath,_outPath);
		strcpy(descriptor->information,"");
	}
	 ~CCFUpdateMaker() 
	{
		free(archive);
		free(outPath);
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
		if (!pleaseStop)
		{

			if (correct)
			{
				strcpy(taskName,"Creating patch (pre-validating)");
				char root[1000];
				strcpy(root,descriptor->path);
				root[strlen(root)-(strlen(descriptor->fileName)+1)]=0;

				GCF * gcf=parseGCF(descriptor->path);
			
				if ((int)gcf>1) 
				{
					validate(root,descriptor->commonPath,gcf,true,descriptor->fileName,DEBUG_LEVEL_DEBUG,taskName,&taskProgress,&pleaseStop);
					deleteGCF(gcf);
				}
			}
			strcpy(taskName,"Creating patch");
			ConfigManager conf;
			if (createUpdateFile(archive,descriptor->path,descriptor->commonPath,outPath,"CF patcher",0,taskName,&taskProgress,&pleaseStop,conf.isSecuredGCFUpdates()))
				strcpy(descriptor->information,"Error during update creation");
			else
				strcpy(descriptor->information,"Update created");
		}	
			

		if (pleaseStop)
			strcpy(descriptor->information,"Update creation interrupted");
		descriptor->update();
		descriptor->runningTask=0;
		notifyTaskEnd();
		return 0;
	}
};

class CCFValidator : public CTask
{
	CCFDescriptor * descriptor;
 
	int correct;
public : CCFValidator(CCFDescriptor * _descriptor, int _correct) : CTask ()
	{
		descriptor=_descriptor;
		descriptor->runningTask=this;
		cfId=descriptor->fileId;
 
		correct=_correct;
		strcpy(descriptor->information,"");
	}
 
 
	virtual DWORD Run( LPVOID /* arg */ )
	{ 
		GCF * gcf=0;
		if (descriptor->isMoreThan4GB)
		{
			strcpy(descriptor->information,"4GB+ files not supported");
			descriptor->update();
			descriptor->runningTask=0;
			notifyTaskEnd();
			return 0;
		}
		if (!pleaseStop)
		{
			char root[1000];
			strcpy(root,descriptor->path);
			root[strlen(root)-(strlen(descriptor->fileName)+1)]=0;

			gcf=parseGCF(descriptor->path);
			int ok=0;
			if ((int)gcf>1) ok=validate(root,descriptor->commonPath,gcf,correct,descriptor->fileName,DEBUG_LEVEL_DEBUG,taskName,&taskProgress,&pleaseStop);
			if (!ok)
			{
				if (correct)
					strcpy(descriptor->information,"File has been corrected and must be updated");
				else
					strcpy(descriptor->information,"Validation failed");
			}
			else
				strcpy(descriptor->information,"File is valid");
		}
		if (pleaseStop)
			strcpy(descriptor->information,"Validation interrupted");

		if ((int)gcf>1) deleteGCF(gcf);
		descriptor->update();
		descriptor->runningTask=0;
		notifyTaskEnd();
		return 0;
	}
};


void initCFManager()
{
	if (cfMngr) return;
	cfMngr=new CCFManager();
}

CCFManager * getCFManager()
{
	if (!cfMngr) initCFManager();
	return cfMngr;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCFManager::CCFManager()
{
	files= new HashMapDword();
	apps= new HashMapDword();
	cdrAvailableFiles= new HashMapDword(HASHMAP_NODELETE);
	if (((CCFToolboxDlg*)getMainDialog())->canceledStart) return;
	refreshContent();
}

CCFManager::~CCFManager()
{
	delete files;
	delete apps;
	delete cdrAvailableFiles;
}

void CCFManager::scanGcf(ContentDescriptionRecord cdr, CDRApp app, char * fileName, char * fullPath)
{
		GCF * gcf = parseGCF(fullPath);
		if ((int)gcf>1)
		{
			if (gcf->appId==app.appId) addFile(cdr,app,fileName, gcf); // some files with same name but different ID
			deleteGCF(gcf);
		}
}

int CCFManager::refreshContent()
{
	purgeFiles();
	ConfigManager conf;
	char * path=conf.getSteamAppsPath();
	printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Refreshing GCF / NCF storage folder content",path,0);
	


	// check if folder exists
	while(!folderExists(path))
	{
		CString folder;
		if (BrowseForFolder(NULL,"GCF / NCF storage path",folder))
		{
			conf.setSteamAppsPath(folder.GetBuffer(0));
			path=conf.getSteamAppsPath();
		}
	/*	else
			getMainDialog()->SendMessage(WM_CLOSE, 0, 0 );
*/
	}
	// checking also for output path
	if (!folderExists(conf.getOutputPath()))
	{
		conf.setOutputPath(conf.getSteamAppsPath());
	}

	// parse CDR and search files
	SteamNetwork network;
	/*char * cdrBlob=network.getContentDescriptionRecord();
	if (!cdrBlob) 
	{
		printDebug(DEBUG_LEVEL_ERROR,"CF manager","ContentDescriptionRecord not available",0);
		return 0;
	}

	printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Parsing ContentDescriptionRecord",0);
	Registry reg;
	RegistryVector * cdrVector=reg.parseVector(cdrBlob);
*/
	printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Parsing ContentDescriptionRecord",0);
	RegistryVector * cdrVector=network.getContentDescriptionRecord();

	if (!cdrVector) 
	{
		printDebug(DEBUG_LEVEL_ERROR,"CF manager","ContentDescriptionRecord not available",0);
		return 0;
	}

	ContentDescriptionRecord cdr=getCDR(cdrVector);

	cdrAvailableFiles->clear();
	
	// search CDR subscriptions for referenced gcf/ncf ids (some can be unavailable)
	RegistryVector * subscriptions=cdrVector->getNode(2)->getVectorValue();
	unsigned int nbSubs=subscriptions->getNbNodes();
	for (unsigned int p=0;p<nbSubs;p++)
	{
		RegistryVector * sub=subscriptions->getNodeAt(p)->getVectorValue();
	 
		RegistryNode * disabledNode=sub->getNode(19);
		if (!disabledNode)
		{
 				
				RegistryVector * apps=sub->getNode(6)->getVectorValue();
		 		for (unsigned int a=0;a<apps->getNbNodes();a++)
				{
					DWORD gcfId=apps->getNodeAt(a)->getDwordDescription();//ValueType();//getDWordValue();
					cdrAvailableFiles->put(gcfId,(void*)1);
				}
		}
	 
	}
	
	
	
	char fullPath[1000];
	char fileName[1000];

	CProgressPopup progress;
	progress.m_message="Refreshing GCF / NCF storage folder content";
	progress.Create(IDD_PROGRESS);
	progress.m_progress.SetRange32(0,getCDRNbApps(cdr));
	progress.m_progress.SetPos(0);

	for (int ind=0;ind<getCDRNbApps(cdr);ind++)
	{
		CDRApp app=getCDRAppAt(cdr,ind);

	//	RegistryNode * node14=app.node->getVectorValue()->getNode(14);
	/*	if (!node14 || 
			(
			 !(node14->getVectorValue()->getNode("gamedir") && 
			 strlen(node14->getVectorValue()->getNode("developer")->getStringValue())>0)
			)
		   )*/
		 
		if (!app.manifestOnlyApp || app.appOfManifestOnlyCache==app.appId || app.appId==1273/*killing floor beta*/)
		{

			strcpy(fileName,app.installDirName);
			strlwr(fileName);
			
			if (app.appOfManifestOnlyCache)
				strcat(fileName,".ncf");
			else
				strcat(fileName,".gcf");
		
			printDebug(DEBUG_LEVEL_EVERYTHING,"CF manager","Searching for",fileName,0);
			sprintf(fullPath,"%s\\%s",path,fileName);
			scanGcf(cdr,app,fileName,fullPath);
		}
 		progress.m_progress.SetPos(ind);
	}
	buildApps(cdr);
	progress.DestroyWindow(); 
//	delete cdrVector;

	return 0;
}


void CCFManager::purgeFiles()
{
	files->clear();
}

void CCFManager::addFile(ContentDescriptionRecord cdr, CDRApp app, char * fileName, GCF *gcf)
{

	char dbg[1000];
	sprintf(dbg,"%s (version: %d)",fileName,gcf->appVersion);
	printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Found",dbg,0);

	if (gcf->appId!=app.appId)
	{
		sprintf(dbg,"%s (id: %d) (CDR id: %d)",fileName,gcf->appId,app.appId);
		//printDebug(DEBUG_LEVEL_ERROR,"CF manager","AppID does not match",dbg,0);
		return; 
	}

	if (gcf->appVersion>app.currentVersionId)
	{
	//	if (gcf->appVersion>app.betaVersionId)
		{
			sprintf(dbg,"%s (version: %d) (CDR version: %d)",fileName,gcf->appVersion,app.currentVersionId);
			printDebug(DEBUG_LEVEL_ERROR,"CF manager","Version is greater that in CDR",dbg,0);
			return; 
		}
	}

	if (gcf->appVersion<app.currentVersionId)
	{
		sprintf(dbg,"%s (version: %d) (CDR version: %d)",fileName,gcf->appVersion,app.currentVersionId);
		printDebug(DEBUG_LEVEL_INFO,"CF manager","Outdated",dbg,0);
	}
	ConfigManager conf;
	CCFDescriptor * descriptor=new CCFDescriptor(cdr,app,fileName,gcf,conf.isAccurateNcfCompletion());
	/*if (gcf->appVersion>app.currentVersionId)
	{
		strcat(descriptor->fileName," *BETA*");
	}*/
	files->put(gcf->appId,descriptor);

}

void CCFManager::addApp(ContentDescriptionRecord cdr,CDRApp app, HashMapDword * files,DWORD * appGcfIds,char * appGcfOptional,int appGcfIdsCount	)
{

	CAppDescriptor * descriptor=new CAppDescriptor(cdr,app,files,appGcfIds,appGcfOptional,appGcfIdsCount);
	apps->put(app.appId,descriptor);

}


void CCFManager::makeArchive(DWORD id, char * outPath, bool correct)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(id);
	
	CTask * archiver= new CCFArchiver(descriptor, outPath,correct);
	archiver->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	archiver->taskName=(char*)malloc(200);
	strcpy(archiver->taskFile,descriptor->fileName);
	strcpy(archiver->taskName,"Creating archive");
	getTaskManager()->addAndStartTask(archiver);	
}

void CCFManager::makeUpdate(DWORD id, char * archive, char * outPath, bool correct)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(id);
	
	CTask * updater= new CCFUpdateMaker(descriptor, archive,outPath,correct);
	updater->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	updater->taskName=(char*)malloc(200);
	strcpy(updater->taskFile,descriptor->fileName);
	strcpy(updater->taskName,"Creating patch");
	getTaskManager()->addAndStartTask(updater);	
}

void CCFManager::apply_Update(DWORD id, char * update)
{

	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(id);
	CTask * updater= new CCFUpdater(descriptor, update);
	updater->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	updater->taskName=(char*)malloc(200);
	strcpy(updater->taskFile,descriptor->fileName);
	strcpy(updater->taskName,"Patching");
	getTaskManager()->addAndStartTask(updater);	
}

void CCFManager::openFile(DWORD id)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(id);
	ShellExecute(AfxGetMainWnd()->m_hWnd, "open", descriptor->path, NULL, NULL, SW_SHOWNORMAL);
}

void CCFManager::validate(DWORD cfId)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(cfId);

	CTask * validator= new CCFValidator(descriptor,0);
	validator->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	validator->taskName=(char*)malloc(200);
	strcpy(validator->taskFile,descriptor->fileName);
	strcpy(validator->taskName,"Validating");
	getTaskManager()->addAndStartTask(validator);	

}
void CCFManager::correct(DWORD cfId)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(cfId);

	CTask * validator= new CCFValidator(descriptor,1);
	validator->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	validator->taskName=(char*)malloc(200);
	strcpy(validator->taskFile,descriptor->fileName);
	strcpy(validator->taskName,"Correcting");
	getTaskManager()->addAndStartTask(validator);	

}

void CCFManager::createMiniCF(DWORD cfId, char * outPath)
{

	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(cfId);

	if (descriptor->isNcf) return;

	CTask * miniCfCreator= new CMiniCFCreator(descriptor,outPath);
	miniCfCreator->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	miniCfCreator->taskName=(char*)malloc(200);
	strcpy(miniCfCreator->taskFile,descriptor->fileName);
	strcpy(miniCfCreator->taskName,"Creating miniGCF");
	getTaskManager()->addAndStartTask(miniCfCreator);	

}

void CCFManager::download(DWORD cfId, char* ticket, int ticketLen,bool correct)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(cfId);
		
	CTask * downloader= new CCFDownloader(descriptor,ticket, ticketLen,correct,false);
	downloader->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	downloader->taskName=(char*)malloc(200);
	strcpy(downloader->taskFile,descriptor->fileName);
	strcpy(downloader->taskName,"Downloading");
	getTaskManager()->addAndStartTask(downloader);	

}
void CCFManager::downloadNew(DWORD cfId, char* ticket, int ticketLen )
{
	//CCFDescriptor * descriptor=(CCFDescriptor *)files->get(cfId);
	SteamNetwork network;
	ConfigManager conf;
	ContentDescriptionRecord cdr=getCDR(network.getContentDescriptionRecord());
	CDRApp app=getCDRApp(cdr,cfId);
	if (!app.node) return;
	CCFDescriptor * descriptor=new CCFDescriptor(cdr,app,conf.getSteamAppsPath(),conf.isAccurateNcfCompletion());
	getCFManager()->files->put(descriptor->fileId,descriptor);
	getTabFiles()->insertFile(descriptor);


	CTask * downloader= new CCFDownloader(descriptor,ticket, ticketLen,false,true);
	downloader->taskFile=(char*)malloc(strlen(descriptor->fileName)+1);
	downloader->taskName=(char*)malloc(200);
	strcpy(downloader->taskFile,descriptor->fileName);
	strcpy(downloader->taskName,"Downloading");
	getTaskManager()->addAndStartTask(downloader);	

}
void CCFManager::cancelTask(DWORD id)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(id);
	if (!descriptor) return;
	if (!descriptor->runningTask) return;
//	sprintf(descriptor->information,"Stopping : %s",descriptor->runningTask->taskName);
	if (descriptor->runningTask->getTaskStatus()==TASK_STATUS_NO_STOP)
	{
		sprintf(descriptor->information,"Please wait : %s",descriptor->runningTask->taskName);
	}
	else
	{
		sprintf(descriptor->information,"Stopping : %s",descriptor->runningTask->taskName);
		descriptor->runningTask->AskToStop();
	}
}

bool CCFManager::deleteFile(DWORD cfId)
{
	CCFDescriptor * descriptor=(CCFDescriptor *)files->get(cfId);
	if (!descriptor) return false;
	if (descriptor->runningTask) return false;


	if (descriptor->isNcf)
	{
		GCF * gcf=parseGCF(descriptor->path);
		if ((int)gcf>1)
		{
			char path[1000];
			ConfigManager conf;
			char absolutePath[1000];
			for (int ind=gcf->directory->itemCount-1;ind>=0;ind--)
			{
				getFilePath(gcf,ind,path);
				sprintf(absolutePath,"%s\\common\\%s%s",conf.getSteamAppsPath(),descriptor->commonPath,path);
				GCFDirectoryEntry toDelete=gcf->directory->entry[ind];
				if (toDelete.directoryType)
				{
					if (DeleteFile(absolutePath))
						printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Deleted",absolutePath,0);
					else
						printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Can't delete",absolutePath,0);
				}
				else
				{
					if (RemoveDirectory(absolutePath))
						printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Deleted",absolutePath,0);
					else
						printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Can't delete",absolutePath,0);
				}
			}
			deleteGCF(gcf);
		}
	}

	if (DeleteFile(descriptor->path))
	{
		printDebug(DEBUG_LEVEL_INFO,"CF manager","Deleted",descriptor->path,0);
		files->remove(cfId);
		return true;
	}
	else
		printDebug(DEBUG_LEVEL_ERROR,"CF manager","Can't delete",descriptor->path,0);
	return false;
}
void CCFManager::refreshApps( ) 
{

		// parse CDR and search files
	SteamNetwork network;
	/*
	char * cdrBlob=network.getContentDescriptionRecord();
	if (!cdrBlob) 
	{
		printDebug(DEBUG_LEVEL_ERROR,"CF manager","ContentDescriptionRecord not available",0);
		return ;
	}

	printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Parsing ContentDescriptionRecord",0);
	Registry reg;
	RegistryVector * cdrVector=reg.parseVector(cdrBlob);
*/
	printDebug(DEBUG_LEVEL_DEBUG,"CF manager","Parsing ContentDescriptionRecord",0);
	RegistryVector * cdrVector=network.getContentDescriptionRecord();
	if (!cdrVector) 
	{
		printDebug(DEBUG_LEVEL_ERROR,"CF manager","ContentDescriptionRecord not available",0);
		return ;
	}
	ContentDescriptionRecord cdr=getCDR(cdrVector);
	buildApps(cdr);
//	delete cdrVector;
}

#define MAX_APPS 2000 // /should be enough

void CCFManager::buildApps(ContentDescriptionRecord cdr)
{
	DWORD appGcfIds[MAX_APPS]; 
	char appGcfOptional[MAX_APPS];

	int appGcfIdsCount=0;
	apps->clear();
	ConfigManager conf;
	int cnt=getCDRNbApps(cdr);
	for (int ind=0;ind<cnt;ind++)
	{

	/*
		char info[100]; 
		sprintf(info,"processing app %u",ind);
		printDebug(1000000,"loader",info,1);
	*/
		CDRApp app=getCDRAppAt(cdr,ind);
		// filter on apps



		RegistryVector * userDefined=getCDRAppUserDefinedData(app);
		//if (!getCDRAppNbFilesystem(app) && (userDefined && userDefined->getNode("gamedir"))) printDebug(DEBUG_LEVEL_TODO,"test","has FS but no gamedir",app.installDirName,0);
	    if (userDefined)
		{
		 	if (userDefined->getNode("gamedir"))
			{
				if (!userDefined->getNode("state") || (stricmp(userDefined->getNode("state")->getStringValue(),"eStateUnavailable")
												    &&	stricmp(userDefined->getNode("state")->getStringValue(),"eStateComingSoonNoPreload")))
				{
					appGcfIdsCount=0;

					// search for localisations
					CDRApp localisedApp=getCDRLocalisedApp(cdr,app,conf.getLanguage());
					if (localisedApp.node)
					{
						// found a localised GCF
						if (cdrAvailableFiles->get(localisedApp.appId))
						{ // only if gcf/ncf is registered somewhere in the subscriptions
							appGcfOptional[appGcfIdsCount]=0;
							appGcfIds[appGcfIdsCount++]=localisedApp.appId;
						}

						// fix 2012-12-06 so it wont crash like shit
						if (appGcfIdsCount==MAX_APPS) 
						{
							appGcfIdsCount--;
							MessageBox(NULL,"Too many apps in CDR, some may be ignored","CF Toolbox",MB_ICONERROR);
							break;
						}

					}
 
					// add "common" apps
					int nbGcfs=getCDRAppNbFilesystem(app);
					for (int i=0;i<nbGcfs;i++)
					{
						CDRAppFilesystem fileSystem=getCDRAppFilesystem(app,i);
					//	if (cdrAvailableFiles->get(fileSystem.appId)) // may not be intresting.
						{ // only if gcf/ncf is registered somewhere in the subscriptions
							appGcfOptional[appGcfIdsCount]=(fileSystem.isOptional ? 1 : 0);
							appGcfIds[appGcfIdsCount++]=fileSystem.appId;
						}
						// fix 2012-12-06 so it wont crash like shit
						if (appGcfIdsCount==MAX_APPS) 
						{
							appGcfIdsCount--;
							MessageBox(NULL,"Too many apps in CDR, some may be ignored","CF Toolbox",MB_ICONERROR);
							break;
						}

					}

					for (int i=0;i<appGcfIdsCount;i++)
					{
						if (appGcfIds[i]!=WINUI_ID && appGcfIds[i]!=MEDIAPLAYER_ID && files->get(appGcfIds[i]))
						{// if at least one gcf found , app is visible
						//	CDRApp part=getCDRApp(cdr,fileSystem.appId);
							addApp(cdr,app,files,appGcfIds,appGcfOptional,appGcfIdsCount);
							break;
						}

					}

				}
			}
		}
	}
}
void CCFManager::executePlugin(CPlugin * plugin, CCFDescriptor * desc)
{
	if (!plugin || !desc) return;

	if (plugin->closeCFToolbox && getTaskManager()->nbTotalTasks)
	{
		printDebug(DEBUG_LEVEL_ERROR,desc->fileName,"Plugin requires to close CF Toolbox, but tasks are currently running.Execution aborted",plugin->name,0);
		return;
	}

	char * templateCommandLine=plugin->getCommandLine(desc);
	if (!templateCommandLine) return;
	char commandLine[5000];
	if (!buildFileCommandLine(commandLine,templateCommandLine,desc->fileId,desc)) return;
 
	CTask * executor= new CPluginExecutor(plugin->name,commandLine,desc,plugin->asynchronous,plugin->updatesFiles,plugin->closeCFToolbox);
	executor->taskFile=(char*)malloc(strlen(desc->fileName)+1);
	executor->taskName=(char*)malloc(200);
	strcpy(executor->taskFile,desc->fileName);
	strcpy(executor->taskName,plugin->name);
	getTaskManager()->addAndStartTask(executor);	


}
void CCFManager::executePlugin(CPlugin * plugin, CAppDescriptor * desc)
{
	if (!plugin || !desc) return;
	
	if (plugin->closeCFToolbox && getTaskManager()->nbTotalTasks)
	{
		printDebug(DEBUG_LEVEL_ERROR,desc->appName,"Plugin requires to close CF Toolbox, but tasks are currently running.Execution aborted",plugin->name,0);
		return;
	}

	
	char * templateCommandLine=plugin->getCommandLine(desc);
	if (!templateCommandLine) return;
	char commandLine[5000];
	if (!buildAppCommandLine(commandLine,templateCommandLine,desc->appId,desc)) return;
	
	CTask * executor= new CPluginExecutor(plugin->name,commandLine,desc,plugin->asynchronous,plugin->updatesFiles,plugin->closeCFToolbox);
	executor->taskFile=(char*)malloc(strlen(desc->appName)+1);
	executor->taskName=(char*)malloc(200);
	strcpy(executor->taskFile,desc->appName);
	strcpy(executor->taskName,plugin->name);
	getTaskManager()->addAndStartTask(executor);
}