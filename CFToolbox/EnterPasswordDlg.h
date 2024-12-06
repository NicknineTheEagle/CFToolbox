#if !defined(AFX_ENTERPASSWORDDLG_H__52390708_4AD0_4B50_A8C5_F559BF47FE54__INCLUDED_)
#define AFX_ENTERPASSWORDDLG_H__52390708_4AD0_4B50_A8C5_F559BF47FE54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnterPasswordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEnterPasswordDlg dialog

class CEnterPasswordDlg : public CDialog
{
// Construction
public:
	CEnterPasswordDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEnterPasswordDlg)
	enum { IDD = IDD_ENTER_PASSWORD };
	CString	m_account;
	CString	m_password;
	BOOL	m_savePassword;
	CString	m_steamguard;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEnterPasswordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEnterPasswordDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTERPASSWORDDLG_H__52390708_4AD0_4B50_A8C5_F559BF47FE54__INCLUDED_)
