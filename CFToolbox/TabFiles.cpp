// TabFiles.cpp : implementation file
//
#include "Accounts.h"
#include "process.h"
#include "stdafx.h"
#include "CFToolbox.h"
#include "TabFiles.h"
#include "TabApps.h"
#include "TabAdd.h"
#include "CFManager.h"
#include "CFDescriptor.h"
#include "Tools.h"
#include "ConfigManager.h"
#include "Debug.h"
#include "launcher.h"
#include "ChooseAccount.h"
#include "Plugins.h"
#include "TabPreferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NB_COLS 6 // also in TaskManager !
/////////////////////////////////////////////////////////////////////////////
// CTabFiles dialog

CTabFiles * theOneAndUniqueTabFiles=0;

CTabFiles * getTabFiles()
{
	return theOneAndUniqueTabFiles;
}


CTabFiles::CTabFiles(CWnd* pParent /*=NULL*/)
: CDialog(CTabFiles::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabFiles)
	//}}AFX_DATA_INIT
}


void CTabFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabFiles)
	DDX_Control(pDX, IDC_FILES, m_filesCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabFiles, CDialog)
//{{AFX_MSG_MAP(CTabFiles)
ON_NOTIFY(NM_RCLICK, IDC_FILES, OnContextualMenu)
ON_NOTIFY(NM_DBLCLK, IDC_FILES, OnDblclkFiles)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_FILES, OnColumnclickFiles)
ON_WM_SIZE()
ON_WM_TIMER()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabFiles message handlers	
BOOL CTabFiles::PreTranslateMessage(MSG* pMsg) 
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
				for (int ind=0;ind<m_filesCtrl.GetItemCount();ind++)
				{
					m_filesCtrl.SetItemState(ind,LVIS_SELECTED,LVIS_SELECTED);
				}
				break;
			}
		case 0x03:
			{
				// ctrl C
				CCFManager * manager=getCFManager();
				
				//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
				POSITION pos = m_filesCtrl.GetFirstSelectedItemPosition();
				CString copy="File\tSize\tVersion\tCompletion\tInformation\r\n";
				while(pos)
				{
					int index = m_filesCtrl.GetNextSelectedItem(pos);
					DWORD cfId=m_filesCtrl.GetItemData(index);
					CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
					if (descriptor)
					{
						CString line;
						char size[100];
						renderSize(descriptor->realSize,size,descriptor->isMoreThan4GB);
						char completion[100];
						if (descriptor->cdrVersion==descriptor->fileVersion)
							sprintf(completion,"%3.2f%%",descriptor->completion);
						else
							strcpy(completion,"outdated");
						line.Format("%s\t%s\t%d\t%s\t%s\r\n",descriptor->fileName,size,descriptor->fileVersion,completion,(descriptor->information ? descriptor->information : ""));
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

void buildListColumns(CListCtrl* list, char ** names,int * width,int nbCols)
{
	for (int ind=0;ind<nbCols;ind++)
	{
		list->InsertColumn(ind,names[ind],LVCFMT_LEFT,width[ind],ind);
	}
}

int insertListValue(CListCtrl* list, int id, char ** values,int nbCols,int iconId=0)
{
	int index=list->InsertItem(0,values[0],iconId);
	
	for (int ind=0;ind<nbCols;ind++)
	{
		list->SetItemText(index,ind,values[ind]);
	}
	list->SetItemData(index,id);
	
	return index;
}

int updateListValue(CListCtrl* list, int index, char ** values,int nbCols)
{
	//int index=list->InsertItem(0,values[0]);
	for (int ind=0;ind<nbCols;ind++)
	{
		list->SetItemText(index,ind,values[ind]);
	}
	//	list->SetItemData(index,id);
	return index;
}

BOOL CTabFiles::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	char * names[NB_COLS]={"File","Size","Version","Completion"/*,"CDR Version" */,"Information","Current operation"};
	int widths[NB_COLS]={200,90,50,70/*,50*/,140,150};
	buildListColumns(&m_filesCtrl,names,widths,NB_COLS);
	m_filesCtrl.SetExtendedStyle(m_filesCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	
	// TODO: Add extra initialization here
	refreshContent();
	getTaskManager()->initFilesDisplay(&m_filesCtrl,this);
	theOneAndUniqueTabFiles=this;
	properties=0;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTabFiles::refreshContent() 
{
	DWORD * ids;
	m_filesCtrl.DeleteAllItems();
	CCFManager * manager=getCFManager();
	if (!manager) return;
	int nb=manager->files->size();		
	ids=(DWORD*)malloc(4*nb);
	nb=manager->files->getDWordKeys(ids);		
	
	for (int ind=0;ind<nb;ind++)
	{
		CCFDescriptor * descriptor=(CCFDescriptor *)manager->files->get(ids[ind]);
		insertFile(descriptor,true);
	}
	free(ids);
}
void CTabFiles::insertFile(CCFDescriptor * descriptor,boolean noAutoUpdate)
{
	char notext[1];
	*notext=0;
	char * values[NB_COLS];
	values[0]=descriptor->fileName;
	char size[50];
	renderSize(descriptor->realSize,size,descriptor->isMoreThan4GB);
	values[1]=size;
	char version[50];sprintf(version,"%d",descriptor->fileVersion);
	values[2]=version;
	char completion[50];sprintf(completion,"%3.2f%%",descriptor->completion);
	if (descriptor->fileVersion<descriptor->cdrVersion) strcpy(completion,"outdated");
	
	values[3]=completion;
	char cdrVersion[50];sprintf(cdrVersion,"%d",descriptor->cdrVersion);
	//	values[4]=cdrVersion;
	
	values[4]=descriptor->information;
	values[5]=(descriptor->runningTask ? descriptor->runningTask->taskName : notext);
	
	insertListValue(&m_filesCtrl, descriptor->fileId, values,NB_COLS);
	if (!noAutoUpdate) SetTimer(0,100,NULL);
}

void CTabFiles::removeFile(DWORD fileId)
{
	int nb=m_filesCtrl.GetItemCount();
	for (int ind=0;ind<nb;ind++)
	{
		if (m_filesCtrl.GetItemData(ind)==fileId)
		{
			m_filesCtrl.DeleteItem(ind);
			SetTimer(0,100,NULL);
			return;
		}
	}
}

void CTabFiles::OnContextualMenu(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
    *pResult = 0;
	// TODO: Add your control notification handler code here
	if (!m_filesCtrl.GetSelectedCount()) return;
	CAccounts accs;
	
	char defaultAccount[1000];
	*defaultAccount=0;
	
	CCFManager * manager=getCFManager();	
	
	// check if running actions
	bool runningActions=false;
	bool hasNcf=false;
	bool hasGcf=false;
	POSITION pos = m_filesCtrl.GetFirstSelectedItemPosition();
	


	while(pos)
	{
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_filesCtrl.GetNextSelectedItem(pos);
		DWORD cfId=m_filesCtrl.GetItemData(index);
		CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
		if (!descriptor) continue;
		if (descriptor->runningTask) runningActions=true;
		if (descriptor->isNcf) hasNcf=true;
		if (!descriptor->isNcf) hasGcf=true;
	}
	// end if check
	ConfigManager conf;
	
    CMenu mnuTop;
	mnuTop.LoadMenu(IDR_FILES_MENU);

	getPlugins()->buildMenu(&mnuTop,ID_ROOT_CANCEL,hasGcf,hasNcf,false,false,false);

	if (!strlen(conf.getFileCommandLineTemplate()))
		mnuTop.EnableMenuItem(ID_ROOT_RUN,MF_GRAYED );	
	if (accs.getNbAccounts()<2)
	{ 
		mnuTop.EnableMenuItem(ID_ROOT_ACCOUNT,MF_GRAYED );
	}
	
	if (runningActions)
	{
		mnuTop.EnableMenuItem(ID_ROOT_OPEN,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_RUN,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_ACCOUNT,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_DOWNLOAD,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PATCHS_MAKEARCHIVE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PATCHS_MAKEUPDATE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PATCHS_APPLYUPDATE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_VALIDATE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_CORRECT,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_DELETE,MF_GRAYED );
		mnuTop.EnableMenuItem(ID_ROOT_PROPERTIES,MF_GRAYED );
		
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
	
	HashMapDword * toDelete=new HashMapDword(HASHMAP_NODELETE);

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
	
	
	//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
	pos = m_filesCtrl.GetFirstSelectedItemPosition();
	CString copy="File\tSize\tVersion\tCompletion\tInformation\r\n";
	bool runOnlyOnceForMultipleSelection=false;
	while(pos)
	{
		if (runOnlyOnceForMultipleSelection) break;
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_filesCtrl.GetNextSelectedItem(pos);
		DWORD cfId=m_filesCtrl.GetItemData(index);
		CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
		if (!descriptor) continue;
		if (command==ID_ROOT_COPY || command==ID_ROOT_CANCEL || command==ID_ROOT_PAUSE_ || !descriptor->runningTask)
		{
			if (plugin)
			{
				manager->executePlugin(plugin,descriptor);
				runOnlyOnceForMultipleSelection=plugin->runOnlyOnceForMultipleSelection;
				continue;
			}
			
			switch (command)
			{
			case (ID_ROOT_OPEN):
				{
					manager->openFile(cfId);
					break;
				}
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
					sprintf(filters,"Update for %s (*.update.gcf)|*.update.gcf|All Files (*.*)|*.*||",descriptor->fileName);
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
								choose.m_file=descriptor->fileName;
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
									choose.m_file=descriptor->fileName;
									
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
			case (ID_ROOT_PAUSE_):
			case (ID_ROOT_CANCEL):
				{
					manager->cancelTask(cfId);
					break;
				}
			case (ID_ROOT_DELETE):
				{
					toDelete->put(cfId,descriptor->fileName);
					break;
				}
			case (ID_ROOT_RUN):
				{
					if (descriptor)
					{
						ConfigManager conf;
						launch(cfId,(conf.isWaitForFileCommandLineEnd() ? _P_WAIT : _P_NOWAIT));
						//	Sleep(1000);
					}
					break;
				}
			case (ID_ROOT_COPY):
				{
					if (descriptor)
					{
						CString line;
						char size[100];
						renderSize(descriptor->realSize,size,descriptor->isMoreThan4GB);
						char completion[100];
						if (descriptor->cdrVersion==descriptor->fileVersion)
							sprintf(completion,"%3.2f%%",descriptor->completion);
						else
							strcpy(completion,"outdated");
						line.Format("%s\t%s\t%d\t%s\t%s\r\n",descriptor->fileName,size,descriptor->fileVersion,completion,(descriptor->information ? descriptor->information : ""));
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
					
					if (descriptor)
					{
						properties=new CPropertiesPopup;
						
						
						
						properties->m_blocks.Format("%d / %d",descriptor->downloadedBlocks,descriptor->nbBlocks);
						
						properties->m_description=descriptor->description;
						
						if (descriptor->commonPath)
							properties->m_commonPath=descriptor->commonPath;
						else
							properties->m_commonPath="No common path";
						
						properties->m_id.Format("%d",descriptor->fileId);
						
						if (descriptor->decryptionKey)
							properties->m_key=descriptor->decryptionKey;
						else
							properties->m_key="Not available";
						
						properties->m_path=descriptor->path;
						
						char size[100];
						renderSize(descriptor->realSize,size,descriptor->isMoreThan4GB);
						properties->m_size=size;
						
						if (descriptor->fileVersion==descriptor->cdrVersion)
						{
							properties->m_version.Format("%d",descriptor->fileVersion);
							properties->m_completion.Format("%3.2f%%",descriptor->completion);
						}
						else
						{
							properties->m_version.Format("%d (last version : %d)",descriptor->fileVersion,descriptor->cdrVersion);
							properties->m_completion="Outdated";
						}
						
						GCF * gcf=parseGCF(descriptor->path);
						
						if ((int)gcf>1)
						{
							properties->m_type.Format("%s (type:%d version:%d)",(descriptor->isNcf ? "NCF" : "GCF"),gcf->cfType,gcf->cfVersion);
							properties->m_files.Format("%d",gcf->directory->fileCount);
							properties->m_folders.Format("%d",gcf->directory->itemCount-gcf->directory->fileCount);
							
							deleteGCF(gcf);
						}
						else
						{
							properties->m_type.Format("%s",(descriptor->isNcf ? "NCF" : "GCF"));
							properties->m_files="Unknown";
							properties->m_folders="Unknown";
						}
						/*
						CString	m_type;
						CString	m_version;
						CString	m_files;
						CString	m_folders;
						*/
						
						
						properties->Create(IDD_PROPERTIES);
					}
					break;
				}
			}
		}
		else
		{
			printDebug(DEBUG_LEVEL_ERROR,"CF manager","Can't perform operation, as a task is currently running",descriptor->fileName,0);
		}
		if (command!=ID_ROOT_COPY && command !=ID_ROOT_PROPERTIES && command!=ID_ROOT_CANCEL && command!=ID_ROOT_PAUSE_) SleepEx(10,FALSE);
		
	}
	switch (command)
	{
		case (ID_ROOT_DELETE):
		{
			
			if (toDelete->size())
			{
				DWORD * list=(DWORD*)malloc(4*toDelete->size());
				int size=toDelete->getDWordKeys(list);
				CString names="Do you want to delete the following files :";
				for (int ind=0;ind<size;ind++)
				{
					CString f;
					f.Format("\r\n%s",toDelete->get(list[ind]));
					names+=f;
				}

				if (MessageBox(names,"Are you sure ?",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)==IDNO)
				{
					free(list);
					return;
				}
				for (int ind=0;ind<size;ind++)
				{
					if(manager->deleteFile(list[ind]))
						removeFile(list[ind]);
				}
					free(list);
			}
			
			break;
		}
		case (ID_ROOT_COPY):
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
			break;
		}
	}	
	delete(toDelete);
}

void CTabFiles::OnDblclkFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_filesCtrl.GetSelectedCount()) return;
	
	DWORD id=getTabPreferences()->m_filesDoubleClick.GetItemData(getTabPreferences()->m_filesDoubleClick.GetCurSel());
	CPlugin * plugin=getPlugins()->getPlugin(id);
	
	CCFManager * manager=getCFManager();
	
	//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
	POSITION pos = m_filesCtrl.GetFirstSelectedItemPosition();
	
	while(pos)
	{
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_filesCtrl.GetNextSelectedItem(pos);
		DWORD cfId=m_filesCtrl.GetItemData(index);
		CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
		if (!descriptor->runningTask) 
		{
			if (!plugin)
			{
				manager->openFile(cfId);	 
			}
			else
			{
				getCFManager()->executePlugin(plugin,descriptor);
			}
		}
		else
		{
			printDebug(DEBUG_LEVEL_ERROR,"CF manager","Can't execute operationas a task is currently running",descriptor->fileName,0);
		}
		SleepEx(10,FALSE);
	}
	
    *pResult = 0;
}



int CALLBACK filesCompareFunc(LPARAM id1, LPARAM id2 , 
							  LPARAM lParamSort)
{  
	int iSubItem = (int)lParamSort;
	
	
	int a=0;
	
	CCFDescriptor * descriptor1 = (CCFDescriptor*) getCFManager()->files->get(id1);
	CCFDescriptor * descriptor2 = (CCFDescriptor*) getCFManager()->files->get(id2);
	
	switch (iSubItem)
	{
	case 0:{
		a=stricmp(descriptor1->fileName,descriptor2->fileName);
		break;
		   }
	case 1:{
		if (descriptor1->realSize==descriptor2->realSize)
			a=0;
		else
			a=(descriptor1->realSize<descriptor2->realSize ? -1 : 1);
		break;
		   }
	case 2:{
		if (descriptor1->fileVersion==descriptor2->fileVersion)
			a=0;
		else
			a=(descriptor1->fileVersion<descriptor2->fileVersion ? -1 : 1);
		break;
		   }
	case 3:{
		int outdated1=(descriptor1->fileVersion<descriptor1->cdrVersion);
		int outdated2=(descriptor2->fileVersion<descriptor2->cdrVersion);
		if (outdated1) a=-1;
		else
			if (outdated2) a=1;
			else
				if (descriptor1->completion==descriptor2->completion)
					a=0;
				else
					a=(descriptor1->completion<descriptor2->completion? -1 : 1);
				break;
		   }
		   /*	case 4:{
		   a=(descriptor1->cdrVersion<descriptor2->cdrVersion? -1 : 1);
		   break;
		   }
		*/	case 4:{
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
	case 5:{
		
		if(
			(! descriptor1->runningTask || strlen(descriptor1->runningTask->taskName)==0)&&
			(! descriptor2->runningTask || strlen(descriptor2->runningTask->taskName)==0)
			) 
			a=0;
		else
			if(! descriptor1->runningTask || strlen(descriptor1->runningTask->taskName)==0) 
				a=1;
			else
				if(! descriptor2->runningTask || strlen(descriptor2->runningTask->taskName)==0) a=-1;
				else
					a=stricmp(descriptor1->runningTask->taskName,descriptor2->runningTask->taskName);
				break;
		   }
	}
	if (iSubItem && !a) a=stricmp(descriptor1->fileName,descriptor2->fileName);
	return  a;
}

void CTabFiles::OnColumnclickFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int nCol = pNMListView->iSubItem;
	
	
    m_filesCtrl.SortItems(filesCompareFunc, nCol);
	
	POSITION pos = m_filesCtrl.GetFirstSelectedItemPosition();
	if (pos)
	{
		int index = m_filesCtrl.GetNextSelectedItem(pos);
		m_filesCtrl.EnsureVisible(index,FALSE);
	}
	
	*pResult = 0;
}

void CTabFiles::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	//if (IsWindow(wndTop))
#ifndef _DEBUG
	m_filesCtrl.SetWindowPos( &wndTop, 0, 0, cx,  cy, 0 );	
#endif
}

void CTabFiles::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(nIDEvent);
	if (!nIDEvent)
	{ // list was changed
		CTabApps *tabA=getTabApps();
		if (tabA) 
		{
			getCFManager()->refreshApps();
			tabA->refreshContent();
		}
		CTabAdd * tabAdd=getTabAdd();
		if (tabAdd)
		{
			tabAdd->refreshTab();
		}
		
	}
	CDialog::OnTimer(nIDEvent);
}
