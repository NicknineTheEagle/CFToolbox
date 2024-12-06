// CFToolbox.h : main header file for the CFTOOLBOX application
//

#if !defined(AFX_CFTOOLBOX_H__E97AE3B3_B673_4F1F_A7FB_65E42619402D__INCLUDED_)
#define AFX_CFTOOLBOX_H__E97AE3B3_B673_4F1F_A7FB_65E42619402D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCFToolboxApp:
// See CFToolbox.cpp for the implementation of this class
//

class CCFToolboxApp : public CWinApp
{
public:
	CCFToolboxApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCFToolboxApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCFToolboxApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CFTOOLBOX_H__E97AE3B3_B673_4F1F_A7FB_65E42619402D__INCLUDED_)
