// HistogramCtrl.cpp : implementation file
//

// stdafx.cpp : source file that includes just the standard includes
//	CPanel.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#include "HistogramCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistogramCtrl

CHistogramCtrl::CHistogramCtrl(DWORD size)
{
	m_size=size;
	values=(DWORD*)malloc(4*size);
	memset(values,0,4*size);
//	m_nPos      = 0;
	m_nLower    = 0;
	m_nUpper    = 100;
}

CHistogramCtrl::~CHistogramCtrl()
{
	free(values);
}


BEGIN_MESSAGE_MAP(CHistogramCtrl, CWnd)
	//{{AFX_MSG_MAP(CHistogramCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSpikeCtrl message handlers

BOOL CHistogramCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	return  CWnd::CreateEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, 
		className, NULL, dwStyle, 
		rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU) nID);
}
 
void CHistogramCtrl::SetRange(UINT nLower, UINT nUpper)
{
	if (nLower<0 || nLower>0xFFFF) nLower=0;
	if (nUpper==0) nUpper=1; 

	m_nLower = nLower;
	m_nUpper = nUpper;
 
}
 
void CHistogramCtrl::InvalidateCtrl()
{
	// Small optimization that just invalidates the client area
	// (The borders don't usually need updating)
	CClientDC dc(this);
	CRect rcClient;

	GetClientRect(rcClient);

	if (m_MemDC.GetSafeHdc() == NULL)
	{
		m_MemDC.CreateCompatibleDC(&dc);
		m_Bitmap.CreateCompatibleBitmap(&dc,rcClient.Width(),rcClient.Height());
		m_MemDC.SelectObject(m_Bitmap);
		// draw scale
	/*	m_MemDC.SetBkColor(RGB(0,0,0));
		CBrush bkBrush(HS_HORIZONTAL,RGB(128,128,0));
		m_MemDC.FillRect(rcClient,&bkBrush); */
	}
	InvalidateRect(rcClient); 

} 
/*
UINT CHistogramCtrl::SetPos(UINT nPos)
{
	if (nPos > m_nUpper)
		nPos = m_nUpper;

	if (nPos < m_nLower)
		nPos = m_nLower;

	UINT nOld = m_nPos;
	m_nPos = nPos;

	DrawSpike();

	Invalidate();

	return nOld;
}*/

void CHistogramCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
	// draw scale
	CRect rcClient;
	GetClientRect(rcClient);

	// draw scale
	if (m_MemDC.GetSafeHdc() != NULL)
	{
		dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_MemDC, 0, 0, SRCCOPY);
	}
}

void CHistogramCtrl::DrawSpike()
{
/*//	CClientDC dc(this);
	UINT  nRange = m_nUpper - m_nLower;
	CRect rcClient;
	GetClientRect(rcClient);

	if (m_MemDC.GetSafeHdc() != NULL)
	{
		m_MemDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_MemDC, 4, 0, SRCCOPY);

		CRect rcTop(rcClient.right - 4, 0, rcClient.right - 2, rcClient.bottom);
		rcTop.top  = (long) (((float) (m_nPos - m_nLower) / nRange) * rcClient.Height());
		rcTop.top  = rcClient.bottom - rcTop.top;

		// draw scale
		CRect rcRight = rcClient;
		rcRight.left = rcRight.right - 4;
		m_MemDC.SetBkColor(RGB(0,0,0));

    CBrush bkBrush(HS_HORIZONTAL,RGB(0,128,0));  
    m_MemDC.FillRect(rcRight,&bkBrush);

		// draw current spike
		CBrush brush(RGB(0,255,0));
		m_MemDC.FillRect(rcTop, &brush);
	}
	*/

 ;
////

	UINT  nRange = m_nUpper - m_nLower;
	CRect rcClient;
	GetClientRect(rcClient);

	if (m_MemDC.GetSafeHdc() != NULL)
	{
	//	m_MemDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_MemDC, 4, 0, SRCCOPY);
 		// draw scale
//	 	CRect rcRight = rcClient;
//		rcRight.left = rcRight.right;
		m_MemDC.SetBkColor(RGB(200,200,200));

	//	CBrush bkBrush(HS_HORIZONTAL,RGB(128,128,0));  
		CBrush bkBrush(RGB(200,200,200));  
		m_MemDC.FillRect(rcClient,&bkBrush);
 
		// draw current spike
		
		for (unsigned int ind=0;ind<m_size-2;ind++)
		{
			DWORD v=values[ind+1];
			if (v<m_nLower) v=m_nLower;
			if (v>m_nUpper) v=m_nUpper;
			float coef=((float)(v - m_nLower) / (float)nRange);
			int c=200-200*ind/(m_size*2); if (c>200) c=200;
			int r=c-coef*128; if (r<0) r=0;
			int g=c-coef*64; if (g<0) g=0;
			if (values[ind])
			{
				CBrush brush(RGB(r,g,c));
			CRect rcTop(rcClient.left+ (8*ind), 0, rcClient.left + (8*ind+7), rcClient.bottom);
			rcTop.top  = (long) (coef * rcClient.Height());
			rcTop.top  = rcClient.bottom - rcTop.top;
		
			m_MemDC.FillRect(rcTop, &brush);
			}
		}
	}
}
