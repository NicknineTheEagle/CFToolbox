// CFToolboxDlg.cpp : implementation file
//


#include "stdafx.h"
#include "CFToolbox.h"
#include "CFToolboxDlg.h"
#include "ServerRegistry.h"
#include "SteamNetwork.h"
#include "Accounts.h"
#include "Debug.h"
#include "TabPreferences.h"
#include "TabLogs.h"
#include "ConfigManager.h"
#include "SteamNetwork.h"
#include "CFManager.h"
#include "TaskManager.h"
#include "Task.h"
#include "socketTools.h"
#include "tools.h"
#include "AppIcons.h"
#include "plugins.h"

#include "CFToolbox_dll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_W 740
#define MAX_H 615
#define UWM_SYSTRAY (WM_USER + 1)

int initialized=0;

CWnd  * mainTab=0;

CWnd  * getMainTab()
{
	return mainTab;
}


CDialog  * mainDialog=0;

CDialog  * getMainDialog()
{
	return mainDialog;
}

bool runningOutOfSpace=false;
void setRunningOutOfSpace(bool outOfSpace)
{
	runningOutOfSpace=outOfSpace;
}
bool isRunningOutOfSpace()
{
	return runningOutOfSpace;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	
	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCFToolboxDlg dialog

CCFToolboxDlg::CCFToolboxDlg(CWnd* pParent /*=NULL*/)
: CDialog(CCFToolboxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCFToolboxDlg)
	m_accounts = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	
	m_hIcon16 = AfxGetApp()->LoadIcon(IDR_ICON16);
	m_hIcon32 = AfxGetApp()->LoadIcon(IDR_ICON32);
	canceledStart=false;
}

void CCFToolboxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCFToolboxDlg)
	DDX_Control(pDX, IDC_ACCOUNTS, m_accountsCtrl);
	DDX_Control(pDX, IDC_TASKS, m_tasks);
	DDX_Control(pDX, IDC_TAB, m_tabs);
	DDX_Text(pDX, IDC_ACCOUNTS, m_accounts);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCFToolboxDlg, CDialog)
//{{AFX_MSG_MAP(CCFToolboxDlg)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_NOTIFY(LVN_COLUMNCLICK, IDC_TASKS, OnColumnclickTasks)
ON_WM_SIZE()
ON_WM_CLOSE()
ON_WM_GETMINMAXINFO()
ON_WM_TIMER()
	ON_NOTIFY(NM_RCLICK, IDC_TASKS, OnContextualMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCFToolboxDlg message handlers

BOOL CCFToolboxDlg::OnInitDialog()
{
	mainTab=&m_tabs;
	mainDialog=this;
	CDialog::OnInitDialog();
	
	
	
	// Add "About..." menu item to system menu.
	
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	
	
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon32, TRUE);			// Set big icon
	SetIcon(m_hIcon16, FALSE);		// Set small icon
	// TODO: Add extra initialization here
	
	NOTIFYICONDATA NID;
	NID.cbSize = sizeof(NID); // Size of the structure duh :)
	NID.hIcon = AfxGetApp()->LoadIcon(IDR_ICON16);//LoadIcon(hInstance, IDR_MAINFRAME); // The icon that will be shown in the systray.
	NID.uCallbackMessage = UWM_SYSTRAY; // Our user defined message.(For notifications, Windows will send us messages when user pressed on our icon)
	NID.hWnd = GetSafeHwnd(); // Handle to the window we would like to get the notifications.
	NID.uID = 1; // Icon ID in systray...
	strcpy(NID.szTip, "CF Toolbox"); // The tooltip string
	NID.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE; // Here we say we want to get notifications about the icon, we have an icon and a tooltip.
	
	Shell_NotifyIcon(NIM_ADD, &NID);
	
	
	CRect rect(0,0,0,0);
	
	/*	m_wndHSplitterBar.Create(WS_CHILD|WS_BORDER|WS_DLGFRAME|WS_VISIBLE,rect,this,999,TRUE);
	m_wndHSplitterBar.SetPanes(&m_tabs,&m_tasks);
	*/
	// resize
	CRect formRect; 
	GetWindowRect(&formRect); 
	
	SetWindowPos(0,0,0 ,MAX_W,MAX_H,0);
	CenterWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

int  CCFToolboxDlg::init()
{
	initDebug("CFToolbox.log", 0);
	
	initServerRegistry();
	
	// should update CDR ? 
	ConfigManager conf;	
	setGlobalLogLevel(conf.getDebugLevel());
	if (conf.isAutomaticCDRUpdate())
	{
		SteamNetwork network;
		network.updateContentDescriptionRecord();
	}
	
	// check if folder exists
	while(!folderExists(conf.getSteamAppsPath()))
	{
		CString folder;
		if (BrowseForFolder(NULL,"GCF / NCF storage path",folder))
		{
			conf.setSteamAppsPath(folder.GetBuffer(0));
		}
		else
		{
			canceledStart=true;
			SendMessage(WM_CLOSE, 0, 0 );
			return 0;
		}
	}
	
	// try to load icons from winui.gcf
	char winuiPath[1000];
	sprintf(winuiPath,"%s\\winui.gcf",conf.getSteamAppsPath());
	GCF * gcf=parseGCF(winuiPath);
	initAppIcons(gcf);
	if ((int)gcf>1)
		deleteGCF(gcf);
	m_tabs.InsertItem(0, _T("GCF/NCF Files"));
	m_tabs.InsertItem(1, _T("Games/Media/Tools"));
	m_tabs.InsertItem(2, _T("Install"));
	m_tabs.InsertItem(3, _T("Bandwidth monitor"));
	m_tabs.InsertItem(4, _T("Preferences"));
	m_tabs.InsertItem(5, _T("Logs"));
	m_tabs.InsertItem(6, _T("Help"));
 
	m_tabs.Init();
	 
 
	CTabPreferences * tabPreferences=getTabPreferences();
	tabPreferences->parent=this;
	
	CTabLogs* tabLogs=(CTabLogs*)m_tabs.m_tabPages[5];
	
	setGlobalLogLevel(conf.getDebugLevel());
	closeDebug();
	initDebug("CFToolbox.log", &tabLogs->m_logsCtrl);	
	
	initCFManager();
	getTaskManager()->initTasksDisplay(&m_tasks,this);
 
	getPlugins(); // init plugins	
 
/*
	CAccounts accs;
	accs.resetAllTickets();
*/
	return 1;
	
}

void CCFToolboxDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCFToolboxDlg::OnPaint() 
{
	
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		
		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		
		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon16);
	}
	else
	{
		CDialog::OnPaint();
		
	}
	if (!initialized )
	{
		initialized=true;
		if (!init())
		{
			SendMessage(WM_CLOSE, 0, 0 );
		}
	} 
	
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCFToolboxDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon16;
}
BOOL CCFToolboxDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
			pMsg->wParam=NULL ;
	} 
	
	return CDialog::PreTranslateMessage(pMsg);
}


int CALLBACK tasksCompareFunc(LPARAM id1, LPARAM id2 , 
							  LPARAM lParamSort)
{  
	int iSubItem = (int)lParamSort;
	int a=0;
	
	char * processing="Processing";
	
	CTask * task1=(CTask *)id1;
	CTask * task2=(CTask *)id2;
	// possible crask if task is dead ...
	switch (iSubItem)
	{
	case 0:{
		a=strcmp(task1->taskFile,task2->taskFile);
		break;
		   }
	case 1:{
		char tskName1[300];
		char tskName2[300];
		if (task1->waiting)
			sprintf(tskName1,"Waiting (%s)",(task1->taskName ?  task1->taskName : processing));
		else
			strcpy(tskName1,(task1->taskName ?  task1->taskName : processing));
		if (task2->waiting)
			sprintf(tskName2,"Waiting (%s)",(task2->taskName ?  task2->taskName : processing));
		else
			strcpy(tskName2,(task2->taskName ?  task2->taskName : processing));
		a=strcmp(tskName1,tskName2);
		break;
		   }
	case 2:{
		if (task1->taskProgress==task2->taskProgress)
			a=0;
		else
			a=(task1->taskProgress<task2->taskProgress? -1 : 1);
		break;
		   }
		
	}
	if (iSubItem && !a) a=stricmp(task1->taskFile,task2->taskFile);
	return  a;
}

void CCFToolboxDlg::OnColumnclickTasks(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	int nCol = pNMListView->iSubItem;
	
	
    m_tasks.SortItems(tasksCompareFunc, nCol);
	
	POSITION pos = m_tasks.GetFirstSelectedItemPosition();
	if (pos)
	{
		int index = m_tasks.GetNextSelectedItem(pos);
		m_tasks.EnsureVisible(index,FALSE);
	}	
	*pResult = 0;
}

void CCFToolboxDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	int tasksHeight=130;
	int accountsHeight=18;
#ifndef	_DEBUG
	//if (IsWindow(wndTop))
	{
		m_tabs.SetWindowPos( &wndTop, 0, 0, cx,  cy-tasksHeight-accountsHeight, 0 );
		m_tasks.SetWindowPos( &wndTop, 0, cy-tasksHeight-accountsHeight, cx,  tasksHeight, 0 );
		m_accountsCtrl.SetWindowPos(&wndTop, 0, cy-accountsHeight, cx,  accountsHeight, 0 );
	}
#endif
	NOTIFYICONDATA NID;
	NID.cbSize = sizeof(NID); // Size of the structure duh :)
	NID.hIcon = AfxGetApp()->LoadIcon(IDR_ICON16);//LoadIcon(hInstance, IDR_MAINFRAME); // The icon that will be shown in the systray.
	NID.uCallbackMessage = UWM_SYSTRAY; // Our user defined message.(For notifications, Windows will send us messages when user pressed on our icon)
	NID.hWnd = GetSafeHwnd(); // Handle to the window we would like to get the notifications.
	NID.uID = 1; // Icon ID in systray...
	strcpy(NID.szTip, "CF Toolbox"); // The tooltip string
	NID.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE; // Here we say we want to get notifications about the icon, we have an icon and a tooltip.
	
	if (nType==SIZE_MINIMIZED) 
	{
		
		ShowWindow(SW_HIDE);
	}
	else
	{
		SetForegroundWindow();
	}
}

void CCFToolboxDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	
	// the minimum client rectangle (in that is lying the view window)
	CRect rc(0, 0, MAX_W-8, MAX_H-26);
	// compute the required size of the frame window rectangle
	// based on the desired client-rectangle size
	CalcWindowRect(rc);
	
	lpMMI->ptMinTrackSize.x = rc.Width();
	lpMMI->ptMinTrackSize.y = rc.Height();
}


LRESULT CCFToolboxDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if ((message == UWM_SYSTRAY) && (lParam==WM_LBUTTONUP || lParam==WM_RBUTTONUP))
		ShowWindow(SW_RESTORE);	
	
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CCFToolboxDlg::OnClose() 
{
	if (!canceledStart)
	{
	if (getTaskManager()->nbTotalTasks)
	{
		MessageBox( "Please stop all running operations before closing CF Toolbox","CF Toolbox",MB_ICONINFORMATION|MB_OK);
		return;
	}
	}
	NOTIFYICONDATA NID;
	NID.cbSize = sizeof(NID); // Size of the structure duh :)
	NID.hIcon = AfxGetApp()->LoadIcon(IDR_ICON16);//LoadIcon(hInstance, IDR_MAINFRAME); // The icon that will be shown in the systray.
	NID.uCallbackMessage = UWM_SYSTRAY; // Our user defined message.(For notifications, Windows will send us messages when user pressed on our icon)
	NID.hWnd = GetSafeHwnd(); // Handle to the window we would like to get the notifications.
	NID.uID = 1; // Icon ID in systray...
	strcpy(NID.szTip, "CF Toolbox"); // The tooltip string
	NID.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE; // Here we say we want to get notifications about the icon, we have an icon and a tooltip.
	
	
	Shell_NotifyIcon(NIM_DELETE, &NID);
	
	
	
	//	Sleep(500); // task has time to finish => no crash ... dirty .. 
	//	getTaskManager()->killAllTasks(); 
	
	//	getTaskManager()->stopAllTasks();
	if (!canceledStart)
	{
		delete getTaskManager(); 
		delete getCFManager();
		Sleep(1000);
	/*
		CAccounts accs;
		accs.resetAllTickets();
	*/
	}
	WSACleanup();
	closeDebug();
	
	CDialog::OnClose();
}

void CCFToolboxDlg::OnContextualMenu(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
 	if (!m_tasks.GetSelectedCount()) return;
	
 
	CPoint point;
    GetCursorPos(&point);
	
    CMenu mnuTop;
    mnuTop.LoadMenu(IDR_TASKS_MENU);
	
    CMenu* pPopup = mnuTop.GetSubMenu(0);
    ASSERT_VALID(pPopup);
	
    int command=pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN| TPM_RETURNCMD, point.x, point.y, AfxGetMainWnd(), NULL);
	
	if (!command) return;
 	CCFManager * manager=getCFManager();
	//	DWORD cfId=m_filesCtrl.GetItemData(m_filesCtrl.GetSelectionMark()); // for single selection
 	POSITION pos = m_tasks.GetFirstSelectedItemPosition();
 
 	while(pos)
	{
	
		int index = m_tasks.GetNextSelectedItem(pos);
//		DWORD cfId=m_tasks.GetItemData(index);
		CTask * task=(CTask*)m_tasks.GetItemData(index); 
		DWORD cfId=task->cfId;
		CCFDescriptor * descriptor = (CCFDescriptor*) manager->files->get(cfId);
		if (descriptor) 
		{
			switch (command)
			{
			case (ID_ROOT_CANCEL):
				{
					manager->cancelTask(cfId);
					break;
				}
			}
		}
 	
	}
 
}

void CCFToolboxDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	//	printDebug(DEBUG_LEVEL_TODO,"test","timer executed",0);
	KillTimer(nIDEvent);
	OnUpdateAccounts();
}

void CCFToolboxDlg::OnUpdateAccounts()
{
	UpdateData(TRUE);
	char acc[2000];
	const char * accounts=::getConnectedAccountsList();
	if (accounts && strlen(accounts)) sprintf(acc,"Connected accounts : %s",accounts);
	else strcpy(acc,"No connected account");
	m_accounts=acc;
	UpdateData(FALSE);
}