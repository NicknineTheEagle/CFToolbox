// TabApps.cpp : implementation file
//
#include "process.h"
#include "stdafx.h"
#include "CFToolbox.h"
#include "TabApps.h"
#include "launcher.h"
#include "CFManager.h"
#include "AppDescriptor.h"
#include "Tools.h"
#include "ConfigManager.h"
#include "CFDescriptor.h"
#include "AppIcons.h"
#include "Debug.h"
#include "Accounts.h"
#include "ChooseAccount.h"
#include "SteamNetwork.h"
#include "TabPreferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabApps dialog

#define NB_APPS_COLS 6 // also in TaskManager !
CTabApps * theOneAndUniqueTabApps=0;

CTabApps * getTabApps()
{
	return theOneAndUniqueTabApps;
}
CTabApps::CTabApps(CWnd* pParent /*=NULL*/)
: CDialog(CTabApps::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabApps)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabApps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabApps)
	DDX_Control(pDX, IDC_APPS, m_appsCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabApps, CDialog)
//{{AFX_MSG_MAP(CTabApps)
ON_WM_SIZE()
ON_NOTIFY(NM_RCLICK, IDC_APPS, OnContextualMenu)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_APPS, OnColumnclickApps)
ON_NOTIFY(NM_DBLCLK, IDC_APPS, OnDblclkApps)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabApps message handlers
BOOL CTabApps::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
			pMsg->wParam=NULL ;
	} 
	if(pMsg->message==WM_CHAR)
	{
		TCHAR chr=static_cast<TCHAR>(pMsg->wParam);
		
		switch(chr)
		{
		case 0x01:
			{
				// ctrl A
				for (int ind=0;ind<m_appsCtrl.GetItemCount();ind++)
				{
					m_appsCtrl.SetItemState(ind,LVIS_SELECTED,LVIS_SELECTED);
				}
				break;
			}
		case 0x03:
			{
				// ctrl C
				CCFManager * manager=getCFManager();
				
				//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
				POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();
				CString copy="Game/Media/Tool\tDeveloper\tType\tSize\tCompletion\tInformation\r\n";
				while(pos)
				{
					int index = m_appsCtrl.GetNextSelectedItem(pos);
					DWORD cfId=m_appsCtrl.GetItemData(index);
					CAppDescriptor * descriptor = (CAppDescriptor*) manager->apps->get(cfId);
					if (descriptor)
					{
						CString line;
						char size[100];
						renderSize(descriptor->realSize,size,false);
						char completion[100];
						
						sprintf(completion,"%3.2f%%",descriptor->completion);
						
						line.Format("%s\t%s\t%\t%s\t%s\t%s\t%s\r\n",descriptor->appName,descriptor->editor,descriptor->type,size,completion,(descriptor->information ? descriptor->information : ""));
						copy+=line;
					}
				}
				if(OpenClipboard())	
				{
					HGLOBAL clipbuffer;
					char * buffer;
					EmptyClipboard();
					clipbuffer = GlobalAlloc(GMEM_DDESHARE, copy.GetLength()+1);
					buffer = (char*)GlobalLock(clipbuffer);
					strcpy(buffer, LPCSTR(copy));
					GlobalUnlock(clipbuffer);
					SetClipboardData(CF_TEXT,clipbuffer);
					CloseClipboard();
				}
				
				break;
			} 
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void buildListColumns(CListCtrl* list, char ** names,int * width,int nbCols);
int insertListValue(CListCtrl* list, int id, char ** values,int nbCols,int iconId);
int updateListValue(CListCtrl* list, int index, char ** values,int nbCols);

BOOL CTabApps::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	char * names[NB_APPS_COLS]={"Game/Media/Tool","Developer","Type","Size","Completion"/*,"CDR Version" */,"Current operation"};
	int widths[NB_APPS_COLS]={240,130,50,90,70/*,50*/,120 };
	buildListColumns(&m_appsCtrl,names,widths,NB_APPS_COLS);
	m_appsCtrl.SetExtendedStyle(m_appsCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	// TODO: Add extra initialization here
	m_appsCtrl.SetImageList(getAppIcons()->m_iconsList,LVSIL_SMALL);
	refreshContent();

	getTaskManager()->initAppsDisplay(&m_appsCtrl,this);
	theOneAndUniqueTabApps=this;
	properties=0;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CTabApps::refreshContent() 
{
	
	DWORD * ids;
	m_appsCtrl.DeleteAllItems();
	CCFManager * manager=getCFManager();
	if (!manager) return;
	int nb=manager->apps->size();		
	ids=(DWORD*)malloc(4*nb);
	nb=manager->apps->getDWordKeys(ids);		
	char * notext="";
	
	
	for (int ind=0;ind<nb;ind++)
	{
		CAppDescriptor * descriptor=(CAppDescriptor *)manager->apps->get(ids[ind]);
		char * values[NB_APPS_COLS];
		values[0]=descriptor->appName;
		
		values[1]=descriptor->editor;
		
		values[2]=descriptor->type;
		
		char size[50];renderSize(descriptor->realSize,size,false);
		values[3]=size;
		
		char completion[50];sprintf(completion,"%3.2f%%",descriptor->completion);
		
		values[4]=completion;
		
		
		values[5]=descriptor->information;
		//	values[5]=(descriptor->runningTask ? descriptor->runningTask->taskName : notext);
		
		insertListValue(&m_appsCtrl, descriptor->appId, values,NB_APPS_COLS,descriptor->iconId);
	}
	free(ids);
}

void CTabApps::OnContextualMenu(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	
	//	HashMap * filesToDelete=new HashMap(HASHMAP_NODELETE);
    *pResult = 0;
	// TODO: Add your control notification handler code here
	if (!m_appsCtrl.GetSelectedCount()) return;
	
	char defaultAccount[1000];
	*defaultAccount=0;
	CCFManager * manager=getCFManager();	
	CAccounts accs;
	
	// check if running actions
	bool runningActions=false;
	bool hasNcf=false;	
	bool hasGame=false;	
	bool hasMedia=false;	
	bool hasTool=false;	
	POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();
	
	while(pos)
	{
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_appsCtrl.GetNextSelectedItem(pos);
		DWORD cfId=m_appsCtrl.GetItemData(index);
		CAppDescriptor * appDescriptor = (CAppDescriptor*) manager->apps->get(cfId);
		if (!appDescriptor) continue;

		if (!stricmp(appDescriptor->type,"tool")) hasTool=true;
		else if (!stricmp(appDescriptor->type,"movie")) hasMedia=true;
		else hasGame=true;

		for (int f=0;f<appDescriptor->appGcfIdsCount;f++)
		{
			DWORD cfId=appDescriptor->appGcfIds[f];
			
			CCFDescriptor * descriptor=(CCFDescriptor*)manager->files->get(cfId);
			if (descriptor)
			{
				
				if (descriptor->runningTask) runningActions=true;
				if (descriptor->isNcf) hasNcf=true;
			}
		}
	}
	// end if check
	
	ConfigManager conf;
	
	
    CMenu mnuTop;
    mnuTop.LoadMenu(IDR_APPS_MENU);

	getPlugins()->buildMenu(&mnuTop,ID_ROOT_CANCEL,false,false,hasGame,hasMedia,hasTool);
	
	if (accs.getNbAccounts()<2)
	{ 
		mnuTop.EnableMenuItem(ID_ROOT_ACCOUNT,MF_GRAYED );
	}
	
	if (!strlen(conf.getAppCommandLineTemplate()))
		mnuTop.EnableMenuItem(ID_ROOT_RUN,MF_GRAYED );
	
	if (runningActions)
	{
		mnuTop.EnableMenuItem(ID_ROOT_OPEN,MF_GRAYED );
		
		mnuTop.EnableMenuItem(ID_ROOT_ACCOUNT,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_DOWNLOAD,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PATCHS_MAKEARCHIVE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PATCHS_MAKEUPDATE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PATCHS_APPLYUPDATE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_VALIDATE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_CORRECT,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_DELETE,MF_GRAYED );
	}
	else
	{
		mnuTop.EnableMenuItem(ID_ROOT_CANCEL,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PAUSE_,MF_GRAYED );
	}
	if (runningActions || hasNcf)
	{
		mnuTop.EnableMenuItem(ID_ROOT_MINI,MF_GRAYED );
	}	
    CMenu* pPopup = mnuTop.GetSubMenu(0);
    ASSERT_VALID(pPopup);
	CPoint point;
    GetCursorPos(&point);	
    int command=pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN| TPM_RETURNCMD, point.x, point.y, AfxGetMainWnd(), NULL);
	
	if (!command) return;
	
	CPlugin * plugin=getPlugins()->getPlugin(command);
	
	
	CString outPath=conf.getOutputPath();
	/*if (command==ID_ROOT_OPEN) 
	{
	POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();
	if (pos)
	{
				int index = m_appsCtrl.GetNextSelectedItem(pos);
				DWORD appId=m_appsCtrl.GetItemData(index);
				CAppDescriptor * appDesc=(CAppDescriptor *)manager->apps->get(appId);
				if (appDesc)
				{
				/*for (int ind=0;ind<appDesc->appGcfIdsCount;ind++)
				{
				CCFDescriptor * desc=(CCFDescriptor*)manager->files->get(appDesc->appGcfIds[ind]);
				if (!desc || desc->completion<100.0f)
				{
				printDebug(DEBUG_LEVEL_ERROR,appDesc->appName,"Can't launch app. A file is incomplete",appDesc->appGcfNames[ind],0);
				return;
				}
				if (desc->runningTask)
				{
				printDebug(DEBUG_LEVEL_ERROR,appDesc->appName,"Can't launch app. A task is running on file",desc->runningTask->taskName,0);
				return;
				}
				}* /
				launchApp(appId);
				}
				return;
				}
}*/
	
	
/*	switch (command)
{
case (ID_ROOT_DELETE):
{
if (MessageBox("Are you sure you want to delete the selected apps ?","Are you sure ?",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)==IDNO)
return;
break;
}
}*/
/*	switch (command)
{
case (ID_ROOT_PATCHS_MAKEARCHIVE):
{
if (!BrowseForFolder(NULL,"Select the archive creation folder",outPath))
return;
break;
}
case (ID_ROOT_PATCHS_MAKEUPDATE):
{
if (!BrowseForFolder(NULL,"Select the update creation folder",outPath))
return;
break;
}
}
	*/	
	HashMapDword * processedFiles=new HashMapDword(HASHMAP_NODELETE);
	
	
	//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
	pos = m_appsCtrl.GetFirstSelectedItemPosition();
	CString copy="Game/Media/Tool\tEditor\tSize\tCompletion\tInformation\r\n";
	bool runOnlyOnceForMultipleSelection=false;
	while(pos)
	{
		if (runOnlyOnceForMultipleSelection) break;
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_appsCtrl.GetNextSelectedItem(pos);
		DWORD appId=m_appsCtrl.GetItemData(index);
		CAppDescriptor * appDescriptor = (CAppDescriptor*) manager->apps->get(appId);

		if (plugin)
		{
			manager->executePlugin(plugin,appDescriptor);
			runOnlyOnceForMultipleSelection=plugin->runOnlyOnceForMultipleSelection;
			continue;
		}


		if (command==ID_ROOT_RUN || command==ID_ROOT_COPY || command==ID_ROOT_PROPERTIES)
		{// work on app level
			switch (command)
			{
			case (ID_ROOT_RUN):
				{
					if (appDescriptor)
					{
						ConfigManager conf;
						launch(appId,(conf.isWaitForAppCommandLineEnd() ? _P_WAIT : _P_NOWAIT),true);
						//	Sleep(1000);
					}
					break;
				}
			case (ID_ROOT_COPY):
				{
					if (appDescriptor)
					{
						CString line;
						char size[100];
						renderSize(appDescriptor->realSize,size,false);
						char completion[100];
						sprintf(completion,"%3.2f%%",appDescriptor->completion);
						line.Format("%s\t%s\t%\t%s\t%s\t%s\t%s\r\n",appDescriptor->appName,appDescriptor->editor,appDescriptor->type,size,completion,(appDescriptor->information ? appDescriptor->information : ""));
						copy+=line;
					}
					break;
				}
			case (ID_ROOT_PROPERTIES):
				{			
					if (properties)
					{
						properties->DestroyWindow();
						delete properties;
						properties=0;
					}
					
					if (appDescriptor)
					{
						properties=new CAppPropertiesPopup;
						properties->m_id.Format("%d",appDescriptor->appId);
						properties->m_name=appDescriptor->appName;
						properties->m_developper=appDescriptor->editor;
						char size[100];
						renderSize(appDescriptor->realSize,size,false);
						properties->m_size=size;
						
						if (appDescriptor->manual)
							properties->m_manual=appDescriptor->manual;
						else
							properties->m_manual="No manual";			
						
						if (appDescriptor->commonPath && strlen(appDescriptor->commonPath))
							properties->m_common=appDescriptor->commonPath;
						else
							properties->m_common="No common path";
						
						properties->Create(IDD_APP_PROPERTIES);
						CString url;
						SteamNetwork network;
						url.Format(network.getUpdatesHistoryURL(),appDescriptor->appId);
						properties->m_browser.Navigate(url, NULL, NULL, NULL, NULL);
						
						for (int ind=0;ind<appDescriptor->nbProperties;ind++)
						{
							char *p[2];
							p[0]=appDescriptor->propertiesNames[ind];
							p[1]=appDescriptor->propertiesValues[ind];
							insertListValue(&properties->m_properties,ind,p,2,-1);
						}
						char empty[1]="";
						char notDownloaded[15]="Not downloaded";
						char outdated[9]="Outdated";
						char sizeV[50];
						char version[50];
						char completion[50];
						for ( ind=0;ind<appDescriptor->appGcfIdsCount;ind++)
						{
							
							char *p[4];
							p[0]=appDescriptor->appGcfNames[ind];
							CCFDescriptor * part=(CCFDescriptor *)getCFManager()->files->get(appDescriptor->appGcfIds[ind]);
							if (!part)
							{
								p[1]=empty;
								p[2]=empty;
								p[3]=notDownloaded;
								
							}
							else
							{
								renderSize(part->realSize,sizeV,part->isMoreThan4GB);
								p[1]=sizeV;
								if (part->fileVersion==part->cdrVersion)
								{
									sprintf(version,"%d",part->fileVersion);
									p[2]=version;
									sprintf(completion,"%3.2f%%",part->completion);
									p[3]=completion;
								}
								else
								{
									sprintf(version,"%d (last version : %d)",part->fileVersion,part->cdrVersion);
									p[2]=version;
									p[3]=outdated;
								}
							}
							insertListValue(&properties->m_cfs,appDescriptor->appGcfIds[ind],p,4,-1);
						}
					}
					break;
					
				}
			}
		}
		else
		{ // it's not a copy / properties => must work on file level
			for (int f=0;f<appDescriptor->appGcfIdsCount;f++)
			{
				DWORD cfId=appDescriptor->appGcfIds[f];
				/*	if (command==ID_ROOT_DELETE)
				{ // counts what files to delete and in how many apps they are used
				filesToDelete->put(cfId,(void*)((DWORD)filesToDelete->get(cfId)+1));
				
			}*/
				if (!processedFiles->get(cfId))
				{
					CCFDescriptor * descriptor=(CCFDescriptor*)manager->files->get(cfId);
					processedFiles->put(cfId,(void*)1);
					if (descriptor)
					{
						if ( command==ID_ROOT_PAUSE_ ||command==ID_ROOT_CANCEL || !descriptor->runningTask)
						{
							switch (command)
							{
							case (ID_ROOT_PATCHS_MAKEARCHIVE):
								{
									manager->makeArchive(cfId,outPath.GetBuffer(0),conf.isCorrectBeforeArchiveUpdateCreation());
									break;
								}
							case (ID_ROOT_MINI):
								{
									manager->createMiniCF(cfId,outPath.GetBuffer(0) );
									break;
								}
							case (ID_ROOT_PATCHS_MAKEUPDATE):
								{
									CString archive;
									char filters[1000];
									
									sprintf(filters,"Archive for %s (*.archive)|*.archive|All Files (*.*)|*.*||",descriptor->fileName);
									if (BrowsForFile( filters, TRUE,"", archive))
									{
										manager->makeUpdate(cfId,archive.GetBuffer(0),outPath.GetBuffer(0),conf.isCorrectBeforeArchiveUpdateCreation());
									}
									break;
								}
							case (ID_ROOT_PATCHS_APPLYUPDATE):
								{
									CString update;
									char filters[1000];
									//	CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
									sprintf(filters,"Update for %s (*.update)|*.update.gcf|All Files (*.*)|*.*||",descriptor->fileName);
									if (BrowsForFile( filters, TRUE,"", update))
									{
										manager->apply_Update(cfId,update.GetBuffer(0) );
										
									}
									break;
								}
							case (ID_ROOT_VALIDATE):
								{
									manager->validate(cfId);
									break;
								}
							case (ID_ROOT_CORRECT):
								{
									manager->correct(cfId);
									break;
								}
							case (ID_ROOT_PAUSE_):
							case (ID_ROOT_CANCEL):
								{
									manager->cancelTask(cfId);
									break;
								}
							case (ID_ROOT_ACCOUNT):
								{
									
									if (accs.getNbAccounts()>0)
									{
										char * accountId=accs.getAccountIdForFileId(cfId);
										
										if (strlen(defaultAccount))
										{
											accountId=defaultAccount;
										}
										else
										{
											if (accs.getNbAccounts()==1)
											{
												accountId=accs.getAccountIdAt(0);
											}
											else
											{
												CChooseAccount choose;
												choose.m_file.Format("%s / %s",appDescriptor->appName, descriptor->fileName);
												choose.m_selectedAccount=_T(accountId);									
												int ok=choose.DoModal();
												if (ok==IDOK)
												{
													if (choose.m_remember)
													{
														strcpy(defaultAccount,choose.m_account.GetBuffer(0));
													}
													accountId=accs.getAccount(choose.m_account.GetBuffer(0))->node->getName();
												}
											}	
										}
										
										if (accountId)
										{
											accs.setAccountIdForFileId(cfId,accountId);
										}
									}		
									break;
								}
							case (ID_ROOT_DOWNLOAD):
								{
									CAccounts accs;
									if (accs.getNbAccounts()>0)
									{
										char * accountId=accs.getAccountIdForFileId(cfId);
										if (!accountId)
										{
											if (strlen(defaultAccount))
											{
												accountId=defaultAccount;
											}
											else
											{
												if (accs.getNbAccounts()==1)
												{
													accountId=accs.getAccountIdAt(0);
												}
												else
												{
													CChooseAccount choose;
													choose.m_file.Format("%s / %s",appDescriptor->appName, descriptor->fileName);
													
													int ok=choose.DoModal();
													if (ok==IDOK)
													{
														if (choose.m_remember)
														{
															strcpy(defaultAccount,choose.m_account.GetBuffer(0));
														}
														accountId=accs.getAccount(choose.m_account.GetBuffer(0))->node->getName();
													}
												}
											}
											if (accountId)
											{
												accs.setAccountIdForFileId(cfId,accountId);
											}
										}
										if (accountId)
										{
											CAccount * acc=accs.getAccount(accountId);
											if (acc)
											{
												char ticket[1000];
												int ticketLen=acc->getContentServerAuthTickets(cfId,ticket);
												/*if (!ticketLen)
												{
													if (acc->authenticate())
													{
														acc->commit();
														ticketLen=acc->getTicket(ticket);
													}
												}*/
												if (ticketLen)
													manager->download(cfId, ticket, ticketLen,conf.isCorrectBeforeArchiveUpdateCreation());
											}
										}
									}
									else 
									{
										MessageBox("No account configured","CF Toolbox",MB_ICONERROR|MB_OK);
									}
									break;
								}
						}
						
					}
					else
					{
						printDebug(DEBUG_LEVEL_ERROR,"CF manager","Can't perform operation, as a task is currently running",descriptor->fileName,0);
					}
					
					if (command!=ID_ROOT_PAUSE && command!=ID_ROOT_CANCEL) SleepEx(10,FALSE);
				}
				}
			}
		}
		}
		if (command==ID_ROOT_COPY)
		{
			if(OpenClipboard())	
			{
				HGLOBAL clipbuffer;
				char * buffer;
				EmptyClipboard();
				clipbuffer = GlobalAlloc(GMEM_DDESHARE, copy.GetLength()+1);
				buffer = (char*)GlobalLock(clipbuffer);
				strcpy(buffer, LPCSTR(copy));
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_TEXT,clipbuffer);
				CloseClipboard();
			}
		}
		
		/*	if (filesToDelete->size())
		{
		// some files to be deleted, delete the ones which are not used by more apps that what asked
		
		  // count files usage
		  HashMap * toDeleteFilesUsage=new HashMap(HASHMAP_NODELETE);
		  int nbApps=manager->apps->size();
		  DWORD *appIds=(DWORD*)malloc(4*nbApps);
		  nbApps=manager->apps->getDWordKeys(appIds);
		  for (int ind=0;ind<nbApps;ind++)
		  {
		  CAppDescriptor * appDescriptor=(CAppDescriptor*)manager->apps->get(appIds[ind]);
		  for (int ind2=0;ind2<appDescriptor->appGcfIdsCount;ind2++)
		  {
		  toDeleteFilesUsage->put(appDescriptor->appGcfIds[ind2],(void*)((DWORD)toDeleteFilesUsage->get(appDescriptor->appGcfIds[ind2])+1));
		  }
		  }
		  free(appIds);
		  
			int nbFiles=filesToDelete->size();
			DWORD *fileIds=(DWORD*)malloc(4*nbFiles);
			nbFiles=filesToDelete->getDWordKeys(fileIds);
			for (ind=0;ind<nbFiles;ind++)
			{	
			DWORD nbToDelete=(DWORD)filesToDelete->get(fileIds[ind]);
			DWORD nbUsed=(DWORD)toDeleteFilesUsage->get(fileIds[ind]);
			
			  if (nbToDelete>=nbUsed)
			  {
			  //				 getTabFiles()->m_filesCtrl.DeleteItem(index);
			  manager->deleteFile(fileIds[ind]);
			  }
			  
				}
				free(fileIds);
				delete toDeleteFilesUsage;
				manager->refreshApps();
				}
				
				  
					
					  delete filesToDelete;
		delete processedFiles;		*/
}



int CALLBACK appsCompareFunc(LPARAM id1, LPARAM id2 , 
							 LPARAM lParamSort)
{  
	int iSubItem = (int)lParamSort;
	
	
	int a=0;
	
	CAppDescriptor * descriptor1 = (CAppDescriptor*) getCFManager()->apps->get(id1);
	CAppDescriptor * descriptor2 = (CAppDescriptor*) getCFManager()->apps->get(id2);
	
	switch (iSubItem)
	{
	case 0:{
		a=stricmp(descriptor1->appName,descriptor2->appName);
		break;
		   }
	case 1:{
		if(
			(! descriptor1->editor || strlen(descriptor1->editor)==0)&&
			(! descriptor2->editor || strlen(descriptor2->editor)==0)
			) 
			a=0;
		else
			if(! descriptor1->editor || strlen(descriptor1->editor)==0) 
				a=1;
			else
				if(! descriptor2->editor || strlen(descriptor2->editor)==0) a=-1;
				else
					a=stricmp(descriptor1->editor,descriptor2->editor);
				break;
		   }
	case 2:{
		a=stricmp(descriptor1->type,descriptor2->type);
		break;
		   }
	case 3:{
		if (descriptor1->realSize==descriptor2->realSize) 
			a=0;
		else
			a=(descriptor1->realSize<descriptor2->realSize ? -1 : 1);
		break;
		   }
	case 4:{
		if (descriptor1->completion==descriptor2->completion) 
			a=0;
		else
			a=(descriptor1->completion<descriptor2->completion ? -1 : 1);
		break;
		   }
		   /*	case 4:{
		   a=(descriptor1->cdrVersion<descriptor2->cdrVersion? -1 : 1);
		   break;
		   }
		*/	
	case 5:{
		if(
			(! descriptor1->information || strlen(descriptor1->information)==0)&&
			(! descriptor2->information || strlen(descriptor2->information)==0)
			) 
			a=0;
		else
			if(! descriptor1->information || strlen(descriptor1->information)==0) 
				a=1;
			else
				if(! descriptor2->information || strlen(descriptor2->information)==0) a=-1;
				else
					a=stricmp(descriptor1->information,descriptor2->information);
				break;
		   }
		
	}
	if (iSubItem && !a) a=stricmp(descriptor1->appName,descriptor2->appName);
	return  a;
}

void CTabApps::OnColumnclickApps(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int nCol = pNMListView->iSubItem;
	
	
    m_appsCtrl.SortItems(appsCompareFunc, nCol);
	
	POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();
	if (pos)
	{
		int index = m_appsCtrl.GetNextSelectedItem(pos);
		m_appsCtrl.EnsureVisible(index,FALSE);
	}
	*pResult = 0;
}

void CTabApps::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	//if (IsWindow(wndTop))
#ifndef	_DEBUG
	m_appsCtrl.SetWindowPos( &wndTop, 0, 0, cx,  cy, 0 );	
#endif
}





void CTabApps::OnDblclkApps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_appsCtrl.GetSelectedCount()) return;
	
	CCFManager * manager=getCFManager();
	ConfigManager conf;
	

	DWORD id=getTabPreferences()->m_appsDoubleClick.GetItemData(getTabPreferences()->m_appsDoubleClick.GetCurSel());
	CPlugin * plugin=getPlugins()->getPlugin(id);
	
	POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();
	if (pos)
	{
		int index = m_appsCtrl.GetNextSelectedItem(pos);
		DWORD appId=m_appsCtrl.GetItemData(index);
		CAppDescriptor * appDesc=(CAppDescriptor *)manager->apps->get(appId);
		if (appDesc)
		{
		/*					for (int ind=0;ind<appDesc->appGcfIdsCount;ind++)
		{
		CCFDescriptor * desc=(CCFDescriptor*)manager->files->get(appDesc->appGcfIds[ind]);
		if (!desc || desc->completion<100.0f)
		{
		printDebug(DEBUG_LEVEL_ERROR,appDesc->appName,"Can't launch app. A file is incomplete",appDesc->appGcfNames[ind],0);
		return;
		}
		if (desc->runningTask)
		{
		printDebug(DEBUG_LEVEL_ERROR,appDesc->appName,"Can't launch app. A task is running on file",desc->runningTask->taskName,0);
		return;
		}
		}*/
		if (!plugin)
		{
			launch(appId,(conf.isWaitForAppCommandLineEnd() ? _P_WAIT : _P_NOWAIT),true);
		}
		else
		{
			getCFManager()->executePlugin(plugin,appDesc);
		}
			/*
			CPlugin * plugin=getPlugins()->getPlugin(0 );
			manager->executePlugin(plugin,appDesc);*/

		}
		return;
	}
	
    *pResult = 0;
}
