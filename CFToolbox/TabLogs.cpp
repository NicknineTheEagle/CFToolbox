// TabLogs.cpp : implementation file
//
#include <direct.h>

#include "stdafx.h"
 
#include "TabLogs.h"
 
#include "debug.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

CTabLogs::CTabLogs(CWnd* pParent /*=NULL*/)
	: CDialog(CTabLogs::IDD, pParent)
{
 	//{{AFX_DATA_INIT(CTabLogs)
 
	//}}AFX_DATA_INIT
}


void CTabLogs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabLogs)
 	DDX_Control(pDX, IDC_LOGS, m_logsCtrl);
 

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabLogs, CDialog)
	//{{AFX_MSG_MAP(CTabLogs)
 
 
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabLogs message handlers
 

 BOOL CTabLogs::PreTranslateMessage(MSG* pMsg) 
     {
          // TODO: Add your specialized code here and/or call the base class
          if(pMsg->message==WM_KEYDOWN)
          {
              if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
                  pMsg->wParam=NULL ;
          } 

          return CDialog::PreTranslateMessage(pMsg);
       }

 
 

BOOL CTabLogs::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	LOGFONT lfLogFont;
	
	memset(&lfLogFont, 0, sizeof(lfLogFont));
	
	strcpy(lfLogFont.lfFaceName, "Courier New");         
	lfLogFont.lfOutPrecision=99;
	lfLogFont.lfQuality=99;
	lfLogFont.lfHeight    = 13;                     
	//lfLogFont.lfWeight    = FW_BOLD;               
	//lfLogFont.lfUnderline = TRUE;                  
	
	m_Font.CreateFontIndirect(&lfLogFont);
	 m_logsCtrl.SetFont(&m_Font);
	// end change font

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabLogs::OnClear() 
{
	// TODO: Add your control notification handler code here
	CString clear="";
	m_logsCtrl.SetWindowText(clear); 
}

void CTabLogs::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	//if (IsWindow(wndTop))
#ifndef _DEBUG
	m_logsCtrl.SetWindowPos( &wndTop, 5, 30, cx-10,  cy-35, 0 );	 
#endif	 
}
