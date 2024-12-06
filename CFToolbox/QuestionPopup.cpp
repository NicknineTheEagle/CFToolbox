// QuestionPopup.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "QuestionPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuestionPopup dialog


CQuestionPopup::CQuestionPopup(CWnd* pParent /*=NULL*/)
	: CDialog(CQuestionPopup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuestionPopup)
	m_value = _T("");
	m_question = _T("");
	//}}AFX_DATA_INIT
}


void CQuestionPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuestionPopup)
	DDX_Text(pDX, IDC_VALUE, m_value);
	DDX_Text(pDX, IDC_Question, m_question);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuestionPopup, CDialog)
	//{{AFX_MSG_MAP(CQuestionPopup)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuestionPopup message handlers
