// 이 MFC 샘플 소스 코드는 MFC Microsoft Office Fluent 사용자 인터페이스("Fluent UI")를 
// 사용하는 방법을 보여 주며, MFC C++ 라이브러리 소프트웨어에 포함된 
// Microsoft Foundation Classes Reference 및 관련 전자 문서에 대해 
// 추가적으로 제공되는 내용입니다.  
// Fluent UI를 복사, 사용 또는 배포하는 데 대한 사용 약관은 별도로 제공됩니다.  
// Fluent UI 라이선싱 프로그램에 대한 자세한 내용은 
// http://msdn.microsoft.com/officeui를 참조하십시오.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#include "OciloscopeDoc.h"
#include "OciloscopeView.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
	m_pView = NULL;
}

COutputWnd::~COutputWnd()
{
	m_pView = NULL;
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 탭 창을 만듭니다.
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("출력 탭 창을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}

	// 출력 창을 만듭니다.
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndHardView.Create(_T("FDSFS"), dwStyle, rectDummy, &m_wndTabs, 2) ) {
		TRACE0("출력 창을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}

	m_wndHardView.SetFont(&m_Font);
	m_wndTabs.AddTab(&m_wndHardView, _T("Wave"), (UINT)0);

	m_PastData = 50;
	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	if (m_pView != NULL) {
		m_pView->SendMessage(UM_VIEW_REDRAW);
	}
}

void COutputWnd::SetView(COciloscopeView *pView)
{
 	m_pView = pView;
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&m_Font);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void COutputWnd::SetGraphData( double data/* = 0*/)
{
	m_wndHardView.SetCpuvalue(data);
}


//////////////////////////////////////////////////////////////////////////   HardWareView
//////////////////////////////////////////////////////////////////////////

CHardWareView::CHardWareView()
{
	for (int iCount = 0; iCount < MAXDATA; iCount++) {
		m_Data[iCount] = double(0);
	}

	m_Gridcount = m_Index = 0;
	m_DisplayMemory = new CDC();
	m_Bitmap = new CBitmap();
	m_Flag = FALSE;
	m_pView = NULL;
	m_PaintDC = NULL;
}

CHardWareView::~CHardWareView()
{
	if (m_Bitmap != NULL) {
		m_Bitmap->DeleteObject();    
		delete m_Bitmap;             
	}

	if (m_DisplayMemory != NULL) {
		m_DisplayMemory->DeleteDC(); 
		delete m_DisplayMemory;      
	}

	if (m_PaintDC != NULL) {
		delete m_PaintDC;
	}
}

BEGIN_MESSAGE_MAP(CHardWareView, CStatic)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList 메시지 처리기

void CHardWareView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void CHardWareView::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();
	}
}

void CHardWareView::DrawAxis(CDC *parm_dc)
{
	CPen *p_old_pen = (CPen *)parm_dc->SelectStockObject(WHITE_PEN);
	int real_width = m_ClientRect.Width() - 40, real_height = m_ClientRect.Height() - 40;

	parm_dc->MoveTo(16, real_height);
	parm_dc->LineTo(real_width + 20, real_height);
	parm_dc->MoveTo(real_width, 16);
	parm_dc->LineTo(real_width, real_height + 20);

	parm_dc->SelectObject(p_old_pen);
}

void CHardWareView::DrawGrid(CDC *parm_dc)	
{
	CPen grid_pen(PS_DOT, 0, RGB(0, 128, 0));
	CPen *p_old_pen = parm_dc->SelectObject(&grid_pen);
	int real_width = m_ClientRect.Width() - 40, real_height = m_ClientRect.Height() - 40;
	double ystep = (m_ClientRect.Height() - 60)/double(5);

	for(int iCount = 0; iCount < 5; iCount++){          // 수평 그리드를 그린다. 
		parm_dc->MoveTo(20, int(real_height - (iCount + 1) * ystep));
		parm_dc->LineTo(real_width, int(real_height - (iCount + 1) * ystep));
	}

	int MaxCount = real_width /20;
	for(int iCount = 1; iCount < MaxCount; iCount++){  // 수직 그리드를 그린다.
		parm_dc->MoveTo(20 + iCount * 20 - m_Gridcount, 20);
		parm_dc->LineTo(20 + iCount * 20 - m_Gridcount, real_height);
	}

	parm_dc->SelectObject(p_old_pen);
	grid_pen.DeleteObject();
}

void CHardWareView::DrawGraph(CDC *parm_dc)
{
	CPen graph_pen(PS_SOLID, 1, RGB(0, 255, 0));
	CPen *p_old_pen = parm_dc->SelectObject(&graph_pen);
	int real_width = m_ClientRect.Width() - 40, real_height = m_ClientRect.Height() - 40;

	int Gap = real_width/MAXDATA;
	for(int i = 0; i <m_Index; i++){
		if(i) parm_dc->LineTo(20 + (i + MAXDATA - m_Index)*Gap, real_height - (int)m_Data[i]/*real_height - int((real_height - 20) * m_Data[i]/100.0) -5*/);
		else parm_dc->MoveTo(20 + (i + MAXDATA - m_Index)*Gap, real_height - (int)m_Data[i]/*real_height - int((real_height - 20) * m_Data[i]/100.0) -5*/);
	}

	
	parm_dc->SelectObject(p_old_pen);
	graph_pen.DeleteObject();
}

void CHardWareView::SetCpuvalue(double Cpuvalue)
{
	Cpuvalue += m_CenterPostion;
		
	if (m_Index > MAXDATA -1) {
		memcpy(m_Data, m_Data + 1, sizeof(double)*(MAXDATA-1));
		m_Index--;
	}
	m_Data[m_Index++] = Cpuvalue;

	Invalidate(FALSE);
}

void CHardWareView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_Flag == FALSE) {

		GetClientRect(m_ClientRect);

		double real_height = m_ClientRect.Height() - 60;
		double ystep = (m_ClientRect.Height() - 60)/double(5);
		m_CenterPostion = (int)(real_height - (3*ystep + ystep/2));

		m_PaintDC = new CPaintDC(this);
		m_DisplayMemory->CreateCompatibleDC(&dc); 
		m_Bitmap->CreateCompatibleBitmap(&dc, m_ClientRect.Width(), m_ClientRect.Height());
		m_Flag = TRUE;
	}

	m_OldBitmap = (CBitmap *) m_DisplayMemory->SelectObject(m_Bitmap);
	CBrush *old_brush = (CBrush *) m_DisplayMemory->SelectStockObject(BLACK_BRUSH);
	m_DisplayMemory->Rectangle(m_ClientRect);
	m_DisplayMemory->SelectObject(old_brush);

	int old_mode = m_DisplayMemory->SetBkMode(TRANSPARENT);

	DrawAxis(m_DisplayMemory);
	DrawGrid(m_DisplayMemory);
	DrawGraph(m_DisplayMemory);
	m_DisplayMemory->SetBkMode(old_mode);

	dc.BitBlt(0, 0, m_ClientRect.Width(), m_ClientRect.Height(), m_DisplayMemory, 0, 0, SRCCOPY);
	m_DisplayMemory->SelectObject(m_OldBitmap);
}

void CHardWareView::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	if (m_Flag == TRUE) {

		if (m_Bitmap != NULL) {
			delete m_Bitmap;
		}

		GetClientRect(&m_ClientRect);
		double real_height = m_ClientRect.Height() - 60;
		double ystep = (m_ClientRect.Height() - 60)/double(5);
		m_CenterPostion = (int)(real_height - (3*ystep + ystep/2));
		m_Bitmap = new CBitmap();
		m_Bitmap->CreateCompatibleBitmap(m_PaintDC, m_ClientRect.Width(), m_ClientRect.Height());
	}
}

BOOL CHardWareView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}
