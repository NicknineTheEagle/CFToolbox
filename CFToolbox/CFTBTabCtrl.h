#if !defined(AFX_CONFIGTABCTRL_H__5E3FFC52_33B3_4008_B360_40D222E54E63__INCLUDED_)
#define AFX_CONFIGTABCTRL_H__5E3FFC52_33B3_4008_B360_40D222E54E63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VssTabCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VssTabCtrl window

class CFTBTabCtrl : public CTabCtrl
{
// Construction
public:
	CFTBTabCtrl();
	CDialog *m_tabPages[8];
	int m_tabCurrent;
	int m_nNumberOfPages;

// Attributes
public:

// Operations
public:
	void Init();
	void SetRectangle();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFTBTabCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetWindowPos( const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags );
	virtual ~CFTBTabCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(VssTabCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGTABCTRL_H__5E3FFC52_33B3_4008_B360_40D222E54E63__INCLUDED_)
