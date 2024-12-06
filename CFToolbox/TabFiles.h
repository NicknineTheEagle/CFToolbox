#if !defined(AFX_TABFILES_H__551E954A_394F_46AC_89C0_1120FAC49B6A__INCLUDED_)
#define AFX_TABFILES_H__551E954A_394F_46AC_89C0_1120FAC49B6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabFiles.h : header file
//
#include "PropertiesPopup.h"
#include "CFDescriptor.h"

/////////////////////////////////////////////////////////////////////////////
// CTabFiles dialog

class CTabFiles : public CDialog
{
// Construction
public:
	CTabFiles(CWnd* pParent = NULL);   // standard constructor
	CPropertiesPopup * properties;
	BOOL PreTranslateMessage(MSG* pMsg);
	void insertFile(CCFDescriptor * descriptor,boolean noAutoUpdate=false);
	void removeFile(DWORD fileId);
	void refreshContent();
// Dialog Data
	//{{AFX_DATA(CTabFiles)
	enum { IDD = IDD_TAB_FILES };
	CListCtrl	m_filesCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabFiles)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabFiles)
	virtual BOOL OnInitDialog();
	afx_msg void OnContextualMenu(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
CTabFiles * getTabFiles();
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABFILES_H__551E954A_394F_46AC_89C0_1120FAC49B6A__INCLUDED_)
