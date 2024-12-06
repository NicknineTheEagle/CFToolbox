#if !defined(AFX_TABAPPS_H__62C20127_C4F5_435C_8139_6322A9B76A42__INCLUDED_)
#define AFX_TABAPPS_H__62C20127_C4F5_435C_8139_6322A9B76A42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabApps.h : header file
//
#include "AppPropertiesPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CTabApps dialog

class CTabApps : public CDialog
{

// Construction
public:
	CTabApps(CWnd* pParent = NULL);   // standard constructor
	CAppPropertiesPopup * properties;  // TODO : make a app properties popup
	BOOL PreTranslateMessage(MSG* pMsg);
	void refreshContent();
// Dialog Data
	//{{AFX_DATA(CTabApps)
	enum { IDD = IDD_TAB_APPS };
	CListCtrl	m_appsCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabApps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabApps)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnContextualMenu(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickApps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkApps(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
CTabApps * getTabApps();
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABAPPS_H__62C20127_C4F5_435C_8139_6322A9B76A42__INCLUDED_)
