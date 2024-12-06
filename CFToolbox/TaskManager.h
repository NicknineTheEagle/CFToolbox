// TaskManager.h: interface for the CTaskManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASKMANAGER_H__43359F71_105F_4A6F_B84B_0AE246BC5095__INCLUDED_)
#define AFX_TASKMANAGER_H__43359F71_105F_4A6F_B84B_0AE246BC5095__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CTaskManager;

#include "Task.h"
#include "HashMap.h"
#include "ConfigManager.h"
#include "SocketTools.h"
#include "tools.h"
#include "debug.h"

CDialog * getMainDialog();

CTaskManager * getTaskManager(int doNotInstanciate=0);

class CTaskManagerUpdater;
class CSteamMonitor;

class CTaskManager  
{
public:
	void refreshApp(DWORD appId);
	void awakeTasks();
	void initTasksDisplay(CListCtrl * tasksCtrl,CDialog * tasksDlg);
	void initFilesDisplay(CListCtrl * filesCtrl,CDialog * filesDlg);
	void initAppsDisplay(CListCtrl * filesCtrl,CDialog * filesDlg);
	
	void refreshDisplay(CTask * task);
	void refreshDisplay( );
	void gc();
	void killAllTasks();
	void _stopAllTasks(int nowait=0);
	int unregisterTask(CTask * task);
	int addAndStartTask(CTask * task);
	CTaskManager();
	virtual ~CTaskManager();
	int nbTotalTasks;
private :
	int nbRunningTasks;
	
	void refreshFilesDisplay(DWORD id);
	CListCtrl * tasksCtrl;
	CDialog * tasksDlg;

	CListCtrl * filesCtrl;
	CDialog * filesDlg;	
	
	CListCtrl * appsCtrl;
	CDialog * appsDlg;	
	
	HashMapDword * tasks;
	HashMapDword * finishedTasks;
	CTaskManagerUpdater * displayUpdater;
	CSteamMonitor * steamMonitor;
};



class CTaskManagerUpdater : public CThread
{
public :
	int stop;
	virtual DWORD Run( LPVOID /* arg */ )
	{ 
		stop=0;
		while (!stop)
		{
			if (getTaskManager(1))
			{
				getTaskManager(1)->refreshDisplay();
				getTaskManager(1)->awakeTasks();
			}
			Sleep(1000);
		}
		return 0;
	}
};
bool isRunningOutOfSpace();
class CSteamMonitor : public CThread
{
public:
	int stop;
	
	virtual DWORD Run( LPVOID /* arg */ )
	{ 
		int count=0;
		stop=0;
		ConfigManager conf;
		while (!stop)
		{
			if (conf.isSharingSteamFolder())
			{
				if (isRunningProcess("steam.exe"))
				{
					printDebug(DEBUG_LEVEL_ERROR,"CF Toolbox","Steam is running and shares the GCF/NCF folder with CF Toolbox. All the current operations will be stopped",0);
					if (getTaskManager(1)) getTaskManager(1)->_stopAllTasks(); // doesn't instanciate !
					MessageBox(NULL,"Steam is running and shares the GCF/NCF folder with CF Toolbox. All the current operations have been stopped.","CF Toolbox",MB_ICONSTOP|MB_OK);
				}
			}

			if (!isRunningOutOfSpace() && count<=0 && _getDiskFreeSpace(conf.getSteamAppsPath())<100)
			{ // !isRunningOutOfSpace() means that another thread noticed the problem
					count=5;
				// less than 100Mb free on disk
					printDebug(DEBUG_LEVEL_ERROR,"CF Toolbox","CF Toolbox is running out of disk space.",0);
					if (getTaskManager(1)) getTaskManager(1)->_stopAllTasks(); // doesn't instanciate !
					MessageBox(NULL,"CF Toolbox is running out of disk space.","CF Toolbox",MB_ICONSTOP|MB_OK);
					getMainDialog()->SendMessage(WM_CLOSE, 0, 0 );
			}
			count--;

			getMainDialog()->SetTimer(0,100,NULL); // refresh accounts list
			
			Sleep(2000);
		}
		return 0;
	}
};


#endif // !defined(AFX_TASKMANAGER_H__43359F71_105F_4A6F_B84B_0AE246BC5095__INCLUDED_)
