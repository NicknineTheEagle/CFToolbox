// PropertiesPopup.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "PropertiesPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertiesPopup dialog


CPropertiesPopup::CPropertiesPopup(CWnd* pParent /*=NULL*/)
	: CDialog(CPropertiesPopup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPropertiesPopup)
	m_blocks = _T("");
	m_commonPath = _T("");
	m_completion = _T("");
	m_files = _T("");
	m_folders = _T("");
	m_id = _T("");
	m_key = _T("");
	m_path = _T("");
	m_size = _T("");
	m_type = _T("");
	m_version = _T("");
	m_description = _T("");
	//}}AFX_DATA_INIT
}


void CPropertiesPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertiesPopup)
	DDX_Text(pDX, IDC_BLOCKS, m_blocks);
	DDX_Text(pDX, IDC_COMMONPATH, m_commonPath);
	DDX_Text(pDX, IDC_COMPLETION, m_completion);
	DDX_Text(pDX, IDC_FILES, m_files);
	DDX_Text(pDX, IDC_FOLDERD, m_folders);
	DDX_Text(pDX, IDC_ID, m_id);
	DDX_Text(pDX, IDC_KEY, m_key);
	DDX_Text(pDX, IDC_PATH, m_path);
	DDX_Text(pDX, IDC_CSIZE, m_size);
	DDX_Text(pDX, IDC_TYPE, m_type);
	DDX_Text(pDX, IDC_VERSION, m_version);
	DDX_Text(pDX, IDC_DESCRIPTION, m_description);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertiesPopup, CDialog)
	//{{AFX_MSG_MAP(CPropertiesPopup)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertiesPopup message handlers
