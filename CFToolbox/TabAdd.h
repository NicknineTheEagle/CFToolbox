#if !defined(AFX_TABADD_H__254BEA28_BA2B_436F_AABD_4CED5AD31574__INCLUDED_)
#define AFX_TABADD_H__254BEA28_BA2B_436F_AABD_4CED5AD31574__INCLUDED_

#include "HashMap.h"
#include "cdr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabAdd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabAdd dialog

class CTabAdd : public CDialog
{
// Construction
public:
	CTabAdd(CWnd* pParent = NULL);   // standard constructor
	BOOL PreTranslateMessage(MSG* pMsg) ;
	void refreshAccounts();
	void refreshTab();
	void fillListValues(const CDRApp& app, char** values);
	HashMapDword * notDownloadedApps;
	HashMapDword * selectedFiles;
	HashMapDword * selectedVersions;
 void OnAppClicked( );
// Dialog Data
	//{{AFX_DATA(CTabAdd)
	enum { IDD = IDD_TAB_ADD };
	CButton	m_appsFrame;
	CButton	m_filesFrame;
	CStatic	m_accLabel;
	CComboBox	m_accountsCtrl;
	CListCtrl	m_filesCtrl;
	CListCtrl	m_appsCtrl;
	CButton	m_downloadCtrl;
	CString	m_account;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabAdd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabAdd)
	afx_msg void OnDownload();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnclickApps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedApps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnAppContextualMenu(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileContextualMenu(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
CTabAdd * getTabAdd();
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABADD_H__254BEA28_BA2B_436F_AABD_4CED5AD31574__INCLUDED_)
