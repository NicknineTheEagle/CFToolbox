#if !defined(AFX_QUESTIONPOPUP_H__0611C427_4CB2_49F9_96FA_8CFCF0FEC6AF__INCLUDED_)
#define AFX_QUESTIONPOPUP_H__0611C427_4CB2_49F9_96FA_8CFCF0FEC6AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuestionPopup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQuestionPopup dialog

class CQuestionPopup : public CDialog
{
// Construction
public:
	CQuestionPopup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQuestionPopup)
	enum { IDD = IDD_QUESTION };
	CString	m_value;
	CString	m_question;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuestionPopup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQuestionPopup)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUESTIONPOPUP_H__0611C427_4CB2_49F9_96FA_8CFCF0FEC6AF__INCLUDED_)
