#include "executionAnalyser.h"
#include "afx.h"
#include "debug.h"

DWORD start=0;

DWORD startSection=0;
DWORD totalSection=0;

void startExecAnalyse(char * message )
{
	printDebug(DEBUG_LEVEL_TODO,"ExecAnalyse","starting",0);
	totalSection=0;
	start=GetTickCount();
}
void stopExecAnalyse(char * message )
{
	DWORD elapsed=GetTickCount()-start;
	DWORD out=elapsed-totalSection;
	printDebug(DEBUG_LEVEL_TODO,"ExecAnalyse","stopped",0);
	char dbg[1000];
	double sectionPer=100.0*(double)totalSection/(double)elapsed;
	sprintf(dbg,"total : %u ms   %f%% out section (%u ms)   %f%% in section (%u ms)",elapsed,100.0-sectionPer,out,sectionPer,totalSection);
	printDebug(DEBUG_LEVEL_TODO,"ExecAnalyse",dbg,0);
}

void enterSection()
{
	startSection=GetTickCount();
}
void exitSection()
{
	totalSection+=GetTickCount()-startSection;
}