#if !defined(AFX_PROGRESSPOPUP_H__6AFC5FC3_08B8_44A4_BD1C_EFCFCE721F6C__INCLUDED_)
#define AFX_PROGRESSPOPUP_H__6AFC5FC3_08B8_44A4_BD1C_EFCFCE721F6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressPopup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressPopup dialog

class CProgressPopup : public CDialog
{
// Construction
public:
	CProgressPopup(CWnd* pParent = NULL);   // standard constructor
	BOOL  PreTranslateMessage(MSG* pMsg); 
// Dialog Data
	//{{AFX_DATA(CProgressPopup)
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl	m_progress;
	CString	m_message;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressPopup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProgressPopup)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSPOPUP_H__6AFC5FC3_08B8_44A4_BD1C_EFCFCE721F6C__INCLUDED_)
