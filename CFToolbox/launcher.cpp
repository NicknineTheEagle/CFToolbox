#include "config.h"
#include "process.h"
#include "debug.h"
#include "launcher.h"
#include "ConfigManager.h"
#include "CFManager.h"
#include "AppDescriptor.h"
#include "CfDescriptor.h"
#include "Thread.h"
#include "QuestionPopup.h"

/*
int __Llock__=0;

void LLock()
{
	while (1)
	{
		if (!__Llock__)
		{
			__Llock__++;
			return;
		}

		Sleep(100);
	}
}

void LUnlock()
{
	__Llock__--;
}
*/

CRITICAL_SECTION lCs; 
void LLock()
{

	/* Enter the critical section -- other threads are locked out */
	EnterCriticalSection(&lCs);
}
 
void LUnlock()
{
	/* Leave the critical section -- other threads can now EnterCriticalSection() */
	LeaveCriticalSection(&lCs);
}
int cutCommandLine(char * commandLine, char ** params)
{
	int nb=0;
	char * ptr=commandLine;

	while (*ptr)
	{
		while (*ptr==' ' || *ptr=='\t') ptr++;
		if (!*ptr) return nb;

		if (*ptr=='"')
		{
			if (nb) params[nb++]=ptr;
			ptr++;
			if (!nb) params[nb++]=ptr;

			while (*ptr!='"' && *ptr) ptr++;
			
			if (!*ptr) return nb;
			if (nb==1) *ptr=0;
			ptr++;
			if (!*ptr) return nb;
			*ptr=0;
			ptr++;
	
		}
		else
		{
			params[nb++]=ptr;
			while (*ptr!=' ' && *ptr!='\t' && *ptr) ptr++;

			if (!*ptr) return nb;

			*ptr=0;
			ptr++;
		}
	}

	return nb;
}


typedef struct CommandLine
{
	char * commandLine;
	int mode;
	CommandLine * next;
} CommandLine;

class CLauncher : public CThread
{
	CommandLine * toExecute;
	CommandLine * lastCommand;

public :
	CLauncher()
	{
		/* Initialize the critical section -- This must be done before locking */
		InitializeCriticalSection(&lCs);	
		toExecute=0;
		lastCommand=0;
	}
	~CLauncher()
	{
			/* Release system object when all finished -- usually at the end of the cleanup code */
		DeleteCriticalSection(&lCs);
	}
	void add(char * command, int mode)
	{

		CommandLine * newOne=(CommandLine*)malloc(sizeof(CommandLine));
		newOne->commandLine=(char*)malloc(strlen(command)+1);
		strcpy(newOne->commandLine,command);
		newOne->mode=mode;
		newOne->next=0;
		LLock();
		if (!lastCommand)
		{
			lastCommand=newOne;
		}
		else
		{
			lastCommand->next=newOne;
			lastCommand=newOne;
		}

		if (!toExecute)
		{
			toExecute=newOne;
		}
	
		LUnlock();
	}

	virtual DWORD Run( LPVOID /* arg */ )
	{
		while (1)
		{
			while (!toExecute) Sleep(100);		
			LLock();
			CommandLine * now=toExecute;
			toExecute=toExecute->next;
			if (!toExecute) lastCommand=0;
			LUnlock();
			execute(now);
			
		}
	}

	void execute(CommandLine * command)
	{
		toExecute=command->next;
		char * params[200];
		printDebug(DEBUG_LEVEL_DEBUG,"Launcher","Executing",command->commandLine,0);
		int nb=cutCommandLine(command->commandLine,params);
		params[nb]=0;	
		if (nb>0)
		{
			printDebug(DEBUG_LEVEL_EVERYTHING,"Launcher","Executing",params[0],0);
			for (int ind=1;ind<nb;ind++)
			{
				printDebug(DEBUG_LEVEL_EVERYTHING,"Launcher","  - param",params[ind],0);
			}
	 
			_spawnvp(command->mode, params[0],  params);
		}

		free(command->commandLine);
		free(command);
	}
};

void applyTemplate(char * line, char * code, char * value)
{
	char temp[1000];
	char * codePtr;
	while (codePtr=strstr(line,code))
	{
		*codePtr=0;
		strcpy(temp,line);
		strcat(temp,value);
		strcat(temp,codePtr+strlen(code));
		strcpy(line,temp);
	}
}
/*
int buildChooseCommandLine(char * commandLine, char * name)
{
	CString path;
	if (strstr(commandLine,"{choose_file}"))
	{

			char filters[1000];
			//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
			sprintf(filters,"File for %s (*.*)|*.*||",name);
			if (BrowsForFile( filters, TRUE,"", path))
			{
				applyTemplate(commandLine,"{choose_file}",path.GetBuffer(0));	
			}
			else return false;
	}
	if (strstr(commandLine,"{choose_folder}"))
	{

			char filters[1000];
			//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
			sprintf(filters,"Select the folder for %s",name);
			if (BrowseForFolder(NULL,filters,path))
			{
				applyTemplate(commandLine,"{choose_folder}",path.GetBuffer(0));	
			}
			else return false;
	}
	return true;
}
*/
int buildChooseOutputFileCommandLine(char * commandLine, char * name)
{
	CString path;

	char * choose=0;
	while (choose=strstr(commandLine,"{choose_outputfile"))
	{
		if (choose==strstr(choose,"{choose_outputfile}"))
		{
			char filters[1000];
			//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
			sprintf(filters,"File for %s (*.*)|*.*||",name);
			if (BrowsForFile( filters, FALSE,"", path))
			{
				applyTemplate(commandLine,"{choose_outputfile}",path.GetBuffer(0));	
			}
			else return false;
		}
		else
		{
			char * prompt=strchr(choose,':');
			if (prompt)
			{
				char * next=strchr(prompt,'}');
				if (next)
				{
					char temp[1000];
					*choose=0;
					prompt++;
					*next=0;
					next++;
					strcpy(temp,commandLine);
		
					char filters[1000];
					//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
					sprintf(filters,"%s (*.*)|*.*||",prompt);
					if (BrowsForFile( filters, FALSE,"", path))
					{
						strcat(temp,path.GetBuffer(0));
					}
					else return false;

					strcat(temp,next);
					strcpy(commandLine,temp);
				}
				else
					break;
			}
			else
				break;
		}
		
	}
	return true;
}

int buildChooseFileCommandLine(char * commandLine, char * name)
{
	CString path;

	char * choose=0;
	while (choose=strstr(commandLine,"{choose_file"))
	{
		if (choose==strstr(choose,"{choose_file}"))
		{
			char filters[1000];
			//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
			sprintf(filters,"File for %s (*.*)|*.*||",name);
			if (BrowsForFile( filters, TRUE,"", path))
			{
				applyTemplate(commandLine,"{choose_file}",path.GetBuffer(0));	
			}
			else return false;
		}
		else
		{
			char * prompt=strchr(choose,':');
			if (prompt)
			{
				char * next=strchr(prompt,'}');
				if (next)
				{
					char temp[1000];
					*choose=0;
					prompt++;
					*next=0;
					next++;
					strcpy(temp,commandLine);
		
					char filters[1000];
					//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
					sprintf(filters,"%s (*.*)|*.*||",prompt);
					if (BrowsForFile( filters, TRUE,"", path))
					{
						strcat(temp,path.GetBuffer(0));
					}
					else return false;

					strcat(temp,next);
					strcpy(commandLine,temp);
				}
				else
					break;
			}
			else
				break;
		}
		
	}
	return true;
}

int buildChooseFolderCommandLine(char * commandLine, char * name)
{
	CString path;

	char * choose=0;
	while (choose=strstr(commandLine,"{choose_folder"))
	{
		if (choose==strstr(choose,"{choose_folder}"))
		{
			char filters[1000];
			//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
			sprintf(filters,"Select the folder for %s",name);
			if (BrowseForFolder(NULL,filters,path))
			{
				applyTemplate(commandLine,"{choose_folder}",path.GetBuffer(0));	
			}
			else return false;
		}
		else
		{
			char * prompt=strchr(choose,':');
			if (prompt)
			{
				char * next=strchr(prompt,'}');
				if (next)
				{
					char temp[1000];
					*choose=0;
					prompt++;
					*next=0;
					next++;
					strcpy(temp,commandLine);
		
					char filters[1000];
					//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
					sprintf(filters,"%s",prompt);
					if (BrowseForFolder(NULL,filters,path))
					{
						strcat(temp,path.GetBuffer(0));	
					}
					else return false;

					strcat(temp,next);
					strcpy(commandLine,temp);
				}
				else
					break;
			}
			else
				break;
		}
	}
	return true;
}

int buildPromptCommandLine(char * commandLine, char * name)
{
	CString path;

	char * choose=0;
	while (choose=strstr(commandLine,"{prompt"))
	{
 		if (choose==strstr(choose,"{prompt}"))
		{
			CQuestionPopup question;
			question.m_question.Format("Enter a value for %s",name);
			question.m_value="";
			int nResponse = question.DoModal();
			if (nResponse == IDOK && question.m_value.GetLength())
			{
				applyTemplate(commandLine,"{prompt}",question.m_value.GetBuffer(0));	
			}
			else return false;
		}
		else
		{
			char * prompt=strchr(choose,':');
			if (prompt)
			{
				char * next=strchr(prompt,'}');
				if (next)
				{
					char temp[1000];
					*choose=0;
					prompt++;
					*next=0;
					next++;
					strcpy(temp,commandLine);
 				
					CQuestionPopup question;
					question.m_question=prompt;
					question.m_value="";
					int nResponse = question.DoModal();
					if (nResponse == IDOK && question.m_value.GetLength())
					{
						strcat(temp,question.m_value.GetBuffer(0));	
					}
					else return false;


					strcat(temp,next);
					strcpy(commandLine,temp);
				}
				else
					break;
			} 
			else
				break;
		}
	}
	return true;
}

int buildChooseCommandLine(char * commandLine, char * name)
{
	if (!buildChooseFileCommandLine(commandLine,name)) return false;
	if (!buildChooseFolderCommandLine(commandLine,name)) return false;
	if (!buildChooseOutputFileCommandLine(commandLine,name)) return false;
	if (!buildPromptCommandLine(commandLine,name)) return false;
	return true;
}

void buildProperties(char * commandLine,CAppDescriptor * desc)
{
	char * property=0;
	while (property=strstr(commandLine,"{property"))
	{
		char * propertyName=strchr(property,':');
		if (propertyName )
		{
			char * next=strchr(propertyName,'}');
			if (next)
			{
				char temp[1000];
				*property=0;
				propertyName++;
				*next=0;
				next++;
				strcpy(temp,commandLine);
				int found=0;
				for (int ind=0;ind<desc->nbProperties && !found;ind++)
				{
					if (!stricmp(propertyName,desc->propertiesNames[ind]))
					{
						found=1;
						strcat(temp,desc->propertiesValues[ind]);
					}
				}
				strcat(temp,next);
				strcpy(commandLine,temp);
			}
			else
				break;
		}
		else
			break;
		
	}
}

bool buildAppCommandLine(char * commandLine, char * commandLineTemplate,DWORD appId,CAppDescriptor * desc)
{
 	ConfigManager conf;
	SteamNetwork network;
	strcpy(commandLine,commandLineTemplate);
//	strlwr(commandLine);
 

	char v[2000];sprintf(v,"%d",appId);
	applyTemplate(commandLine,"{app_id}",v);
	applyTemplate(commandLine,"{steamapps_language}",conf.getLanguage());
	applyTemplate(commandLine,"{configserver_host}",network.getConfigServer());
	sprintf(v,"%d",network.getConfigServerPort());
	applyTemplate(commandLine,"{configserver_port}",v);
	applyTemplate(commandLine,"{cftoolbox_version}",CFTOOLBOX_VERSION);
	applyTemplate(commandLine,"{storage_folder}",conf.getSteamAppsPath());
	applyTemplate(commandLine,"{output_folder}",conf.getOutputPath());
	if (!desc) desc=(CAppDescriptor *)getCFManager()->apps->get(appId);
	if (!desc) return false;

	buildProperties(commandLine,desc);

	applyTemplate(commandLine,"{app_name}",desc->appName);
	sprintf(v,"%u",(int)desc->completion);
	applyTemplate(commandLine,"{completion}",v);
	applyTemplate(commandLine,"{developer}",desc->editor);
	applyTemplate(commandLine,"{manual}",(desc->manual ? desc->manual : ""));
	applyTemplate(commandLine,"{common_folder}",(desc->commonPath ? desc->commonPath: "")); 
	sprintf(v,"%u",desc->appGcfIdsCount);
	applyTemplate(commandLine,"{files_count}",v);
	char temp[1000];
	*v=0;
	for (int ind=0;ind<desc->appGcfIdsCount;ind++)
	{
		sprintf(temp,"%u ",desc->appGcfIds[ind]);
		strcat(v,temp);
	}
	applyTemplate(commandLine,"{files_ids}",v);
 	*v=0;
	for (ind=0;ind<desc->appGcfIdsCount;ind++)
	{
		sprintf(temp,"\"%s\" ",desc->appGcfNames[ind]);
		strcat(v,temp);
	}	
	applyTemplate(commandLine,"{files_names}",v);
	*v=0;
	for (ind=0;ind<desc->appGcfIdsCount;ind++)
	{
		sprintf(temp,"\"%s\\%s\" ",conf.getSteamAppsPath(),desc->appGcfNames[ind]);
		strcat(v,temp);
	}	
	applyTemplate(commandLine,"{files_paths}",v);

	if (!buildChooseCommandLine(commandLine,desc->appName))	return false;

	return true;
}

bool buildFileCommandLine(char * commandLine, char * commandLineTemplate,DWORD fileId,CCFDescriptor * desc)
{
 	ConfigManager conf;
	SteamNetwork network;
	strcpy(commandLine,commandLineTemplate);
//	strlwr(commandLine);

	

	char v[2000];sprintf(v,"%d",fileId);
	applyTemplate(commandLine,"{file_id}",v);
	applyTemplate(commandLine,"{cftoolbox_version}",CFTOOLBOX_VERSION);
	applyTemplate(commandLine,"{steamapps_language}",conf.getLanguage());
	applyTemplate(commandLine,"{configserver_host}",network.getConfigServer());
	sprintf(v,"%d",network.getConfigServerPort());
	applyTemplate(commandLine,"{configserver_port}",v);
	applyTemplate(commandLine,"{storage_folder}",conf.getSteamAppsPath());
	applyTemplate(commandLine,"{output_folder}",conf.getOutputPath());
	if (!desc) desc=(CCFDescriptor *)getCFManager()->files->get(fileId);
	if (!desc) return false;


	applyTemplate(commandLine,"{file_name}",desc->fileName);
	sprintf(v,"%d",(int)desc->completion);
	applyTemplate(commandLine,"{completion}",v);

	sprintf(v,"%u",desc->cdrVersion);
	applyTemplate(commandLine,"{cdr_version}",v);
	applyTemplate(commandLine,"{common_folder}",(desc->commonPath ? desc->commonPath: "")); 
	applyTemplate(commandLine,"{decryption_key}",(desc->decryptionKey ? desc->decryptionKey : ""));
	sprintf(v,"%u",desc->downloadedBlocks);
	applyTemplate(commandLine,"{downloaded_blocks}",v);
	sprintf(v,"%u",desc->realSize);
	applyTemplate(commandLine,"{file_size}",v);
	sprintf(v,"%u",desc->fileVersion);
	applyTemplate(commandLine,"{file_version}",v);
	applyTemplate(commandLine,"{file_type}",(desc->isNcf ? "ncf" : "gcf"));
	sprintf(v,"%u",desc->nbBlocks);
	applyTemplate(commandLine,"{nb_blocks}",v);
	applyTemplate(commandLine,"{file_path}",desc->path);

	if (!buildChooseCommandLine(commandLine,desc->fileName)) return false;

	return true;
}
 

CLauncher * theLauncher=0;

CLauncher * getLauncher()
{
	if (!theLauncher) 
	{
		theLauncher=new CLauncher();
		theLauncher->Start();
	}
	return theLauncher;
}

void launch(DWORD id,int mode, bool isApp)
{
	ConfigManager conf;
	
	char commandLine[500];	
	int builded=false;
	if (isApp)
		builded=buildAppCommandLine(commandLine, conf.getAppCommandLineTemplate(),id);
	else
		builded=buildFileCommandLine(commandLine, conf.getFileCommandLineTemplate(),id);

	if (builded)
	{
		getLauncher()->add(commandLine,mode);
		Sleep(10);
	}
}

void launch(CCFDescriptor * descriptor,int mode)
{
	ConfigManager conf;
	
	char commandLine[500];	
	if (buildFileCommandLine(commandLine, conf.getFileCommandLineTemplate(),descriptor->fileId,descriptor))
	{
		getLauncher()->add(commandLine,mode);
		Sleep(10);
	}

}
void launch(CAppDescriptor * descriptor,int mode)
{
	ConfigManager conf;
	
	char commandLine[500];	
	if (buildAppCommandLine(commandLine, conf.getAppCommandLineTemplate(),descriptor->appId,descriptor))
	{
		getLauncher()->add(commandLine,mode);
		Sleep(10);
	}
}


