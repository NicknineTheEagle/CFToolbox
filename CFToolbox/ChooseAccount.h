#if !defined(AFX_CHOOSEACCOUNT_H__977D2142_8CC9_4ECC_AC4C_4DD31B4C17AC__INCLUDED_)
#define AFX_CHOOSEACCOUNT_H__977D2142_8CC9_4ECC_AC4C_4DD31B4C17AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseAccount.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseAccount dialog

class CChooseAccount : public CDialog
{
// Construction
public:
	CChooseAccount(CWnd* pParent = NULL);   // standard constructor
	CString m_selectedAccount;
// Dialog Data
	//{{AFX_DATA(CChooseAccount)
	enum { IDD = IDD_CHOOSE_ACCOUNT };
	CButton	m_rememberCtrl;
	CComboBox	m_accounts;
	CString	m_account;
	CString	m_file;
	BOOL	m_remember;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseAccount)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseAccount)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEACCOUNT_H__977D2142_8CC9_4ECC_AC4C_4DD31B4C17AC__INCLUDED_)
