#if !defined(AFX_TABLOGS_H__AA990E0E_9886_4A80_9841_CA102E160BAC__INCLUDED_)
#define AFX_TABLOGS_H__AA990E0E_9886_4A80_9841_CA102E160BAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabLogs.h : header file
//
#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

class CTabLogs : public CDialog
{
// Construction
public:
	CTabLogs(CWnd* pParent = NULL);   // standard constructor
	BOOL PreTranslateMessage(MSG* pMsg) ;	
	LRESULT OnDataNotify( WPARAM /* wParam */, LPARAM /* lParam */ );

// Dialog Data
	//{{AFX_DATA(CTabLogs)
	enum { IDD = IDD_TAB_LOGS };
	CEdit	m_logsCtrl;
 

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabLogs)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFont m_Font;
	// Generated message map functions
	//{{AFX_MSG(CTabLogs)
 
	virtual BOOL OnInitDialog();
	afx_msg void OnClear();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLOGS_H__AA990E0E_9886_4A80_9841_CA102E160BAC__INCLUDED_)
