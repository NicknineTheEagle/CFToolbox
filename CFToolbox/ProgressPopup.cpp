// ProgressPopup.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "ProgressPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressPopup dialog


CProgressPopup::CProgressPopup(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressPopup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressPopup)
	m_message = _T("");
	//}}AFX_DATA_INIT
}


void CProgressPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressPopup)
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Text(pDX, IDC_MESSAGE, m_message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressPopup, CDialog)
	//{{AFX_MSG_MAP(CProgressPopup)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressPopup message handlers
BOOL CProgressPopup::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
			pMsg->wParam=NULL ;
	} 
	
	return CDialog::PreTranslateMessage(pMsg);
}