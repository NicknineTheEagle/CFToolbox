//{{AFX_INCLUDES()
#include "webbrowser2.h"
//}}AFX_INCLUDES
#if !defined(AFX_APPPROPERTIESPOPUP_H__7CDFC661_48A7_424A_91E4_5F0234E8084E__INCLUDED_)
#define AFX_APPPROPERTIESPOPUP_H__7CDFC661_48A7_424A_91E4_5F0234E8084E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AppPropertiesPopup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAppPropertiesPopup dialog

class CAppPropertiesPopup : public CDialog
{
// Construction
public:
	CAppPropertiesPopup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAppPropertiesPopup)
	enum { IDD = IDD_APP_PROPERTIES };
	CListCtrl	m_properties;
	CListCtrl	m_cfs;
	CString	m_size;
	CString	m_developper;
	CString	m_common;
	CString	m_id;
	CString	m_manual;
	CString	m_name;
	CWebBrowser2	m_browser;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppPropertiesPopup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAppPropertiesPopup)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPPROPERTIESPOPUP_H__7CDFC661_48A7_424A_91E4_5F0234E8084E__INCLUDED_)
