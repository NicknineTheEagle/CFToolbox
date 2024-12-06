// TaskManager.cpp: implementation of the CTaskManager class.
//
//////////////////////////////////////////////////////////////////////
#include "debug.h"
#include "stdafx.h"
#include "CFToolbox.h"
#include "CFToolboxDlg.h"
#include "TaskManager.h"
#include "Thread.h"
#include "CFDescriptor.h"
#include "CFManager.h"
#include "ConfigManager.h"
#include "tools.h"
#include "AppDescriptor.h"
#include "Bandwidth.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define NB_COLS 6 // also in TabFiles
#define NB_APPS_COLS 6 // also in TabApps


CTaskManager * taskMngr=0;

void buildListColumns(CListCtrl* list, char ** names,int * width,int nbCols);
int insertListValue(CListCtrl* list, int id, char ** values,int nbCols,int iconId=0);
int updateListValue(CListCtrl* list, int index, char ** values,int nbCols);
CRITICAL_SECTION tCs; 
void TLock()
{

	/* Enter the critical section -- other threads are locked out */
	EnterCriticalSection(&tCs);
}
 
void TUnlock()
{
	/* Leave the critical section -- other threads can now EnterCriticalSection() */
	LeaveCriticalSection(&tCs);
}
void initTaskManager()
{
	/* Initialize the critical section -- This must be done before locking */
	InitializeCriticalSection(&tCs);
	
	if (taskMngr) return;
	taskMngr=new CTaskManager();
}

CTaskManager * getTaskManager(int doNotInstanciate)
{
	if (!taskMngr && !doNotInstanciate) initTaskManager();
	return taskMngr;
}
/*
int __Tlock__=0;

void TLock()
{
	while (1)
	{
		if (!__Tlock__)
		{
			__Tlock__++;
			return;
		}

		Sleep(100);
	}
}

void TUnlock()
{
	__Tlock__--;
}
*/


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTaskManager::CTaskManager()
{
	if (((CCFToolboxDlg*)getMainDialog())->canceledStart) return;
	nbRunningTasks=0;
	nbTotalTasks=0;
	filesDlg=0;
	filesCtrl=0;
	appsDlg=0;
	appsCtrl=0;
	tasksDlg=0;
	tasksCtrl=0;
	tasks=new HashMapDword(HASHMAP_NODELETE);
	finishedTasks=new HashMapDword(HASHMAP_NODELETE);
	displayUpdater=new CTaskManagerUpdater();
	displayUpdater->Start();
	steamMonitor=new CSteamMonitor();
	steamMonitor->Start();
	getBandwidthMonitor(); // start it
}

CTaskManager::~CTaskManager()
{
	if (((CCFToolboxDlg*)getMainDialog())->canceledStart) return;
	displayUpdater->stop=1;
	steamMonitor->stop=1;
	Sleep(1000);
	displayUpdater->Stop();
	steamMonitor->Stop();
	
	Sleep(100);
 
//	delete steamMonitor; // else crashing
//	delete displayUpdater; // else crashing
	

	killAllTasks();
	getBandwidthMonitor()->Stop();
	gc();
	delete tasks;
	delete finishedTasks;
		
	/* Release system object when all finished -- usually at the end of the cleanup code */
	DeleteCriticalSection(&tCs);
}

int CTaskManager::addAndStartTask(CTask *task)
{
//	TLock();
	tasks->put((DWORD)task,task);
	task->setTaskManager(this);
	nbTotalTasks++;
//	TUnlock();
	int res=0;
	ConfigManager conf;
	if (nbRunningTasks<conf.getMaxTasks())
	{	
	//	TLock();
		task->waiting=0;
		task->Start();
		nbRunningTasks++;
	//	TUnlock();
	}
	refreshDisplay(task);
	
	
	return res;
}

int CTaskManager::unregisterTask(CTask *task)
{
 
	TLock();
 
	bool close=task->close;
	tasks->remove((DWORD)task);
 
/*	int res=task->Stop();
	delete task; */
	finishedTasks->put((DWORD)task,task);
	nbRunningTasks--;
	nbTotalTasks--;
 
	TUnlock();
 
	

	if (close)
		{
			 getMainDialog()->SendMessage(WM_CLOSE, 0, 0 );
			//((CCFToolboxDlg*)getMainDialog())->OnClose();
		}
	refreshDisplay(task);
	return 0;
}

void CTaskManager::_stopAllTasks(int nowait)
{
	DWORD * keys;
	int nbTasks;
 
	TLock();
 
	nbTasks=tasks->size();
	keys=(DWORD*)malloc(4*nbTasks);
	
	nbTasks=tasks->getDWordKeys(keys);
 
	for (int ind=0;ind<nbTasks;ind++)
	{ // remove waiting ones
		CTask * task=(CTask *)keys[ind];
		if (task->waiting)
		{
			tasks->remove(keys[ind]);
			nbTotalTasks--;
			task->Stop();
			delete task;
		}
	//	task->Stop();
		//tasks->remove(keys[ind]);
	}
 
	nbTasks=tasks->getDWordKeys(keys);
 
	for (int ind=0;ind<nbTasks;ind++)
	{
		CTask * task=(CTask *)keys[ind];
 
		task->AskToStop();
	//	task->Stop();
		//tasks->remove(keys[ind]);
	}
	free(keys);
 
	TUnlock();
 
	// wait for all tasks to be destoyed
	if (!nowait)
	{
		while (tasks->size()) 
		{
			SleepEx(500,FALSE);
		}

		gc();
		nbRunningTasks=0;
		nbTotalTasks=0;
	}
	else
		gc();
}

void CTaskManager::killAllTasks()
{
	DWORD * keys;
	int nbTasks;
 
	TLock();
 
	nbTasks=tasks->size();
	keys=(DWORD*)malloc(4*nbTasks);
	
	nbTasks=tasks->getDWordKeys(keys);
 
	for (int ind=0;ind<nbTasks;ind++)
	{ // remove waiting ones
		CTask * task=(CTask *)keys[ind];
		CCFDescriptor * desc=(CCFDescriptor*)getCFManager()->files->get(task->cfId);
		if (desc) 
		{
			desc->runningTask=0;
			desc->update();
		}
//		task->AskToStop();
 
		task->Stop();
 
		tasks->remove(keys[ind]);
		nbTotalTasks--;
	 
		
		delete task;
	//	task->Stop();
		//tasks->remove(keys[ind]);
	}
	TUnlock();
	gc();
	nbRunningTasks=0;
	nbTotalTasks=0;
}


void CTaskManager::gc()
{
	TLock();
	DWORD * keys;
	int nbTasks;

	nbTasks=finishedTasks->size();
	keys=(DWORD*)malloc(4*nbTasks);
	nbTasks=finishedTasks->getDWordKeys(keys);
	for (int ind=0;ind<nbTasks;ind++)
	{
		finishedTasks->remove(keys[ind]);
		CTask * task=(CTask *)keys[ind];
		//task->Stop();
		delete task;
	}
	free(keys);
	TUnlock();
}

void CTaskManager::refreshDisplay()
{
	if (!tasksCtrl || !tasksDlg) return;

	CTask * tasks[1000];
	int nb=0;

	for (int ind=0;ind<tasksCtrl->GetItemCount();ind++)
	{	
		tasks[ind]=(CTask*)tasksCtrl->GetItemData(ind);
		nb++;
	}

	for (int ind=0;ind<nb;ind++)
	{
		refreshDisplay(tasks[ind]);
	}
	gc();
}

void CTaskManager::refreshDisplay(CTask * task)
{
	if (!tasksCtrl || !tasksDlg) return;
 	
	char * notext="";
	char * processing="Processing";
	char percent[20];
 
 	int running=(int)tasks->get((DWORD)task);
	int controlTaskIndex=-1;
	HashMapDword * updatedFiles=new HashMapDword(HASHMAP_NODELETE);
	for (int ind=0;ind<tasksCtrl->GetItemCount();ind++)
	{	
		CTask * tsk=(CTask*)tasksCtrl->GetItemData(ind);
		if (tsk==task)
		{
			controlTaskIndex=ind;
			break;
		}
	}
	if (!running)
	{
		// task was stopped

		if (controlTaskIndex!=-1) tasksCtrl->DeleteItem(controlTaskIndex);
		CTask * stoppedTsk=(CTask*) finishedTasks->get((DWORD)task);

		if (stoppedTsk)
		{
			refreshFilesDisplay(stoppedTsk->cfId);
			updatedFiles->put(stoppedTsk->cfId,getCFManager()->files->get(task->cfId));
		}
		
	}
	else
	{
		// task is running
		char tskName[300];
		if (task->waiting)
			sprintf(tskName,"Waiting (%s)",(task->taskName ?  task->taskName : processing));
		else
			strcpy(tskName,(task->taskName ?  task->taskName : processing));

		char * data[3];
		data[0]=(task->taskFile ?  task->taskFile : notext);
		data[1]=tskName;
		sprintf(percent,"%d%%",task->taskProgress);
		data[2]= percent;
	 
		if (controlTaskIndex==-1)
			insertListValue(tasksCtrl,(DWORD)task,data,3);
		else
			updateListValue(tasksCtrl,controlTaskIndex,data,3);
		refreshFilesDisplay(task->cfId);
		updatedFiles->put(task->cfId,getCFManager()->files->get(task->cfId));
	}

	if (updatedFiles->size())
	{
		// search for updated apps;
		CCFManager * cfManager=getCFManager();
		int nbApps=cfManager->apps->size();
		DWORD * apps=(DWORD *)malloc(4*nbApps);
		cfManager->apps->getDWordKeys(apps);
		for (int ind=0;ind<nbApps;ind++)
		{
			CAppDescriptor * desc=(CAppDescriptor *)cfManager->apps->get(apps[ind]);
			for (int cf=0;cf<desc->appGcfIdsCount;cf++)
			{
				CCFDescriptor * file=(CCFDescriptor *)updatedFiles->get(desc->appGcfIds[cf]);
				if (file)
				{
					refreshApp(apps[ind]);
					break ;
				}
			}
		}
		free (apps);
	}

	delete updatedFiles;
	// update files ?
/*	tasksDlg->SendMessage( AM_DATANOTIFY, 0, 0 );
	if (filesDlg) filesDlg->SendMessage( AM_DATANOTIFY, 0, 0 );*/
}

void CTaskManager::initTasksDisplay(CListCtrl * _control,CDialog * _dialog)
{
	tasksCtrl=_control;
	tasksDlg=_dialog;
	char * names[3]={"File","Task","Completion"};
	int widths[3]={300,300,100};
	buildListColumns(tasksCtrl,names,widths,3);
	tasksCtrl->SetExtendedStyle(tasksCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CTaskManager::initFilesDisplay(CListCtrl * _control,CDialog * _dialog)
{
	filesCtrl=_control;
	filesDlg=_dialog;
 
}


void CTaskManager::initAppsDisplay(CListCtrl * _control,CDialog * _dialog)
{
	appsCtrl=_control;
	appsDlg=_dialog;
 
}


void CTaskManager::refreshFilesDisplay(DWORD id)
{
	if (!filesDlg || !filesCtrl) return;
	
	int fileIndex=-1;
	for (int ind=0;ind<filesCtrl->GetItemCount();ind++)
	{	
		DWORD ctrlId=filesCtrl->GetItemData(ind);
		if (id==ctrlId)
		{
			fileIndex=ind;
			break;
		}
	}

	if (fileIndex!=-1)
	{
		CCFDescriptor * descriptor=(CCFDescriptor *)getCFManager()->files->get(id);
		
		if (descriptor)
		{
			char * notext="";
			char * values[NB_COLS];
			values[0]=descriptor->fileName;
			char size[50];renderSize(descriptor->realSize,size,descriptor->isMoreThan4GB);
			values[1]=size;
			char version[50];sprintf(version,"%d",descriptor->fileVersion);
			values[2]=version;
			
			char completion[50];
			if (descriptor->runningTask)
				sprintf(completion,"%d.00%%",descriptor->runningTask->taskProgress);
			else
			{
				sprintf(completion,"%3.2f%%",descriptor->completion);
				if (descriptor->fileVersion<descriptor->cdrVersion) strcpy(completion,"outdated");
			}
			values[3]=completion;
			char cdrVersion[50];sprintf(cdrVersion,"%d",descriptor->cdrVersion);
		//	values[4]=cdrVersion;
			
			values[4]=descriptor->information;
			values[5]=(descriptor->runningTask ? descriptor->runningTask->taskName : notext);

			updateListValue(filesCtrl, fileIndex, values,NB_COLS);
		}
	}
}

void CTaskManager::awakeTasks()
{
	ConfigManager conf;
 
	if ((nbRunningTasks<nbTotalTasks))// && (nbRunningTasks<conf.getMaxTasks()))
	{
		DWORD * keys;
		int nbTasks;
TLock();
		nbTasks=tasks->size();
		keys=(DWORD*)malloc(4*nbTasks);
		nbTasks=tasks->getDWordKeys(keys);
	 /*
		for (int ind=0;ind<nbTasks ;ind++)
		{
			CTask * task=(CTask *)keys[ind];
			if (task->waiting && task->pleaseStop )
			{
				tasks->remove(keys[ind]);
				nbTotalTasks--;
				delete task;
			}
		}
		nbTasks=tasks->getDWordKeys(keys,nbTasks);
	 */
		for (int  ind=0;ind<nbTasks;ind++)
		{
		 
			CTask * task=(CTask *)keys[ind];
			if (task->waiting && (nbRunningTasks<conf.getMaxTasks() || task->pleaseStop))
			{
				 
				task->waiting=0;
				task->Start();
				nbRunningTasks++;				 
			}
		}
		free(keys);
TUnlock();
	}
}

void CTaskManager::refreshApp(DWORD appId)
{
	if (!appsDlg || !appsCtrl) return;
	
	int appIndex=-1;
	for (int ind=0;ind<appsCtrl->GetItemCount();ind++)
	{	
		DWORD ctrlId=appsCtrl->GetItemData(ind);
		if (appId==ctrlId)
		{
			appIndex=ind;
			break;
		}
	}
 	if (appIndex!=-1)
	{
		char tmp[100];
		CAppDescriptor * app=(CAppDescriptor*)getCFManager()->apps->get(appId);
 		if (app)
		{
			*(app->information)=0;
			app->completion=0;
			app->realSize=0;

			for (int ind=0;ind<app->appGcfIdsCount;ind++)
			{
 				CCFDescriptor * file=(CCFDescriptor*)getCFManager()->files->get(app->appGcfIds[ind]);
				if (file)
				{
					if (file->runningTask)
					{
						strcpy(tmp,file->runningTask->taskName);
						char * details=strstr(tmp," : ");
						if (details) *details=0;
						if (!strstr(app->information,tmp))
						{
	 
							if (strlen(app->information))
								strcat(app->information,", ");
							strcat(app->information,tmp);
						}
					}
					if (file->cdrVersion==file->fileVersion) app->completion+=file->completion*(float)file->realSize;
					app->realSize+=file->realSize;
				}
				else
				{
					if (!app->appGcfOptional[ind])
					{
						app->realSize+=app->appGcfSizes[ind];
					}
				}

			}
 
			app->completion/=(double)(signed __int64)app->realSize;

		

			char * notext="";
			char * values[NB_APPS_COLS];
			values[0]=app->appName;
			values[1]=app->editor;
			values[2]=app->type;
			char size[50];renderSize(app->realSize,size,false);
			values[3]=size;
			char completion[50];sprintf(completion,"%3.2f%%",app->completion);
			values[4]=completion;
			
		//	values[4]=cdrVersion;
			
			values[5]=app->information;

			updateListValue(appsCtrl, appIndex, values,NB_APPS_COLS);
 
		}
	}	
}
