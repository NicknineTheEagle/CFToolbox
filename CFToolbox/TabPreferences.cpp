	// TabPreferences.cpp : implementation file
//

#include "Tools.h"
#include "stdafx.h"
#include "CFToolbox.h"
#include "TabPreferences.h"
#include "Accounts.h"
#include "SteamNetwork.h"
#include "QuestionPopup.h"
#include "direct.h"
#include "ConfigManager.h"
#include "TabFiles.h"
#include "TabApps.h"
#include "TabAdd.h"
#include "CFManager.h"
#include "TaskManager.h"
#include "debug.h"
#include "appIcons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabPreferences dialog


static char * levels[]={"All","Debug","Info","Error","Todo","No logs"};


int nbSupportedLanguages=19;
static char * supportedLanguagesCodes[]={"English","French","Italian","German","Spanish","sChinese","Korean","Koreana","tChinese","Thai","Portuguese","Japanese","Russian","Danish","Dutch","Finnish","Norwegian","Polish","Swedish"};
static char * supportedLanguages[]={"English","French","Italian","German","Spanish","Chinese (simplified)","Korean","Korean (adult)","Chinese (traditional)","Thai","Portuguese","Japanese","Russian","Danish","Dutch","Finnish","Norwegian","Polish","Swedish"};



CTabPreferences * theOneAndUniqueTabPreferences=0;

CTabPreferences * getTabPreferences()
{
	return theOneAndUniqueTabPreferences;
}

void configureDebugLevel(CComboBox * combo,int * val,int level)
{
	for (int ind=0;ind<6;ind++)
	{
		combo->AddString(levels[ind]);combo->SetItemData(combo->FindStringExact(0,levels[ind]),ind);
	}
	combo->SetCurSel(level);
	*val=level;
}

void configureLanguages(CComboBox * combo,char * language)
{
	for (int ind=0;ind<nbSupportedLanguages;ind++)
	{
		combo->AddString(supportedLanguages[ind]);combo->SetItemData(combo->FindStringExact(0,supportedLanguages[ind]),(DWORD)supportedLanguagesCodes[ind]);
	}
	for (ind=0;ind<combo->GetCount();ind++)
	{
		char * elt=(char*)combo->GetItemData(ind);
		
		if (!stricmp(elt,language))
		{
			combo->SetCurSel(ind);
			break;
		}
	}
}


void trim(CString * str)
{
	str->TrimLeft();
	str->TrimRight();
}

CTabPreferences::CTabPreferences(CWnd* pParent /*=NULL*/)
: CDialog(CTabPreferences::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabPreferences)
	m_host = _T("");
	m_port = 0;
	m_remember = FALSE;
	m_login = _T("");
	m_password = _T("");
	m_accounts = -1;
	m_gcfPath = _T("");
	m_automaticUpdate = FALSE;
	m_logsLevelVal = -1;
	m_maxTasks = 0;
	m_outputPath = _T("");
	m_validateBeforeArchiveUpdate = FALSE;
	m_commandLine = _T("");
	m_fileCommandLine = _T("");
	m_waitApp = FALSE;
	m_waitFile = FALSE;
	m_sharing = FALSE;
	m_secured = FALSE;
	m_noReset = FALSE;
	m_bandwidth = FALSE;
	m_resetMiniGCF = FALSE;
	m_validateDownloads = FALSE;
	m_accurateNcfCompletion = FALSE;
	m_disableInstall = FALSE;
	m_steamguard = _T("");
	//}}AFX_DATA_INIT
	theOneAndUniqueTabPreferences=this;
	
}


void CTabPreferences::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabPreferences)
	DDX_Control(pDX, IDC_STEAMGUARD, m_steamguardCtrl);
	DDX_Control(pDX, IDC_DO_DISCONNECT, m_doDisconnectCtrl);
	DDX_Control(pDX, IDC_FILE_DBLCLICK, m_filesDoubleClick);
	DDX_Control(pDX, IDC_APP_DBLCLICK, m_appsDoubleClick);
	DDX_Control(pDX, IDC_NORESET, m_noResetCtrl);
	DDX_Control(pDX, IDC_LANGUAGE, m_language);
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDC_ACCOUNTS, m_accountsCtrl);
	DDX_Control(pDX, IDC_DO_LOGIN, m_doLoginCtrl);
	DDX_Control(pDX, IDC_REMEMBER, m_rememberCtrl);
	DDX_Control(pDX, IDC_PASSWORD, m_passwordCtrl);
	DDX_Control(pDX, IDC_LOGIN, m_loginCtrl);
	DDX_Text(pDX, IDC_HOST, m_host);
	DDX_Text(pDX, IDC_PORT, m_port);
	DDV_MinMaxInt(pDX, m_port, 1, 65535);
	DDX_Check(pDX, IDC_REMEMBER, m_remember);
	DDX_Text(pDX, IDC_LOGIN, m_login);
	DDX_Text(pDX, IDC_PASSWORD, m_password);
	DDX_CBIndex(pDX, IDC_ACCOUNTS, m_accounts);
	DDX_Text(pDX, IDC_GCF_PATH, m_gcfPath);
	DDX_Check(pDX, IDC_AUTOMATICUPDATE, m_automaticUpdate);
	DDX_CBIndex(pDX, IDC_LEVEL, m_logsLevelVal);
 	DDX_Control(pDX, IDC_LEVEL, m_logsLevel);
	DDX_Text(pDX, IDC_MAX_TASKS, m_maxTasks);
	DDV_MinMaxInt(pDX, m_maxTasks, 1, 50);
	DDX_Text(pDX, IDC_PATCH_PATH, m_outputPath);
	DDX_Check(pDX, IDC_VALIDATE_PATCHS, m_validateBeforeArchiveUpdate);
	DDX_Text(pDX, IDC_COMMAND, m_commandLine);
	DDV_MaxChars(pDX, m_commandLine, 400);
	DDX_Text(pDX, IDC_FILECOMMAND, m_fileCommandLine);
	DDV_MaxChars(pDX, m_fileCommandLine, 400);
	DDX_Check(pDX, IDC_WAIT_APP, m_waitApp);
	DDX_Check(pDX, IDC_WAIT_FILE, m_waitFile);
	DDX_Check(pDX, IDC_SHARING, m_sharing);
	DDX_Check(pDX, IDC_SECURED, m_secured);
	DDX_Check(pDX, IDC_NORESET, m_noReset);
	DDX_Check(pDX, IDC_BANDWIDTH, m_bandwidth);
	DDX_Check(pDX, IDC_MINI, m_resetMiniGCF);
	DDX_Check(pDX, IDC_VALIDATE_DOWNLOADS, m_validateDownloads);
	DDX_Check(pDX, IDC_SECURED2, m_accurateNcfCompletion);
	DDX_Check(pDX, IDC_CHECK1, m_disableInstall);
	DDX_Text(pDX, IDC_STEAMGUARD, m_steamguard);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabPreferences, CDialog)
//{{AFX_MSG_MAP(CTabPreferences)
ON_BN_CLICKED(IDC_SET, OnSetConfigServer)
ON_WM_SETFOCUS()
ON_BN_CLICKED(IDC_BUTTON6, OnNewAccount)
ON_BN_CLICKED(IDC_DELETE, OnDeleteAccount)
ON_BN_CLICKED(IDC_DO_LOGIN, OnDoLogin)
ON_CBN_SELCHANGE(IDC_ACCOUNTS, OnSelchangeAccounts)
	ON_BN_CLICKED(IDC_GCF_BROWSE, OnGcfBrowse)
	ON_BN_CLICKED(IDC_UPDATECDR, OnUpdateCDR)
	ON_BN_CLICKED(IDC_AUTOMATICUPDATE, OnAutomaticCDRUpdate)
	ON_CBN_SELCHANGE(IDC_LEVEL, OnSelChangeLevel)
	ON_BN_CLICKED(IDC_SET2, OnSetMaxTask)
	ON_BN_CLICKED(IDC_PATCH_BROWSE, OnPatchBrowse)
	ON_BN_CLICKED(IDC_VALIDATE_PATCHS, OnValidatePatchs)
	ON_CBN_SELCHANGE(IDC_LANGUAGE, OnSelchangeLanguage)
	ON_BN_CLICKED(IDC_SET3, OnSetCommandLine)
	ON_BN_CLICKED(IDC_SET4, OnSetFileCommandLine)
	ON_BN_CLICKED(IDC_WAIT_FILE, OnWaitFile)
	ON_BN_CLICKED(IDC_WAIT_APP, OnWaitApp)
	ON_BN_CLICKED(IDC_SHARING, OnSharing)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_KILL, OnKill)
	ON_BN_CLICKED(IDC_SECURED, OnSecured)
	ON_BN_CLICKED(IDC_BANDWIDTH, OnRealBandwidthUsage)
	ON_BN_CLICKED(IDC_MINI, OnResetMiniGCF)
	ON_CBN_SELCHANGE(IDC_FILE_DBLCLICK, OnSelchangeFileDblclick)
	ON_CBN_SELCHANGE(IDC_APP_DBLCLICK, OnSelchangeAppDblclick)
	ON_BN_CLICKED(IDC_VALIDATE_DOWNLOADS, OnValidateDownloads)
	ON_BN_CLICKED(IDC_SECURED2, OnAccurate)
	ON_BN_CLICKED(IDC_CHECK1, OnCheckDisableInstall)
	ON_BN_CLICKED(IDC_DO_DISCONNECT, OnDoDisconnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabPreferences message handlers
BOOL CTabPreferences::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
			pMsg->wParam=NULL ;
	} 
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CTabPreferences::OnSetConfigServer() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	SteamNetwork network;
	network.setConfigServer(m_host.GetBuffer(0),m_port);
	m_host=network.getConfigServer();
	m_port=network.getConfigServerPort();
	UpdateData(FALSE);
}


void CTabPreferences::init()
{
	
	
	SteamNetwork network;
	m_host=network.getConfigServer();
	m_port=network.getConfigServerPort();
	
	CAccounts accs;
	int nbAccs=accs.getNbAccounts();
	
	
	m_accountsCtrl.ResetContent();
	m_deleteButton.EnableWindow(false);
	for (int ind=0;ind<nbAccs;ind++)
	{
		m_accountsCtrl.AddString(accs.getAccountIdAt(ind));
		m_accountsCtrl.SetItemData(m_accountsCtrl.FindStringExact(0,accs.getAccountIdAt(ind)),ind);
	}
	setAccForm(false);
	if (nbAccs) 
	{
		setAccForm(true);
		m_deleteButton.EnableWindow(true);
		m_accounts=0;
		fillAccForm();
	}
	ConfigManager conf;

	m_gcfPath=conf.getSteamAppsPath();
	m_outputPath=conf.getOutputPath();
	m_automaticUpdate=conf.isAutomaticCDRUpdate();
	m_validateBeforeArchiveUpdate=conf.isCorrectBeforeArchiveUpdateCreation();
	m_validateDownloads=conf.isValidateDownloads();

	m_commandLine=conf.getAppCommandLineTemplate();
	m_fileCommandLine=conf.getFileCommandLineTemplate();

	m_waitFile=conf.isWaitForFileCommandLineEnd();
	m_waitApp=conf.isWaitForAppCommandLineEnd();
	m_sharing=conf.isSharingSteamFolder();
	m_secured=conf.isSecuredGCFUpdates();
	m_bandwidth=conf.isShowBandwidthUsage();
	m_resetMiniGCF=conf.isResetMiniCF();
	m_accurateNcfCompletion=conf.isAccurateNcfCompletion();
	m_disableInstall=conf.isDisableInstall();
}

void CTabPreferences::setAccForm(bool enabled)
{
	//	UpdateData(TRUE);
	m_login="";
	m_loginCtrl.EnableWindow(enabled);
	m_password="";
	m_passwordCtrl.EnableWindow(enabled);
	m_steamguard="";
	m_steamguardCtrl.EnableWindow(enabled);

	m_remember=false;
	m_rememberCtrl.EnableWindow(enabled);

	m_noReset=false;
	m_noResetCtrl.EnableWindow(enabled);

	m_doLoginCtrl.EnableWindow(enabled);
 
	m_doDisconnectCtrl.EnableWindow(enabled);
	//	UpdateData(FALSE);
}

void CTabPreferences::OnSetFocus(CWnd* pOldWnd) 
{
	CDialog::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	UpdateData(TRUE);
	init();	
	UpdateData(FALSE);
}

BOOL CTabPreferences::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	init();

	
	ConfigManager conf;

 	configureDebugLevel(&m_logsLevel,&m_logsLevelVal,conf.getDebugLevel());
	configureLanguages(&m_language,conf.getLanguage());
	m_maxTasks=conf.getMaxTasks();
		
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTabPreferences::OnSelChangeLevel() 
{
 
	UpdateData(TRUE);
	
	setGlobalLogLevel(m_logsLevelVal);
	ConfigManager conf;
	conf.setDebugLevel(m_logsLevelVal);
}

void CTabPreferences::fillAccForm()
{
	int ind=m_accountsCtrl.GetItemData(m_accounts);
	CAccounts accs;
	CAccount * account=accs.getAccount(accs.getAccountIdAt(ind));
	setAccForm(true);
	char * v=account->getLogin();
	
	if (v) m_login=v;
	v=account->getPassword();
	if (v) m_password=v;
	v=account->getSteamguard();
	if (v) m_steamguard=v;
	m_remember=account->isSavePassword();
//	m_noReset=account->isNoResetTicket();

	delete account;
}

void CTabPreferences::OnNewAccount() 
{
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	CQuestionPopup question;
	question.m_question="New account identifier";
	question.m_value="";
	int nResponse = question.DoModal();
	if (nResponse == IDOK && question.m_value.GetLength())
	{
		CAccounts accs;
		CAccount * acc= accs.getAccount(question.m_value.GetBuffer(0));
		if (acc)
		{
			MessageBox("This account ID is already created","Error",MB_OK|MB_ICONERROR);
			delete(acc);
			return;
		}
		acc = new CAccount;
		trim(&question.m_value);
		acc->setId(question.m_value.GetBuffer(0));
		acc->commit();
		delete acc;
		init();
		m_accountsCtrl.SelectString(0,question.m_value.GetBuffer(0));
	}
	UpdateData(FALSE);
	getTabAdd()->refreshAccounts();
}

void CTabPreferences::OnDeleteAccount() 
{
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	int ind=m_accountsCtrl.GetItemData(m_accounts);
	CAccounts accs;
	CAccount * account=accs.getAccount(accs.getAccountIdAt(ind));
	account->remove();
	delete account;
	
	init();
	m_accountsCtrl.SetCurSel(0);
	UpdateData(FALSE);
	getTabAdd()->refreshAccounts();
}

void CTabPreferences::OnDoLogin() 
{
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	int ind=m_accountsCtrl.GetItemData(m_accounts);
	CAccounts accs;
	CAccount * account=accs.getAccount(accs.getAccountIdAt(ind));
	trim(&m_login);
	trim(&m_password);
	trim(&m_steamguard);
//	account->setTicket(0,0);
	account->setLogin(m_login.GetBuffer(0));
	account->setPassword(m_password.GetBuffer(0));
	account->setSteamguard(m_steamguard.GetBuffer(0));
	account->setSavePassword(m_remember);
//	account->setNoResetTicket(m_noReset);
	account->disconnect();
	account->authenticate();
	account->commit();
	delete account;
	setAccForm(true);
	fillAccForm();
	UpdateData(FALSE);
}


void CTabPreferences::OnSelchangeAccounts() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	setAccForm(true);
	fillAccForm();
	UpdateData(FALSE);	
}
 



void CTabPreferences::OnGcfBrowse() 
{
	char cwd[1001];
	
	if (getTaskManager()->nbTotalTasks)
	{
		MessageBox( "Please stop the running operations.","CF Toolbox",MB_ICONINFORMATION|MB_OK);
		return;
	}	
	
	_getcwd(cwd,1000);
	UpdateData(TRUE);
	
	if (BrowseForFolder(m_hWnd, "GCF / NCF storage path", m_gcfPath))
	{
		_chdir(cwd); 
	//	getTaskManager()->stopAllTasks();
		ConfigManager conf;
		conf.setSteamAppsPath(m_gcfPath.GetBuffer(0));
		/*MessageBox("The application must be restarted","Steam apps folder changed",MB_OK);
		parent->SendMessage( WM_CLOSE, 0, 0 );*/
	
		// try to load icons from winui.gcf
		char winuiPath[1000];
		sprintf(winuiPath,"%s\\winui.gcf",conf.getSteamAppsPath());
		GCF * gcf=parseGCF(winuiPath);
		if ((int)gcf>1)
		{
			getAppIcons()->reloadIcons(gcf);
			deleteGCF( gcf);
		}

		getCFManager()->refreshContent();
		getTaskManager()->refreshDisplay();
		CTabFiles *tab=getTabFiles();
		if (tab) tab->refreshContent();
		
		CTabApps *tabA=getTabApps();
 		if (tabA) 
		{
			tabA->m_appsCtrl.SetImageList(getAppIcons()->m_iconsList,LVSIL_SMALL);
			tabA->refreshContent();
		}
		CTabAdd * tabAdd=getTabAdd();
		if (tabAdd)
		{
			tabAdd->refreshTab();
		}

	}
	_chdir(cwd); 
	UpdateData(FALSE);	 
}
 
/*
class CCDRUpdater : public CTask
{

public: virtual DWORD Run( LPVOID  )
	{ 
		getTaskManager()->stopAllTasks();
		SteamNetwork network;
		network.updateContentDescriptionRecord();

		getCFManager()->refreshContent();
		getTaskManager()->refreshDisplay();
		CTabFiles *tab=getTabFiles();
		if (tab) tab->refreshContent();
		CTabApps *tabA=getTabApps();
		if (tabA) tabA->refreshContent();
		CTabAdd * tabAdd=getTabAdd();
		if (tabAdd)
		{
			tabAdd->refreshTab();
		}
		getMainTab()->EnableWindow(true);
		return 0;
	 }
};
*/
 
void CTabPreferences::OnUpdateCDR() 
{
	// TODO: Add your control notification handler code here
	
 
//		getTaskManager()->stopAllTasks();
	if (getTaskManager()->nbTotalTasks)
	{
		MessageBox( "Please stop all running operations.","CF Toolbox",MB_ICONINFORMATION|MB_OK);
		return;
	}		

	SteamNetwork network;
	if (network.updateContentDescriptionRecord())
	{

		getCFManager()->refreshContent();
		getTaskManager()->refreshDisplay();
		CTabFiles *tab=getTabFiles();
		if (tab) tab->refreshContent();
		CTabApps *tabA=getTabApps();
		if (tabA) tabA->refreshContent();
		CTabAdd * tabAdd=getTabAdd();
		if (tabAdd)
		{
			tabAdd->refreshTab();
		} 
	}
}

void CTabPreferences::OnAutomaticCDRUpdate() 
{
	UpdateData(TRUE);	
	ConfigManager conf;
	conf.setAutomaticCDRUpdate(m_automaticUpdate);
	UpdateData(FALSE);	
}

 

void CTabPreferences::OnSetMaxTask() 
{
	UpdateData(TRUE);	
	ConfigManager conf;
	conf.setMaxTask(m_maxTasks);
	UpdateData(FALSE);	
}

void CTabPreferences::OnPatchBrowse() 
{
	char cwd[1001];
	if (getTaskManager()->nbTotalTasks)
	{
		MessageBox( "Please stop the running operations.","CF Toolbox",MB_ICONINFORMATION|MB_OK);
		return;
	}	
	
	_getcwd(cwd,1000);
	UpdateData(TRUE);
	
	if (BrowseForFolder(m_hWnd, "archives / updates output path", m_outputPath))
	{
		_chdir(cwd); 

		ConfigManager conf;
//		getTaskManager()->stopAllTasks();
		conf.setOutputPath(m_outputPath.GetBuffer(0));
		/*MessageBox("The application must be restarted","Steam apps folder changed",MB_OK);
		parent->SendMessage( WM_CLOSE, 0, 0 );*/
	
		
	//	getCFManager()->refreshContent();
		getTaskManager()->refreshDisplay();
		CTabFiles *tab=getTabFiles();
		if (tab) tab->refreshContent();
		CTabApps *tabA=getTabApps();
		if (tabA) tabA->refreshContent();
		CTabAdd * tabAdd=getTabAdd();
		if (tabAdd)
		{
			tabAdd->refreshTab();
		}
	}
	_chdir(cwd); 
	UpdateData(FALSE);
	
}

void CTabPreferences::OnValidatePatchs() 
{
	UpdateData(TRUE);	
	ConfigManager conf;
	conf.setCorrectBeforeArchiveUpdateCreation(m_validateBeforeArchiveUpdate);
	UpdateData(FALSE);	
}

 
void CTabPreferences::OnSelchangeLanguage() 
{
	if (getTaskManager()->nbTotalTasks)
	{
		MessageBox( "Please stop the running operations.","CF Toolbox",MB_ICONINFORMATION|MB_OK);
		return;
	}	

	UpdateData(TRUE);
	
	ConfigManager conf;
//	getTaskManager()->stopAllTasks();
	
	conf.setLanguage((char*)m_language.GetItemData(m_language.GetCurSel()))	;
	
	getCFManager()->refreshApps();
	getTaskManager()->refreshDisplay();
	CTabFiles *tab=getTabFiles();
	if (tab) tab->refreshContent();
	CTabApps *tabA=getTabApps();
	if (tabA) tabA->refreshContent();
	CTabAdd * tabAdd=getTabAdd();
	if (tabAdd)
	{
		tabAdd->refreshTab();
	}
	
}

void CTabPreferences::OnSetCommandLine() 
{
	UpdateData(TRUE);
	
	ConfigManager conf;
	m_commandLine.TrimLeft();
	m_commandLine.TrimRight();
	conf.setAppCommandLineTemplate(m_commandLine.GetBuffer(0));
	UpdateData(FALSE);	
}

void CTabPreferences::OnSetFileCommandLine() 
{
	UpdateData(TRUE);
	
	ConfigManager conf;
	m_fileCommandLine.TrimLeft();
	m_fileCommandLine.TrimRight();
	conf.setFileCommandLineTemplate(m_fileCommandLine.GetBuffer(0));
	UpdateData(FALSE);
}

void CTabPreferences::OnWaitFile() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setWaitForFileCommandLineEnd(m_waitFile);
	
	
}

void CTabPreferences::OnWaitApp() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setWaitForAppCommandLineEnd(m_waitApp);
	
}

void CTabPreferences::OnSharing() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setSharingSteamFolder(m_sharing);	
}

void CTabPreferences::OnStop() 
{
	// TODO: Add your control notification handler code here
	if (MessageBox("Are you sure","Ask for all the tasks to stop",MB_ICONWARNING|MB_YESNO)==IDYES)
		getTaskManager()->_stopAllTasks(1);
}

void CTabPreferences::OnKill() 
{
	// TODO: Add your control notification handler code here
	if (MessageBox("Are you sure","Kill all tasks immediatly",MB_ICONWARNING|MB_YESNO)==IDYES)
	getTaskManager()->killAllTasks();
	getTabFiles()->refreshContent();
	getTabApps()->refreshContent();
	getTabAdd()->refreshTab();
		
}

void CTabPreferences::OnSecured() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setSecuredGCFUpdates(m_secured);		
}

void CTabPreferences::OnRealBandwidthUsage() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setShowBandwidthUsage(m_bandwidth);		
	
}

void CTabPreferences::OnResetMiniGCF() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setResetMiniCF(m_resetMiniGCF);
}

void CTabPreferences::OnSelchangeFileDblclick() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char action[300];*action=0;

	DWORD id=m_filesDoubleClick.GetItemData(m_filesDoubleClick.GetCurSel());
	CPlugin * plugin=getPlugins()->getPlugin(id);

	if (plugin)
	{
			if (plugin->menuCategory)
				sprintf(action,"%s/%s",plugin->menuCategory,plugin->menuItem);
			else
				sprintf(action,"%s",plugin->menuItem);
	}

	ConfigManager conf;
	conf.setOnFileDoubleClick(action);	
}

void CTabPreferences::OnSelchangeAppDblclick() 
{
	// TODO: Add your control notification handler code here
	char action[300];*action=0;
	UpdateData(TRUE);

	DWORD id=m_appsDoubleClick.GetItemData(m_appsDoubleClick.GetCurSel());
	CPlugin * plugin=getPlugins()->getPlugin(id);

	if (plugin)
	{
			if (plugin->menuCategory)
				sprintf(action,"%s/%s",plugin->menuCategory,plugin->menuItem);
			else
				sprintf(action,"%s",plugin->menuItem);
	}

	ConfigManager conf;
	conf.setOnAppDoubleClick(action);	
}

void CTabPreferences::OnValidateDownloads() 
{
	UpdateData(TRUE);	
	ConfigManager conf;
	conf.setValidateDownloads(m_validateDownloads);
	UpdateData(FALSE);
	
}

void CTabPreferences::OnAccurate() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setAccurateNcfCompletion(m_accurateNcfCompletion);			
}

void CTabPreferences::OnCheckDisableInstall() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ConfigManager conf;
	conf.setDisableInstall(m_disableInstall);	
	MessageBox("Press OK to close CF Toolbox","CF Toolbox",MB_OK);
	exit(0);
}

void CTabPreferences::OnDoDisconnect() 
{
 	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	int ind=m_accountsCtrl.GetItemData(m_accounts);
	CAccounts accs;
	CAccount * account=accs.getAccount(accs.getAccountIdAt(ind));
	trim(&m_login);

	account->setLogin(m_login.GetBuffer(0));
	
	account->disconnect();
	delete account;
	UpdateData(FALSE);	
}
