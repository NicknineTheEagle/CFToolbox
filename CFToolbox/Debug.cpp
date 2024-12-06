#include <stdio.h>
#include <stdlib.h>
#include "Debug.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <windows.h>

 

 
#ifdef GLOBAL_DEBUG_LEVEL
// THIS VERSION IS ONLY FOR CFToolbox : global log level
int globalLogLevel=DEBUG_LEVEL_INFO;
void setGlobalLogLevel(int l)
{
	globalLogLevel=l;
}
#endif
FILE * logFile=0;

#ifdef _DEBUG_AFX_MODE_
CEdit *	logControl=0;
#else
int logStdOut=0;
#endif

/*int __lock__=0;

void CCLock()
{
	while (1)
	{
		if (!__lock__)
		{
			__lock__++;
			return;
		}

		Sleep(100);
	}
}

void CCUnlock()
{
	__lock__--;
}
*/
CRITICAL_SECTION debugCs; 
void CCLock()
{

	/* Enter the critical section -- other threads are locked out */
	EnterCriticalSection(&debugCs);
}
 
void CCUnlock()
{
	/* Leave the critical section -- other threads can now EnterCriticalSection() */
	LeaveCriticalSection(&debugCs);
}

char * severity[5]={"DEBUG","INFO","ERROR","*TODO*","ALL"};

char * getDateTime(char * buff)
{
	   _strdate( buff );
	   strcat(buff," ");
	   _strtime( buff +strlen(buff));
	   return buff;
}

char * getSeverity(int level)
{
	if (level==DEBUG_LEVEL_DEBUG) return severity[0];
	if (level==DEBUG_LEVEL_INFO) return severity[1];
	if (level==DEBUG_LEVEL_ERROR) return severity[2];
	if (level==DEBUG_LEVEL_TODO) return severity[3];
	return severity[4];
}

void stdoutHexaPrintf(unsigned char b)
{
	char * h="0123456789ABCDEF";
	printf("%c%c",*(h+(b/16)),*(h+(b%16)));
}

void stdoutHexaPrintf(char * b,int l,int full)
{
	for (int ind=0;ind<l;ind++)
	{
		if (full) if ((ind%16)==0) printf("%4x  ",ind);
		
		stdoutHexaPrintf((unsigned char)*(b+ind));
		if (full) if ((ind%16)==7) printf(" ");
		if ((ind%16)==15) 
		{
			printf("     ");
			for (int a=ind-15;a<=ind;a++) printf("%c",(b[a]<32 ? '.' : (b[a]>127 ? '.' : b[a])));
			printf("\n"); 
		}
		else printf(" ");
	}
	if (l%16)
	{
		int a;
		for (a=0;a<(16-(l%16))*3+((l%16)<8 ? 1 : 0);a++)
			printf(" ");
		printf("    ");
		for (a=(l/16)*16;a<l;a++) printf("%c",(b[a]<32 ? '.' : (b[a]>127 ? '.' : b[a])));
		
	}
	printf("\n");
}

void fhexaPrintf(FILE * file, unsigned char b)
{
	char * h="0123456789ABCDEF";
	fprintf(file,"%c%c",*(h+(b/16)),*(h+(b%16)));
}

void fhexaPrintf(FILE * file, char * b,int l,int full)
{
	for (int ind=0;ind<l;ind++)
	{
		if (full) if ((ind%16)==0) fprintf(file,"%4x  ",ind);
		
		fhexaPrintf(file,(unsigned char)*(b+ind));
		if (full) if ((ind%16)==7) fprintf(file," ");
		if ((ind%16)==15) 
		{
			fprintf(file,"     ");
			for (int a=ind-15;a<=ind;a++) fprintf(file,"%c",(b[a]<32 ? '.' : (b[a]>127 ? '.' : b[a])));
			fprintf(file,"\r\n"); 
		}else fprintf(file," ");
	}
	if (l%16)
	{
		int a;
		for (a=0;a<(16-(l%16))*3+((l%16)<8 ? 1 : 0);a++)
			fprintf(file," ");
		fprintf(file,"    ");
		for (a=(l/16)*16;a<l;a++) fprintf(file,"%c",(b[a]<32 ? '.' : (b[a]>127 ? '.' : b[a])));
		
	}
	fprintf(file,"\r\n");
}

#ifdef _DEBUG_AFX_MODE_
void shexaPrintf(CString * log, unsigned char b)
{
	char * h="0123456789ABCDEF";
	char tmp[3];
	sprintf(tmp,"%c%c",*(h+(b/16)),*(h+(b%16)));
	(*log)=(*log)+tmp;
}

void shexaPrintf(CString * log, char * b,int l,int full)
{
	char tmp[256];
	for (int ind=0;ind<l;ind++)
	{
		if (full) 
			if ((ind%16)==0){ sprintf(tmp,"%4x  ",ind);
			(*log)=(*log)+tmp;
			}
			
			shexaPrintf(log,(unsigned char)*(b+ind));
			if (full) if ((ind%16)==7)(*log)=(*log)+" ";
			if ((ind%16)==15) 
			{
				(*log)=(*log)+"     ";
				for (int a=ind-15;a<=ind;a++) 
				{sprintf(tmp,"%c",(b[a]<32 ? '.' : (b[a]>127 ? '.' : b[a])));
				(*log)=(*log)+tmp;}
				(*log)=(*log)+"\r\n"; 
			}else (*log)=(*log)+" ";
	}
	if (l%16)
	{
		int a;
		for (a=0;a<(16-(l%16))*3+((l%16)<8 ? 1 : 0);a++)
			(*log)=(*log)+" ";
		(*log)=(*log)+"    ";
		for (a=(l/16)*16;a<l;a++){ sprintf(tmp,"%c",(b[a]<32 ? '.' : (b[a]>127 ? '.' : b[a])));
		(*log)=(*log)+tmp;}
	}
	(*log)=(*log)+"\r\n";
}

int initDebug(char * newLogFile, CEdit * control )
{
	/* Initialize the critical section -- This must be done before locking */
	InitializeCriticalSection(&debugCs);

	_tzset(); 
 
	logControl=control;
	if (newLogFile)
	{
		logFile=fopen(newLogFile,"aw");
		return !logFile;
	}
	logFile=0;
	
	return 0;
}
#else
int initDebug(char * newLogFile,int logStdout)
{
	/* Initialize the critical section -- This must be done before locking */
	InitializeCriticalSection(&debugCs);
	
	_tzset();
	logStdOut=logStdout;
	if (newLogFile)
	{
		logFile=fopen(newLogFile,"aw");
		return !logFile;
	}
	logFile=0;
	
	return 0;
}
#endif

void closeDebug()
{
	logControl=0;
	if (logFile) fclose(logFile);
	logFile=0;
	/* Release system object when all finished -- usually at the end of the cleanup code */
	DeleteCriticalSection(&debugCs);
}

void closeDebugDialog()
{
#ifdef _DEBUG_AFX_MODE_
	logControl=0;
	/* Release system object when all finished -- usually at the end of the cleanup code */
	DeleteCriticalSection(&debugCs);
#endif

}

#ifdef _DEBUG_AFX_MODE_
void addMfcLogs(CString logStr)
{
	unsigned int len =logControl->GetWindowTextLength();
	
	if (len+logStr.GetLength()>logControl->GetLimitText())
	{
 		logControl->SetSel(0,len+logStr.GetLength()-logControl->GetLimitText(),TRUE);
		CString clear="";
		logControl->ReplaceSel(clear);
	}

	logControl->SetSel(len,len,FALSE);		 
	logControl->ReplaceSel(logStr);
	logControl->LineScroll(logControl->GetLineCount());
}
#endif

void printDebug(int level, char * who, char * what, int desiredLevel)
{
#ifdef GLOBAL_DEBUG_LEVEL
	if (globalLogLevel> level) return;
#else
	if (desiredLevel> level) return;
#endif

	CCLock();
#ifdef _DEBUG_AFX_MODE_
	if (logControl)
	{
		CString logStr;
		if (who) logStr.Format("%s - %s : %s\r\n",getSeverity(level),who,what);
		else logStr.Format("%s - %s\r\n",getSeverity(level),what);

		addMfcLogs(logStr);
	}
#else
	if (logStdOut)
	{
		printf("%s - %s : %s\r\n",getSeverity(level),who,what);
	}
	
#endif
	if (logFile)
	{
		char dt[50];
		getDateTime(dt);
		
		fprintf(logFile,"%s - %s - %s : %s\r\n",dt,getSeverity(level),who,what);
		fflush(logFile);
	}
	CCUnlock();
}

void printDebug(int level, char * who, char * what,ULONGLONG value, int desiredLevel)
{
#ifdef GLOBAL_DEBUG_LEVEL
	if (globalLogLevel> level) return;
#else
	if (desiredLevel> level) return;
#endif
	
#ifdef _DEBUG_AFX_MODE_
	if (logControl)
	{
 		CString logStr;
		if (who) logStr.Format("%s - %s : %s : %u\r\n",getSeverity(level),who,what,value);
		else logStr.Format("%s - %s : %u\r\n",getSeverity(level),what,value);

		addMfcLogs(logStr);
	}
#else
	if (logStdOut)
	{
		printf("%s - %s : %s : %u\r\n",getSeverity(level),who,what,value);
	}
	
#endif
	if (logFile)
	{
		char dt[50];
		getDateTime(dt);
		
		fprintf(logFile,"%s - %s - %s : %s : %u\r\n",dt,getSeverity(level),who,what,value);
		fflush(logFile);
	}
}

void printDebug(int level, char * who, char * what,char *str, int desiredLevel)
{
#ifdef GLOBAL_DEBUG_LEVEL
	if (globalLogLevel> level) return;
#else
	if (desiredLevel> level) return;
#endif
	CCLock();
#ifdef _DEBUG_AFX_MODE_
	if (logControl)
	{
		CString logStr;
		if (who) logStr.Format("%s - %s : %s : %s\r\n",getSeverity(level),who,what,str);
		else logStr.Format("%s - %s : %s\r\n",getSeverity(level),what,str);

		addMfcLogs(logStr);

	}
#else
	if (logStdOut)
	{
		printf("%s - %s : %s : %s\r\n",getSeverity(level),who,what,str);
	}
	
#endif
	if (logFile)
	{
		char dt[50];
		getDateTime(dt);
		
		fprintf(logFile,"%s - %s - %s : %s : %s\r\n",dt,getSeverity(level),who,what,str);
		fflush(logFile);
	}
	CCUnlock();
}

void printDebug(int level, char * who, char * what, char * data, int len, int desiredLevel)
{
#ifdef GLOBAL_DEBUG_LEVEL
	if (globalLogLevel> level) return;
#else
	if (desiredLevel> level) return;
#endif
	CCLock();
#ifdef _DEBUG_AFX_MODE_
	if (logControl)
	{
		//logDialog->UpdateData(TRUE);
		CString logStr;
		if (who) logStr.Format("%s - %s : %s\r\n",getSeverity(level),who,what);
		else logStr.Format("%s - %s\r\n",getSeverity(level),what);
		shexaPrintf(&logStr,data,len,1);
		addMfcLogs(logStr);
	}
#else
	if (logStdOut)
	{
		printf("%s - %s : %s\r\n" ,getSeverity(level),who,what);
		stdoutHexaPrintf(data,len,1);
	}
	
#endif
	if (logFile)
	{
		char dt[50];
		getDateTime(dt);
		fprintf(logFile,"%s - %s - %s : %s\r\n",dt,getSeverity(level),who,what);
		fhexaPrintf(logFile,data,len,1);
		fflush(logFile);
	}	
	CCUnlock();
}
