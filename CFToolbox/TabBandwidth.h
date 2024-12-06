//{{AFX_INCLUDES()
#include "webbrowser2.h"
//}}AFX_INCLUDES
#if !defined(AFX_TABBANDWIDTH_H__2457957D_57B8_47E0_8989_7EAB4EEE2BA3__INCLUDED_)
#define AFX_TABBANDWIDTH_H__2457957D_57B8_47E0_8989_7EAB4EEE2BA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabBandwidth.h : header file
//
#include "HistogramCtrl.h"
#include "webbrowser2.h"

#include "Task.h"

/////////////////////////////////////////////////////////////////////////////
// CTabBandwidth dialog

class CTabBandwidth : public CDialog
{
// Construction
public:
	CTabBandwidth(CWnd* pParent = NULL);   // standard constructor
BOOL  PreTranslateMessage(MSG* pMsg) ;
void addDownloader(char * name, CTask * task);
void removeDownloader(char * name, CTask * task);
void setCSBannerVisibility(bool show);
CHistogramCtrl  * m_bandwidthGraph;
// CWebBrowser2 * m_pBrowser ;
// Dialog Data
	//{{AFX_DATA(CTabBandwidth)
	enum { IDD = IDD_TAB_BANDWIDTH };
	CStatic	m_hider;
	CButton	m_from;
	CComboBox	m_downloads;
	CString	m_current;
	CString	m_min;
	CWebBrowser2	m_explorer;
	CString	m_max;
	CString	m_remaining;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabBandwidth)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
 int explorerW;
 int explorerH;
	// Generated message map functions
	//{{AFX_MSG(CTabBandwidth)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeDownloader();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CTabBandwidth * getTabBandwidth();

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABBANDWIDTH_H__2457957D_57B8_47E0_8989_7EAB4EEE2BA3__INCLUDED_)
