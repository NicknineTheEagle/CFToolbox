// CFToolboxDlg.h : header file
//

#if !defined(AFX_CFTOOLBOXDLG_H__05D24D1D_7101_476B_9546_1A88BE09FE5F__INCLUDED_)
#define AFX_CFTOOLBOXDLG_H__05D24D1D_7101_476B_9546_1A88BE09FE5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CFTBTabCtrl.h"
#include "SplitterBar.h"
/////////////////////////////////////////////////////////////////////////////
// CCFToolboxDlg dialog

class CCFToolboxDlg : public CDialog
{
// Construction
public:
	CCFToolboxDlg(CWnd* pParent = NULL);	// standard constructor
	BOOL  PreTranslateMessage(MSG* pMsg) ;
	 void  OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	 int init();
	 int canceledStart;
	void OnUpdateAccounts();
	 // Dialog Data
	//{{AFX_DATA(CCFToolboxDlg)
	enum { IDD = IDD_CFTOOLBOX_DIALOG };
	CEdit	m_accountsCtrl;
	CListCtrl	m_tasks;
	CFTBTabCtrl	m_tabs;
	CString	m_accounts;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCFToolboxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
LRESULT  WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
// Implementation
//protected:
//		CSplitterBar m_wndHSplitterBar;
	HICON m_hIcon16;
	HICON m_hIcon32;
	// Generated message map functions
	//{{AFX_MSG(CCFToolboxDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnColumnclickTasks(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnContextualMenu(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CWnd  * getMainTab();
CDialog  * getMainDialog();
void setRunningOutOfSpace(bool outOfSpace);
bool isRunningOutOfSpace();
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_CFTOOLBOXDLG_H__05D24D1D_7101_476B_9546_1A88BE09FE5F__INCLUDED_)
