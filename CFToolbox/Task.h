// Task.h: interface for the CTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASK_H__F65A23A6_E195_4FAD_AB1D_33FDA4E213E1__INCLUDED_)
#define AFX_TASK_H__F65A23A6_E195_4FAD_AB1D_33FDA4E213E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CTask;

#include "Thread.h"
#include "TaskManager.h"

#define TASK_STATUS_NO_STOP 0
#define TASK_STATUS_STOPABLE 1

class CTask : public CThread  
{
public:
	void AskToStop();
	DWORD cfId;
	int getTaskStatus();
	void setTaskStatus(int status);
	void notifyTaskEnd();
	CTask();
	virtual ~CTask();
	void setTaskManager(CTaskManager * taskManager);

	int pleaseStop;
	DWORD taskProgress;
	char * taskName;
	char * taskFile;
	int waiting;
	int close;
private : 	
	CTaskManager * taskManager;
	int taskStatus;
 



};

#endif // !defined(AFX_TASK_H__F65A23A6_E195_4FAD_AB1D_33FDA4E213E1__INCLUDED_)
