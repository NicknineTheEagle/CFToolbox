// ChooseAccount.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "ChooseAccount.h"
#include "Accounts.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseAccount dialog


CChooseAccount::CChooseAccount(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseAccount::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseAccount)
	m_account = _T("");
	m_file = _T("");
	m_remember = FALSE;
	//}}AFX_DATA_INIT
	m_selectedAccount=_T("");
}


void CChooseAccount::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseAccount)
	DDX_Control(pDX, IDC_REMEMBER, m_rememberCtrl);
	DDX_Control(pDX, IDC_ACCOUNT_LIST, m_accounts);
	DDX_CBString(pDX, IDC_ACCOUNT_LIST, m_account);
	DDX_Text(pDX, IDC_FILE, m_file);
	DDX_Check(pDX, IDC_REMEMBER, m_remember);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseAccount, CDialog)
	//{{AFX_MSG_MAP(CChooseAccount)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseAccount message handlers

BOOL CChooseAccount::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CAccounts accs;
	int nb=accs.getNbAccounts();
	for (int ind=0;ind<nb;ind++)
	{
		char * acc=accs.getAccountIdAt(ind);
		m_accounts.AddString(acc);
	}
	if (!m_selectedAccount.IsEmpty() && m_selectedAccount.GetLength())
	{
		m_accounts.SetCurSel(m_accounts.FindString(0,m_selectedAccount));
	}
		else
	m_accounts.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
