// VssTabCtrl.cpp : implementation file
//
#include "resource.h"
#include "stdafx.h"
  
#include "CFTBTabCtrl.h"
#include "TabFiles.h"
#include "TabApps.h"
#include "TabAdd.h"
#include "TabBandwidth.h"
#include "TabPreferences.h"
#include "TabLogs.h"
#include "TabHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFTBTabCtrl

CFTBTabCtrl::CFTBTabCtrl()
{
  m_tabCurrent=0;
  m_tabPages[0]=new CTabFiles;
  m_tabPages[1]=new CTabApps;
  m_tabPages[2]=new CTabAdd;
  m_tabPages[3]=new CTabBandwidth;
  m_tabPages[4]=new CTabPreferences;
  m_tabPages[5]=new CTabLogs;
  m_tabPages[6]=new CTabHelp;
 m_nNumberOfPages=7; 
}

CFTBTabCtrl::~CFTBTabCtrl()
{
 for(int nCount=0; nCount < m_nNumberOfPages; nCount++){
  delete m_tabPages[nCount];
 }
}


BEGIN_MESSAGE_MAP(CFTBTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CConfigTabCtrl)
		ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
 

/////////////////////////////////////////////////////////////////////////////
// VssTabCtrl message handlers

 

void CFTBTabCtrl::Init()
{
 m_tabCurrent=0;

 m_tabPages[0]->Create(IDD_TAB_FILES, this);
 m_tabPages[1]->Create(IDD_TAB_APPS, this);
 m_tabPages[2]->Create(IDD_TAB_ADD, this);
 m_tabPages[3]->Create(IDD_TAB_BANDWIDTH, this);
 m_tabPages[4]->Create(IDD_TAB_PREFERENCES, this);
 m_tabPages[5]->Create(IDD_TAB_LOGS, this);
 m_tabPages[6]->Create(IDD_TAB_HELP, this);
 
 m_tabPages[0]->ShowWindow(SW_SHOW);
 m_tabPages[1]->ShowWindow(SW_HIDE);
 m_tabPages[2]->ShowWindow(SW_HIDE);
 m_tabPages[3]->ShowWindow(SW_HIDE);
 m_tabPages[4]->ShowWindow(SW_HIDE);
 m_tabPages[5]->ShowWindow(SW_HIDE);
 m_tabPages[6]->ShowWindow(SW_HIDE);

 SetRectangle();
}

void CFTBTabCtrl::SetRectangle()
{
 CRect tabRect, itemRect;
 int nX, nY, nXc, nYc;

 GetClientRect(&tabRect);
 GetItemRect(0, &itemRect);

 nX=itemRect.left;
 nY=itemRect.bottom+1;
 nXc=tabRect.right-itemRect.left-1;
 nYc=tabRect.bottom-nY-1;

 m_tabPages[m_tabCurrent]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
 for(int nCount=0; nCount < m_nNumberOfPages; nCount++){
  if (nCount!=m_tabCurrent) m_tabPages[nCount]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
 }
}

//////////////////////////////////////////////////////
// CMyTabCtrl message handlers

void CFTBTabCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
 CTabCtrl::OnLButtonDown(nFlags, point);

 if(m_tabCurrent != GetCurFocus()){
  m_tabPages[m_tabCurrent]->ShowWindow(SW_HIDE);
  m_tabCurrent=GetCurFocus();
  m_tabPages[m_tabCurrent]->ShowWindow(SW_SHOW);
  m_tabPages[m_tabCurrent]->SetFocus();
 }
}

void CFTBTabCtrl::SetWindowPos( const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags )
{
#ifndef _DEBUG
//	if (IsWindow(m_hWnd))
	{
		CTabCtrl::SetWindowPos(pWndInsertAfter,x,y,cx,cy,nFlags);
	
		SetRectangle();
	}
#endif
}