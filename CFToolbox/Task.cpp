// Task.cpp: implementation of the CTask class.
//
//////////////////////////////////////////////////////////////////////
#include "debug.h"
#include "stdafx.h"
#include "CFToolbox.h"
#include "Task.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTask::CTask()
{
	cfId=-1;
	taskFile=0;
	taskName=0;
	taskProgress=0;

	taskStatus=TASK_STATUS_STOPABLE;
	waiting=1;
	pleaseStop=0;
	close=0;
}

CTask::~CTask()
{
	if (taskFile) free(taskFile);
	if (taskName) free(taskName);
}

void CTask::setTaskManager(CTaskManager * _taskManager)
{
	taskManager=_taskManager;
}

void CTask::notifyTaskEnd()
{
 
	taskManager->unregisterTask(this);
 
}


void CTask::setTaskStatus(int status)
{
	taskStatus=status;
}

int CTask::getTaskStatus()
{
	return taskStatus;
}

void CTask::AskToStop()
{
 
	pleaseStop=1;
}
