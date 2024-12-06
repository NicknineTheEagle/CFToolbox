//{{AFX_INCLUDES()
#include "webbrowser2.h"
//}}AFX_INCLUDES
#if !defined(AFX_TABHELP_H__09BD25CE_6178_455F_BBCC_CEB2874D34C2__INCLUDED_)
#define AFX_TABHELP_H__09BD25CE_6178_455F_BBCC_CEB2874D34C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabHelp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabHelp dialog

class CTabHelp : public CDialog
{
// Construction
public:
	CTabHelp(CWnd* pParent = NULL);   // standard constructor
	BOOL PreTranslateMessage(MSG* pMsg) ;	
// Dialog Data
	//{{AFX_DATA(CTabHelp)
	enum { IDD = IDD_TAB_HELP };
	CStatic	m_notfound;
	CWebBrowser2	m_explorer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabHelp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool noHelp;
	// Generated message map functions
	//{{AFX_MSG(CTabHelp)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABHELP_H__09BD25CE_6178_455F_BBCC_CEB2874D34C2__INCLUDED_)
