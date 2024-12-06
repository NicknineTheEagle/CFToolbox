// EnterPasswordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "EnterPasswordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnterPasswordDlg dialog


CEnterPasswordDlg::CEnterPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEnterPasswordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnterPasswordDlg)
	m_account = _T("");
	m_password = _T("");
	m_savePassword = FALSE;
	m_steamguard = _T("");
	//}}AFX_DATA_INIT
}


void CEnterPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnterPasswordDlg)
	DDX_Text(pDX, IDC_ACCOUNT, m_account);
	DDX_Text(pDX, IDC_PASSWORD, m_password);
	DDX_Check(pDX, IDC_SAVE_PASSWORD, m_savePassword);
	DDX_Text(pDX, IDC_STEAMGUARD2, m_steamguard);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnterPasswordDlg, CDialog)
	//{{AFX_MSG_MAP(CEnterPasswordDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnterPasswordDlg message handlers
