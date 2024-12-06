#if !defined(AFX_PROPERTIESPOPUP_H__DA828AF0_8F2E_4405_B07E_94064174B66C__INCLUDED_)
#define AFX_PROPERTIESPOPUP_H__DA828AF0_8F2E_4405_B07E_94064174B66C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertiesPopup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropertiesPopup dialog

class CPropertiesPopup : public CDialog
{
// Construction
public:
	CPropertiesPopup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPropertiesPopup)
	enum { IDD = IDD_PROPERTIES };
	CString	m_blocks;
	CString	m_commonPath;
	CString	m_completion;
	CString	m_files;
	CString	m_folders;
	CString	m_id;
	CString	m_key;
	CString	m_path;
	CString	m_size;
	CString	m_type;
	CString	m_version;
	CString	m_description;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertiesPopup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPropertiesPopup)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTIESPOPUP_H__DA828AF0_8F2E_4405_B07E_94064174B66C__INCLUDED_)
