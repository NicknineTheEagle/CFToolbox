// TabBandwidth.cpp : implementation file
//

#include "stdafx.h"
#include "CFToolbox.h"
#include "TabBandwidth.h"
#include "Bandwidth.h"
#include "tools.h"
#include "CFManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabBandwidth dialog

CTabBandwidth * theOneAndUniqueTabBandwidth;
CTabBandwidth * getTabBandwidth()
{
	return theOneAndUniqueTabBandwidth;
}


CTabBandwidth::CTabBandwidth(CWnd* pParent /*=NULL*/)
: CDialog(CTabBandwidth::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabBandwidth)
	m_current = _T("");
	m_min = _T("");
	m_max = _T("");
	m_remaining = _T("");
	//}}AFX_DATA_INIT
	m_bandwidthGraph=new CHistogramCtrl(getBandwidthMonitor()->nbInterval);
}


void CTabBandwidth::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabBandwidth)
 	DDX_Control(pDX, IDC_HIDER, m_hider);
	DDX_Control(pDX, IDC_FROM, m_from);
	DDX_Control(pDX, IDC_COMBO1, m_downloads);
	DDX_Text(pDX, IDC_CURRENT, m_current);
	DDX_Text(pDX, IDC_MINIMUM, m_min);
	DDX_Control(pDX, IDC_EXPLORER2, m_explorer);
	DDX_Text(pDX, IDC_MAX, m_max);
	DDX_Text(pDX, IDC_REMAINING, m_remaining);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabBandwidth, CDialog)
//{{AFX_MSG_MAP(CTabBandwidth)
ON_WM_SIZE()
ON_WM_TIMER()
ON_WM_SHOWWINDOW()
ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeDownloader)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabBandwidth message handlers
BOOL CTabBandwidth::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
			pMsg->wParam=NULL ;
	} 
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CTabBandwidth::OnSize(UINT nType, int cx, int cy) 
{
	
	CDialog::OnSize(nType, cx, cy);
	
#ifndef _DEBUG
	//	m_filesCtrl.SetWindowPos( &wndTop, 0, 0, cx,  cy, 0 );	
#endif
	
}

BOOL CTabBandwidth::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CRect rect;
	GetDlgItem(IDC_BANDWIDTH_GRAPH)->GetWindowRect(rect);
	ScreenToClient(rect);
	
	m_bandwidthGraph->Create(WS_VISIBLE | WS_CHILD, rect, this, 100);
    m_bandwidthGraph->SetRange(0,1);
	
	m_bandwidthGraph->DrawSpike();
	m_bandwidthGraph->InvalidateCtrl();
	theOneAndUniqueTabBandwidth=this;
	m_downloads.ResetContent();
	m_downloads.AddString(" All downloads");
	m_downloads.SetItemData(0,0);
	m_downloads.SetCurSel(0);
	
	explorerW=m_explorer.GetWidth();
	explorerH=m_explorer.GetHeight();


	setCSBannerVisibility(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
#include "debug.h"
void CTabBandwidth::OnTimer(UINT nIDEvent) 
{
	
	switch (nIDEvent)
	{
	case 3:
	case 1:
		{
			if (nIDEvent==1) UpdateData(TRUE);
			DWORD values[100];
			// TODO: Add your message handler code here and/or call default
			int size=getBandwidthMonitor()->nbInterval;
			CCFDownloader * associatedTask=(CCFDownloader*)m_downloads.GetItemData(m_downloads.GetCurSel());
			char remainingTime[200];
			*remainingTime=0;
			if (associatedTask) 
			{
				getBandwidthMonitor()->getBandwidth(associatedTask->cfId,values);
				DWORD r=associatedTask->remainingTime;
				if (r)
				{
					renderTime(r,remainingTime);
				}
			}
			else
			{
				getBandwidthMonitor()->getTotalBandwidth(values);
			
				DWORD maxR=0;
				for (int d=0;d<m_downloads.GetCount();d++)
				{
					CCFDownloader * associatedTask=(CCFDownloader*)m_downloads.GetItemData(d);
					if (associatedTask)
					{
						DWORD r=associatedTask->remainingTime;
						if (r>maxR) maxR=r;
					}
				}
				if (maxR)
				{
					renderTime(maxR,remainingTime);
				}

			}
			
			if (strlen(remainingTime))
			{
				m_remaining=remainingTime;
			}
			else
				m_remaining="";

			DWORD max=0;
			DWORD min=0x7FFFFFFF;
			for (int ind=0;ind<size;ind++)
			{
				
				if (ind && ind<size-1){
					m_bandwidthGraph->values[ind]=(2* values[ind]+ values[ind+1]+values[ind-1])/4;
				}
				if (max<m_bandwidthGraph->values[ind]) max=m_bandwidthGraph->values[ind]; 
				if (m_bandwidthGraph->values[ind]!=0 && min>m_bandwidthGraph->values[ind]) min=m_bandwidthGraph->values[ind]; 
				
				
			}
			
			int interval=getBandwidthMonitor()->intervalSec;
			int current=m_bandwidthGraph->values[size-2];
			char speed[40];
			
			renderSize(max/interval,speed,false);
			m_max.Format("%s/Sec",speed);
			
			if (min==0x7FFFFFFF) min=0;
			renderSize(min/interval,speed,false);
			m_min.Format("%s/Sec",speed);
			
			renderSize(current/interval,speed,false);
			m_current.Format("%s/Sec",speed);
			
			DWORD diff= (max-min)/8;
			
			if (diff>min) min=0; else min-=diff;
			max+=diff;
			
			
			m_bandwidthGraph->SetRange(min,max);
			m_bandwidthGraph->DrawSpike();
			m_bandwidthGraph->InvalidateCtrl();
			
			if (nIDEvent==1) UpdateData(FALSE);
			break;
		}
	case 0 : // hider
		{
			m_hider.ShowWindow( SW_HIDE );
			m_hider.ShowWindow( SW_SHOW );
			break;
		}
	case 2 : // refresh content server banner
		{
			CCFDownloader * associatedTask=(CCFDownloader*)m_downloads.GetItemData(m_downloads.GetCurSel());
			if (associatedTask)
			{
				if (strlen(associatedTask->contentServerURL)) 
				{
					m_explorer.Navigate(associatedTask->contentServerURL,NULL,NULL,NULL,NULL);
					KillTimer(nIDEvent);
				}
			}
			else
				KillTimer(nIDEvent);
			
			break;
		}
	}
}

void CTabBandwidth::setCSBannerVisibility(bool show)
{
	//m_explorer.ShowWindow((show?SW_SHOW:SW_HIDE));//SetVisible(show);
	m_explorer.SetWidth((show ? explorerW : 0));
	m_explorer.SetHeight((show ? explorerH : 0));
	m_hider.ShowWindow((show?SW_SHOW:SW_HIDE));
 
	m_from.ShowWindow((show?SW_SHOW:SW_HIDE));
}

void CTabBandwidth::addDownloader(char * name, CTask * task)
{
	m_downloads.AddString(name);
	m_downloads.SetItemData(m_downloads.FindString(0,name),(DWORD)task);
}

void CTabBandwidth::removeDownloader(char * name, CTask * task)
{
	int toDelete=m_downloads.FindString(0,name);
	if (toDelete==m_downloads.GetCurSel()) 
	{
		setCSBannerVisibility(false);
		m_downloads.SetCurSel(0);
		OnSelchangeDownloader();
	}
	m_downloads.DeleteString(toDelete);
	if (m_downloads.GetCount()==1) setCSBannerVisibility(false);
}

void CTabBandwidth::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		
		OnTimer(1);	
		SetTimer(1,getBandwidthMonitor()->intervalSec*1000/2,NULL);
		
	}
	else
	{
		KillTimer(1);
		
	}
}

void CTabBandwidth::OnSelchangeDownloader() 
{
	// TODO: Add your control notification handler code here
	setCSBannerVisibility(false);
	CCFDownloader * associatedTask=(CCFDownloader*)m_downloads.GetItemData(m_downloads.GetCurSel());
	if (associatedTask) 
	{
		OnTimer(0);
		SetTimer(0,1000,NULL);
		if (!strlen(associatedTask->contentServerURL)) SetTimer(2,500,NULL);
		else
			m_explorer.Navigate(associatedTask->contentServerURL,NULL,NULL,NULL,NULL);
		
		setCSBannerVisibility(true);
	}
	else
		KillTimer(0);
	OnTimer(3);
}
