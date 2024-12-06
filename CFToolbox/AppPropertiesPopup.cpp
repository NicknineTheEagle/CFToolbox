// AppPropertiesPopup.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "AppPropertiesPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAppPropertiesPopup dialog

void buildListColumns(CListCtrl* list, char ** names,int * width,int nbCols);
int insertListValue(CListCtrl* list, int id, char ** values,int nbCols,int iconId);
int updateListValue(CListCtrl* list, int index, char ** values,int nbCols);

CAppPropertiesPopup::CAppPropertiesPopup(CWnd* pParent /*=NULL*/)
	: CDialog(CAppPropertiesPopup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppPropertiesPopup)
	m_size = _T("");
	m_developper = _T("");
	m_common = _T("");
	m_id = _T("");
	m_manual = _T("");
	m_name = _T("");
	//}}AFX_DATA_INIT
}


void CAppPropertiesPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppPropertiesPopup)
	DDX_Control(pDX, IDC_PROPERTIES, m_properties);
	DDX_Control(pDX, IDC_GCFS, m_cfs);
	DDX_Text(pDX, IDC_CSIZE, m_size);
	DDX_Text(pDX, IDC_DEVELOPPER, m_developper);
	DDX_Text(pDX, IDC_COMMON, m_common);
	DDX_Text(pDX, IDC_ID, m_id);
	DDX_Text(pDX, IDC_MANUAL, m_manual);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_EXPLORER1, m_browser);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAppPropertiesPopup, CDialog)
	//{{AFX_MSG_MAP(CAppPropertiesPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppPropertiesPopup message handlers

BOOL CAppPropertiesPopup::OnInitDialog() 
{
	CDialog::OnInitDialog();
		
	char * cfnames[4]={"File","Size","Version","Completion" };
	int cfwidths[4]={150,90,60,90};
	buildListColumns(&m_cfs,cfnames,cfwidths,4);
	m_cfs.SetExtendedStyle(m_cfs.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
 
	char * pnames[2]={"Property","Value" };
	int pwidths[2]={140,250};
	buildListColumns(&m_properties,pnames,pwidths,2);
	m_properties.SetExtendedStyle(m_properties.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
