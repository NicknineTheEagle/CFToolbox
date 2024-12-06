// TabAdd.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "TabAdd.h"
#include "Accounts.h"
#include "SteamNetwork.h"
#include "cdr.h"
#include "CFManager.h"
#include "AppDescriptor.h"
#include "AppIcons.h"
#include "cdr.h"
#include "CFDescriptor.h"
#include "AppDescriptor.h"
#include "Launcher.h"
#include "process.h"
#include "propertiesPopup.h"
#include "tabFiles.h"
#include "tabApps.h"
#include "QuestionPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool isAddEnabled()
{
	ConfigManager conf;
	return !conf.isDisableInstall();	
}
void buildListColumns(CListCtrl* list, char ** names,int * width,int nbCols);
int insertListValue(CListCtrl* list, int id, char ** values,int nbCols,int iconId);
int updateListValue(CListCtrl* list, int index, char ** values,int nbCols);
CTabAdd * theOneAndUniqueTabAdd=0;

CTabAdd * getTabAdd()
{
	return theOneAndUniqueTabAdd;
}
/////////////////////////////////////////////////////////////////////////////
// CTabAdd dialog


CTabAdd::CTabAdd(CWnd* pParent /*=NULL*/)
: CDialog(CTabAdd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabAdd)
	m_account = _T("");
	//}}AFX_DATA_INIT
}


void CTabAdd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabAdd)
	DDX_Control(pDX, IDC_GMTFRM, m_appsFrame);
	DDX_Control(pDX, IDC_FILESFRM, m_filesFrame);
	DDX_Control(pDX, IDC_ACCLBL, m_accLabel);
	DDX_Control(pDX, IDC_ACCOUNT_LIST, m_accountsCtrl);
	DDX_Control(pDX, IDC_FILES, m_filesCtrl);
	DDX_Control(pDX, IDC_APPS, m_appsCtrl);
	DDX_Control(pDX, IDC_DOWNLOAD, m_downloadCtrl);
	DDX_CBString(pDX, IDC_ACCOUNT_LIST, m_account);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabAdd, CDialog)
//{{AFX_MSG_MAP(CTabAdd)
ON_BN_CLICKED(IDC_DOWNLOAD, OnDownload)
ON_WM_SHOWWINDOW()
ON_NOTIFY(LVN_COLUMNCLICK, IDC_APPS, OnColumnclickApps)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_APPS, OnItemchangedApps)
ON_WM_TIMER()
	ON_NOTIFY(NM_RCLICK, IDC_APPS, OnAppContextualMenu)
	ON_NOTIFY(NM_RCLICK, IDC_FILES, OnFileContextualMenu)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabAdd message handlers
BOOL CTabAdd::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
			pMsg->wParam=NULL ;
	} 
	
	return CDialog::PreTranslateMessage(pMsg);
}
void CTabAdd::OnDownload() 
{
if (isAddEnabled())
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	
	POSITION pos = m_filesCtrl.GetFirstSelectedItemPosition();
	
	if (!pos) return ;
	
	CCFManager * cfManager=getCFManager();
	
	CAccounts accs;
	if (accs.getNbAccounts()>0)
	{
		
		CAccount * acc=accs.getAccount(m_account.GetBuffer(0));
		if (acc)
		{
/*			CFT_CMClient client=acc->connect();

			char ticket[1000];
			int ticketLen=acc->getTicket(ticket);
			if (!ticketLen)
			{

				if (acc->authenticate())
				{
					acc->commit();
					ticketLen=acc->getTicket(ticket);
				}
			}

			if (ticketLen)
*/
			{
				
				POSITION pos = m_filesCtrl.GetFirstSelectedItemPosition();
				
				while(pos)
				{
					//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
					int index = m_filesCtrl.GetNextSelectedItem(pos);
					DWORD appId=m_filesCtrl.GetItemData(index);
					char ticket[1000];
					int ticketLen=acc->getContentServerAuthTickets(appId,ticket);
					if (ticketLen)
					{	
						accs.setAccountIdForFileId(appId,m_account.GetBuffer(0));
						cfManager->downloadNew(appId,ticket,ticketLen);
					}
				}
			}
		}
	}
 
	refreshTab();
}
}

void CTabAdd::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		//refreshTab();
	}
	// TODO: Add your message handler code here
}

void CTabAdd::refreshAccounts()
{
if (isAddEnabled())
{
		// refresh the accounts list
	char prevA[5000];
	m_accountsCtrl.GetLBText(m_accountsCtrl.GetCurSel(),prevA);
	
	m_accountsCtrl.ResetContent();
	CAccounts accs;
	int nb=accs.getNbAccounts();
	for (int ind=0;ind<nb;ind++)
	{
		char * acc=accs.getAccountIdAt(ind);
		m_accountsCtrl.AddString(acc);
	}
	int indexA=m_accountsCtrl.FindString(0,prevA);
	if (indexA<0) indexA=0;
	m_accountsCtrl.SetCurSel(indexA);
	
	if (m_accountsCtrl.GetCount()) 
		m_downloadCtrl.EnableWindow(true);
	else
		m_downloadCtrl.EnableWindow(false);
}

}
void CTabAdd::refreshTab()
{
 if (isAddEnabled())
 {
	// refresh the files list
	
	HashMapDword * notDownloadedFiles=new HashMapDword(HASHMAP_NODELETE);
	selectedFiles=new HashMapDword(HASHMAP_NODELETE);
	
	m_filesCtrl.DeleteAllItems();
	SteamNetwork network;
	CCFManager * cfManager=getCFManager();
	RegistryVector * cdrVector=network.getContentDescriptionRecord();
	ContentDescriptionRecord cdr=getCDR(cdrVector);
	
	int len=cfManager->cdrAvailableFiles->size();
	DWORD * cdrFiles=(DWORD*)malloc(4*len);
	
	int nbNotDownloadedFiles=0;
	cfManager->cdrAvailableFiles->getDWordKeys(cdrFiles);
	
	for (  int ind=0;ind<len;ind++)
	{
		if (!cfManager->files->get(cdrFiles[ind]))
		{
			CDRApp app=getCDRApp(cdr,cdrFiles[ind]);
			if (isFile(app))
				//if (app.node && app.node->getVectorValue()->getNode(11)) // has a version node
			{
				//				RegistryVector * userDefined=getCDRAppUserDefinedData(app);
				//				if (!userDefined || !userDefined->getNode("gamedir"))
				//	if (!app.manifestOnlyApp || app.appOfManifestOnlyCache==app.appId)
				{
					notDownloadedFiles->put(app.appId,(void*)1);
					char * values[3];
					char name[1000];
					sprintf(name,"%s.%s",app.installDirName,(app.appOfManifestOnlyCache ? "ncf" : "gcf"));
					strlwr(name);
					values[0]=name;
					char size[50];
					renderSize(app.maxCacheFileSizeMb*1024*1024,size,false);
					values[1]=size;	
					char version[50];
					sprintf(version,"%d",app.currentVersionId);
					values[2]=version;	
					
					insertListValue(&m_filesCtrl, app.appId, values,3 ,-1);
				}
			}
		}
	}
	
	// refresh the apps list
//	notDownloadedApps->clear();
	HashMapDword noFiles;
	DWORD appGcfIds[200]; // /should be enough
	char appGcfOptional[200];
 	int appGcfIdsCount=0;
	m_appsCtrl.DeleteAllItems();
	ConfigManager conf;
	
	for (  ind=0;ind<getCDRNbApps(cdr);ind++)
	{
		CDRApp app=getCDRAppAt(cdr,ind);
		// filter on apps
		RegistryVector * userDefined=getCDRAppUserDefinedData(app);
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
					
					if (localisedApp.node && notDownloadedFiles->get(localisedApp.appId))
					{
						// found a localised GCF
						if (notDownloadedFiles->get(localisedApp.appId))
						{ // only if gcf/ncf is not downloaded
							appGcfOptional[appGcfIdsCount]=0;
							appGcfIds[appGcfIdsCount++]=localisedApp.appId;
						}
					}
					
					// add "common" apps
					int nbGcfs=getCDRAppNbFilesystem(app);
					for (int i=0;i<nbGcfs;i++)
					{
						CDRAppFilesystem fileSystem=getCDRAppFilesystem(app,i);
						if (notDownloadedFiles->get(fileSystem.appId)) 
						{ // only if gcf/ncf is not downloaded
							
							appGcfOptional[appGcfIdsCount]=(fileSystem.isOptional ? 1 : 0);
							appGcfIds[appGcfIdsCount++]=fileSystem.appId;
						}
					}
					
					if (appGcfIdsCount)
					{
						// at least one gcf not downloaded
						
						CAppDescriptor * localDescriptor=(CAppDescriptor *)cfManager->apps->get(app.appId);
						
						CAppDescriptor * descriptor=new CAppDescriptor(cdr,app,&noFiles,appGcfIds,appGcfOptional,appGcfIdsCount);
						
						char empty[1]="";
						char * values[4];
						values[0]=descriptor->appName;
						
						values[1]=descriptor->editor;
						

						char size[50];
						if (localDescriptor)
						{
							descriptor->realSize=localDescriptor->realSize;
						}
						renderSize(descriptor->realSize,size,false);
						values[2]=size;
						
						values[3]=empty;
						if (userDefined)
						{
							RegistryNode * status=userDefined->getNode("state");
							if (status && strlen(status->getStringValue()) && stricmp("eStateAvailable",status->getStringValue()) && stricmp("eStateJustReleased",status->getStringValue()))
							{
								values[3]=status->getStringValue()+strlen("eState");
							}
							RegistryNode * mediaType=userDefined->getNode("MediaFileType");
							if (mediaType)
							{
								values[3]=mediaType->getStringValue();
							}
							strcpy(descriptor->information,values[3]);
						}
						
						insertListValue(&m_appsCtrl, descriptor->appId, values,4,descriptor->iconId);
						notDownloadedApps->put(descriptor->appId,descriptor);
					}
					
				}
			}
		}
	}
	
	free(cdrFiles);
	delete notDownloadedFiles;
	}
}

BOOL CTabAdd::OnInitDialog() 
{
	CDialog::OnInitDialog();

	notDownloadedApps=new HashMapDword(HASHMAP_DELETE);
	char * cfnames[3]={"File","Size","Version"};
	int cfwidths[3]={300,100,60};
	buildListColumns(&m_filesCtrl,cfnames,cfwidths,3);
	m_filesCtrl.SetExtendedStyle(m_filesCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	char * names[4]={"Game/Media/Tool","Developer","Size","Type"};
	int widths[4]={320,170,90,90   };
	buildListColumns(&m_appsCtrl,names,widths,4);
	m_appsCtrl.SetExtendedStyle(m_appsCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	m_appsCtrl.SetImageList(getAppIcons()->m_iconsList,LVSIL_SMALL);
	theOneAndUniqueTabAdd=this;
	refreshAccounts();
	
	refreshTab();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
int CALLBACK addAppsCompareFunc(LPARAM id1, LPARAM id2 , 
								LPARAM lParamSort)
{  
	int iSubItem = (int)lParamSort;
	
	
	int a=0;
	
	CAppDescriptor * descriptor1 = (CAppDescriptor*) getTabAdd()->notDownloadedApps->get(id1);
	CAppDescriptor * descriptor2 = (CAppDescriptor*) getTabAdd()->notDownloadedApps->get(id2);
	
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
		if (descriptor1->realSize==descriptor2->realSize)
			a=0;
		else
			a=(descriptor1->realSize<descriptor2->realSize ? -1 : 1);
		break;
		   }
		
	case 3:{
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

int CALLBACK addFilesCompareFunc(LPARAM id1, LPARAM id2 , 
								 LPARAM lParamSort)
{  
	int iSubItem = (int)lParamSort;
	int selected1=(int)getTabAdd()->selectedFiles->get(id1);
	int selected2=(int)getTabAdd()->selectedFiles->get(id2);
	
	if (!selected1 && selected2) return 1;
	if (selected1 && !selected2) return -1;
	
	SteamNetwork network;
	RegistryVector * cdrVector=network.getContentDescriptionRecord();
	RegistryVector * filesVector=cdrVector->getNode(1)->getVectorValue();
	RegistryNode * id1Node=filesVector->getNode(id1);
	RegistryNode * id2Node=filesVector->getNode(id2);
	
	if (!id1 || !id2) return 0; // shouldn't happen
	
	return stricmp(id1Node->getVectorValue()->getNode(3)->getStringValue(),id2Node->getVectorValue()->getNode(3)->getStringValue());
	/*
	
	  int a=0;
	  
		CAppDescriptor * descriptor1 = (CAppDescriptor*) getTabAdd()->notDownloadedApps->get(id1);
		CAppDescriptor * descriptor2 = (CAppDescriptor*) getTabAdd()->notDownloadedApps->get(id2);
		
		  switch (iSubItem)
		  {
		  case 0:{
		  a=stricmp(descriptor1->appName,descriptor2->appName);
		  break;
		  }
		  case 1:{
		  if(! descriptor1->editor || strlen(descriptor1->editor)==0) return  1;
		  if(! descriptor2->editor || strlen(descriptor2->editor)==0) return -1;
		  a=stricmp(descriptor1->editor,descriptor2->editor);
		  break;
		  }
		  case 2:{
		  a=(descriptor1->realSize<descriptor2->realSize ? -1 : 1);
		  break;
		  }
		  
			case 3:{
			if(! descriptor1->information || strlen(descriptor1->information)==0) return  1;
			if(! descriptor2->information || strlen(descriptor2->information)==0) return -1;
			a=stricmp(descriptor1->information,descriptor2->information);
			break;
			}
			
			  }
			  
				return  a;
	*/
}


void CTabAdd::OnColumnclickApps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (isAddEnabled())
	{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int nCol = pNMListView->iSubItem;
	
	
    m_appsCtrl.SortItems(addAppsCompareFunc, nCol);
	
	POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();
	if (pos)
	{
		int index = m_appsCtrl.GetNextSelectedItem(pos);
		m_appsCtrl.EnsureVisible(index,FALSE);
	}
	*pResult = 0;
	}
	
}

void CTabAdd::OnAppClicked() 
{
	if (isAddEnabled())
	{
	selectedFiles->clear();
	DWORD size=m_filesCtrl.GetItemCount();
	for (DWORD a=0;a<size;a++)
	{
		m_filesCtrl.SetItemState(a, 0, LVIS_SELECTED );
	}	
	
	POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();
	
	while(pos)
	{
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_appsCtrl.GetNextSelectedItem(pos);
		DWORD appId=m_appsCtrl.GetItemData(index);
		CAppDescriptor * appDescriptor = (CAppDescriptor*) notDownloadedApps->get(appId);
		for (int ind=0;ind<appDescriptor->appGcfIdsCount;ind++)
		{
			DWORD fileId=appDescriptor->appGcfIds[ind];
			selectedFiles->put(fileId,(void*)1);
			/*			for (DWORD a=0;a<size;a++)
			{
			if (m_filesCtrl.GetItemData(a)==fileId)
			m_filesCtrl.SetItemState(a, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
		}*/
		}
	}
	
	// select files
	for (  a=0;a<size;a++)
	{
		if (selectedFiles->get(m_filesCtrl.GetItemData(a)))
			m_filesCtrl.SetItemState(a, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	// sort
	m_filesCtrl.SortItems(addFilesCompareFunc,0);
	// ensure visible
	pos = m_filesCtrl.GetFirstSelectedItemPosition();
	if (pos)
	{
		int index = m_filesCtrl.GetNextSelectedItem(pos);
		m_filesCtrl.EnsureVisible(index,FALSE);
	}
	
	}
}



void CTabAdd::OnItemchangedApps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (isAddEnabled())
	{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	SetTimer(0,100,NULL);
	*pResult = 0;
	}
}

void CTabAdd::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	//	printDebug(DEBUG_LEVEL_TODO,"test","timer executed",0);
	KillTimer(nIDEvent);
	OnAppClicked();
	CDialog::OnTimer(nIDEvent);
}

void CTabAdd::OnAppContextualMenu(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (isAddEnabled())
	{
    *pResult = 0;
	// TODO: Add your control notification handler code here
	if (!m_appsCtrl.GetSelectedCount()) return;
	CTabApps * tabApps=getTabApps();

	ConfigManager conf;	
    CMenu mnuTop;
    mnuTop.LoadMenu(IDR_ADDAPPS_MENU);
	if (!strlen(conf.getAppCommandLineTemplate()))
		mnuTop.EnableMenuItem(ID_ROOT_RUN,MF_GRAYED );	
    CMenu* pPopup = mnuTop.GetSubMenu(0);
    ASSERT_VALID(pPopup);
 	CPoint point;
    GetCursorPos(&point);	
    int command=pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN| TPM_RETURNCMD, point.x, point.y, AfxGetMainWnd(), NULL);
	
	if (!command) return;
	

	CCFManager * manager=getCFManager();
	SteamNetwork network;
  	//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
	POSITION pos = m_appsCtrl.GetFirstSelectedItemPosition();

	while(pos)
	{
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_appsCtrl.GetNextSelectedItem(pos);
		DWORD appId=m_appsCtrl.GetItemData(index);

		CAppDescriptor * appDescriptor = (CAppDescriptor*) manager->apps->get(appId);
		if (!appDescriptor) appDescriptor = (CAppDescriptor*) notDownloadedApps->get(appId);

		if (command==ID_ROOT_RUN || command==ID_ROOT_COPY || command==ID_ROOT_PROPERTIES)
		{// work on app level
			switch (command)
			{
			case (ID_ROOT_RUN):
				{
 
						ConfigManager conf;
						launch(appDescriptor,(conf.isWaitForAppCommandLineEnd() ? _P_WAIT : _P_NOWAIT));
						//	Sleep(1000); 
					break;
				}
 
			case (ID_ROOT_PROPERTIES):
				{			
					if (tabApps->properties)
					{
						tabApps->properties->DestroyWindow();
						delete tabApps->properties;
						tabApps->properties=0;
					}
					
					if (appDescriptor)
					{
						tabApps->properties=new CAppPropertiesPopup;
						tabApps->properties->m_id.Format("%d",appDescriptor->appId);
						tabApps->properties->m_name=appDescriptor->appName;
						tabApps->properties->m_developper=appDescriptor->editor;
						char size[100];
						renderSize(appDescriptor->realSize,size,false);
						tabApps->properties->m_size=size;
						
						if (appDescriptor->manual)
							tabApps->properties->m_manual=appDescriptor->manual;
						else
							tabApps->properties->m_manual="No manual";			
						
						if (appDescriptor->commonPath && strlen(appDescriptor->commonPath))
							tabApps->properties->m_common=appDescriptor->commonPath;
						else
							tabApps->properties->m_common="No common path";
						
						tabApps->properties->Create(IDD_APP_PROPERTIES);
						CString url;
						SteamNetwork network;
						url.Format(network.getUpdatesHistoryURL(),appDescriptor->appId);
						tabApps->properties->m_browser.Navigate(url, NULL, NULL, NULL, NULL);
						
						for (int ind=0;ind<appDescriptor->nbProperties;ind++)
						{
							char *p[2];
							p[0]=appDescriptor->propertiesNames[ind];
							p[1]=appDescriptor->propertiesValues[ind];
							insertListValue(&tabApps->properties->m_properties,ind,p,2,-1);
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
							insertListValue(&tabApps->properties->m_cfs,appDescriptor->appGcfIds[ind],p,4,-1);
						}
					}
					break;
					
				}
			}
		}
 
	 }
	 }
}

void CTabAdd::OnFileContextualMenu(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (isAddEnabled())
	{
	CTabFiles * tabFiles=getTabFiles();
    *pResult = 0;
	// TODO: Add your control notification handler code here
	if (!m_filesCtrl.GetSelectedCount()) return;
	ConfigManager conf;
	
    CMenu mnuTop;
    mnuTop.LoadMenu(IDR_ADDFILES_MENU);
	if (!strlen(conf.getFileCommandLineTemplate()))
		mnuTop.EnableMenuItem(ID_ROOT_RUN,MF_GRAYED );	

	if (m_filesCtrl.GetSelectedCount()>1)
	{
		mnuTop.EnableMenuItem(ID_ROOT_SELECT,MF_GRAYED );	
	}

    CMenu* pPopup = mnuTop.GetSubMenu(0);
    ASSERT_VALID(pPopup);
	
 	CPoint point;
    GetCursorPos(&point);	
    int command=pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN| TPM_RETURNCMD, point.x, point.y, AfxGetMainWnd(), NULL);
	
	if (!command) return;
	

	SteamNetwork network;
 	CCFManager * manager=getCFManager();
	RegistryVector * cdrVector=network.getContentDescriptionRecord();
	ContentDescriptionRecord cdr=getCDR(cdrVector);
	//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
	POSITION pos = m_filesCtrl.GetFirstSelectedItemPosition();
 
	while(pos)
	{
		//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
		int index = m_filesCtrl.GetNextSelectedItem(pos);
		DWORD cfId=m_filesCtrl.GetItemData(index);
		CDRApp app=getCDRApp(cdr,cfId);
		CCFDescriptor * descriptor = new CCFDescriptor(cdr, app, conf.getSteamAppsPath(),conf.isAccurateNcfCompletion());//(CCFDescriptor*) manager->files->get(cfId);
 
 
			switch (command)
			{
		 	case (ID_ROOT_RUN):
				{
 						ConfigManager conf;
						launch(descriptor,(conf.isWaitForFileCommandLineEnd() ? _P_WAIT : _P_NOWAIT));
						//	Sleep(1000);
					break;
				}
		 	case (ID_ROOT_SELECT):
				{
					CQuestionPopup question;
					question.m_question="Select the GCF/NCF files containing :";
					question.m_value="";
					int nResponse = question.DoModal();
					if (nResponse == IDOK && question.m_value.GetLength())
					{
						selectedFiles->clear();
						DWORD size=m_filesCtrl.GetItemCount();
						for (DWORD a=0;a<size;a++)
						{
							m_filesCtrl.SetItemState(a, 0, LVIS_SELECTED );
						}	

						// select files
						char search[1000];
						strcpy(search,question.m_value.GetBuffer(0));
						strlwr(search);
					//	printDebug(DEBUG_LEVEL_TODO,"searching",search,0);
						for ( a=0;a<size;a++)
						{
							DWORD id=m_filesCtrl.GetItemData(a);
							CDRApp ap=getCDRApp(cdr,id);
							CCFDescriptor * desc = new CCFDescriptor(cdr, ap, conf.getSteamAppsPath(),conf.isAccurateNcfCompletion());
						//	printDebug(DEBUG_LEVEL_TODO,"in ",desc->fileName,0);
							if (strstr(desc->fileName,search))
							{	
								selectedFiles->put(id,(void*)1);
								m_filesCtrl.SetItemState(a, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
							}
						}
						// sort
						m_filesCtrl.SortItems(addFilesCompareFunc,0);
						// ensure visible
						POSITION pos2 = m_filesCtrl.GetFirstSelectedItemPosition();
						if (pos2)
						{
							int index = m_filesCtrl.GetNextSelectedItem(pos2);
							m_filesCtrl.EnsureVisible(index,FALSE);
						}
					}
					break;
				}		 	
			case (ID_ROOT_PROPERTIES):
				{			
					if (tabFiles->properties)
					{
						tabFiles->properties->DestroyWindow();
						delete tabFiles->properties;
						tabFiles->properties=0;
					}
					
					if (descriptor)
					{
						tabFiles->properties=new CPropertiesPopup;
						
						
						
						tabFiles->properties->m_blocks.Format("%d / %d",descriptor->downloadedBlocks,descriptor->nbBlocks);
						
						tabFiles->properties->m_description=descriptor->description;
						
						if (descriptor->commonPath)
							tabFiles->properties->m_commonPath=descriptor->commonPath;
						else
							tabFiles->properties->m_commonPath="No common path";
						
						tabFiles->properties->m_id.Format("%d",descriptor->fileId);
						
						if (descriptor->decryptionKey)
							tabFiles->properties->m_key=descriptor->decryptionKey;
						else
							tabFiles->properties->m_key="Not available";
						
						tabFiles->properties->m_path=descriptor->fileName;
						
						char size[100];
						renderSize(descriptor->realSize,size,descriptor->isMoreThan4GB);
						tabFiles->properties->m_size=size;
						
						if (descriptor->fileVersion==descriptor->cdrVersion)
						{
							tabFiles->properties->m_version.Format("%d",descriptor->fileVersion);
							tabFiles->properties->m_completion.Format("%3.2f%%",descriptor->completion);
						}
						else
						{
							tabFiles->properties->m_version.Format("%d (last version : %d)",descriptor->fileVersion,descriptor->cdrVersion);
							tabFiles->properties->m_completion="Outdated";
						}
						
						GCF * gcf=parseGCF(descriptor->path);
						
						if ((int)gcf>1)
						{
							tabFiles->properties->m_type.Format("%s (type:%d version:%d)",(descriptor->isNcf ? "NCF" : "GCF"),gcf->cfType,gcf->cfVersion);
							tabFiles->properties->m_files.Format("%d",gcf->directory->fileCount);
							tabFiles->properties->m_folders.Format("%d",gcf->directory->itemCount-gcf->directory->fileCount);
							
							deleteGCF(gcf);
						}
						else
						{
							tabFiles->properties->m_type.Format("%s",(descriptor->isNcf ? "NCF" : "GCF"));
							tabFiles->properties->m_files="Unknown";
							tabFiles->properties->m_folders="Unknown";
						}
						/*
						CString	m_type;
						CString	m_version;
						CString	m_files;
						CString	m_folders;
						*/
						
						
						tabFiles->properties->Create(IDD_PROPERTIES);
					}
					break;
				}
			}
				
			delete descriptor; 
	}
}
}


void CTabAdd::OnSize(UINT nType, int cx, int cy) 
{
 CDialog::OnSize(nType, cx, cy);
	RECT rect;
	RECT rect2;
	RECT rect3;
#ifndef _DEBUG

	m_downloadCtrl.GetClientRect  (&rect);	
  	m_downloadCtrl.SetWindowPos( &wndTop, (cx-5)-rect.right, (cy-5)-rect.bottom, rect.right,  rect.bottom, 0 );	
 
	m_accLabel.GetClientRect  (&rect2);	
	m_accLabel.SetWindowPos( &wndTop, 5, (cy-5)-rect2.bottom, rect2.right,  rect2.bottom, 0 );

	m_accountsCtrl.GetClientRect  (&rect3);	
	m_accountsCtrl.SetWindowPos( &wndTop, 10+rect2.right, (cy-5)-rect3.bottom, ((cx-5)-rect.right)-rect2.right-15,  rect3.bottom, 0 );

	m_appsFrame.GetClientRect(&rect);	
	m_appsFrame.SetWindowPos( &wndTop, 5, 5, (cx-10),(cy-40)/2, 0 );

	m_appsCtrl.GetClientRect(&rect);	
	m_appsCtrl.SetWindowPos( &wndTop, 10, 20, (cx-20),(cy-40)/2-20, 0 );

	m_filesFrame.GetClientRect(&rect);	
	m_filesFrame.SetWindowPos( &wndTop, 5, (cy-40)/2+10, (cx-10),(cy-40)/2, 0 );

	m_filesCtrl.GetClientRect(&rect);	
	m_filesCtrl.SetWindowPos( &wndTop, 10, (cy-40)/2+25, (cx-20),(cy-40)/2-20, 0 );
#endif
	
	
}
