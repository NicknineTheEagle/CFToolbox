// TabHelp.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "TabHelp.h"
#include "direct.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabHelp dialog


CTabHelp::CTabHelp(CWnd* pParent /*=NULL*/)
	: CDialog(CTabHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabHelp)
	//}}AFX_DATA_INIT
}


void CTabHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabHelp)
	DDX_Control(pDX, IDC_NOTFOUND, m_notfound);
	DDX_Control(pDX, IDC_EXPLORER1, m_explorer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabHelp, CDialog)
	//{{AFX_MSG_MAP(CTabHelp)
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabHelp message handlers

BOOL CTabHelp::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	noHelp=false;
	FILE * f = fopen("help\\index.html","r");
	if (f)
	{
		fclose(f);
		SetTimer(0,100,NULL);
		m_notfound.SetWindowPos(&wndTop, 0,0,0,0,0 );
	}
	else
	{
		noHelp=true;
	 	m_explorer.SetWindowPos(&wndTop, 0,0,0,0,0 );
		m_explorer.SetVisible(false);
		m_explorer.EnableWindow(false);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabHelp::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	#ifndef _DEBUG
	if (!noHelp) m_explorer.SetWindowPos( &wndTop, 5, 5, cx-10,  cy-10, 0 );	 
#endif	 
}

 BOOL CTabHelp::PreTranslateMessage(MSG* pMsg) 
     {
          // TODO: Add your specialized code here and/or call the base class
          if(pMsg->message==WM_KEYDOWN)
          {
              if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
                  pMsg->wParam=NULL ;
          } 

          return CDialog::PreTranslateMessage(pMsg);
       }

 

void CTabHelp::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(nIDEvent);
	char cwd[1001];
	_getcwd(cwd,1000);
	char path[1000];
	sprintf(path,"file:///%s/help/index.html",cwd);
	m_explorer.Navigate(path,NULL,NULL,NULL,NULL);
	 
}
