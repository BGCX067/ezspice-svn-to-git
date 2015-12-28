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

// EzSpiceView.cpp : CEzSpiceView 클래스의 구현
//

#include "stdafx.h"
#include "EzSpice.h"

#include "MainFrm.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include "DrawTool.h"
#include "ElementSearchDlg.h"
#include "ElementFactory.h"
#include "BaseElement.h"
#include <algorithm>
#include <math.h>
#include <afxinet.h>
#include "CircultSimulation.h"
#include "DigitalMultimeter.h"

int CEzSpiceView::m_GridExtent = 10;

// #ifdef _DEBUG
// #define new DEBUG_NEW
// #endif


// CEzSpiceView

IMPLEMENT_DYNCREATE(CEzSpiceView, CScrollView)

BEGIN_MESSAGE_MAP(CEzSpiceView, CScrollView)
	// 표준 인쇄 명령입니다.
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_MOUSEWHEEL()
	ON_UPDATE_COMMAND_UI(ID_USER_GRIDLINE, &CEzSpiceView::OnUpdateUserGridline)
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CEzSpiceView::OnFilePrintPreview)
	ON_COMMAND(ID_USER_GRIDLINE, &CEzSpiceView::OnUserGridline)
	ON_COMMAND(ID_USER_DEVICE_DISIT, &CEzSpiceView::OnUserDeviceDisit)
	ON_COMMAND(ID_USER_DEVICE_FUNC, &CEzSpiceView::OnUserDeviceFunc)
	ON_COMMAND(ID_USER_DEVICE_OSCIL, &CEzSpiceView::OnUserDeviceOscil)
	ON_COMMAND(ID_USER_DEVICE_POWER, &CEzSpiceView::OnUserDevicePower)
	ON_COMMAND(ID_USER_STATE_UNDO, &CEzSpiceView::OnUserStateUndo)
	ON_COMMAND(ID_USER_STATE_REDO, &CEzSpiceView::OnUserStateRedo)
	ON_COMMAND(ID_USER_STATE_INCREASE, &CEzSpiceView::OnUserStateIncrease)
	ON_COMMAND(ID_USER_STATE_DECREASE, &CEzSpiceView::OnUserStateDecrease)
	ON_COMMAND(ID_USER_SRARCH_DETAIL, &CEzSpiceView::OnUserSrarchDetail)
	ON_COMMAND(ID_USER_SIMUL_PROV1, &CEzSpiceView::OnUserSimulProv1)
	ON_COMMAND(ID_USER_SIMUL_PROV2, &CEzSpiceView::OnUserSimulProv2)
	ON_COMMAND(ID_USER_SIMUL_START, &CEzSpiceView::OnUserSimulStart)
	ON_COMMAND(ID_EDIT_UNDO, &CEzSpiceView::OnEditUndo)
	ON_MESSAGE(UM_RECV_EVENT, &CEzSpiceView::OnRecvEvent)
	ON_COMMAND(ID_JUNCTION, &CEzSpiceView::OnJunction)
	ON_COMMAND(ID_GROUND, &CEzSpiceView::OnGround)
	ON_COMMAND(ID_DECIDE, &CEzSpiceView::OnDecide)
	ON_UPDATE_COMMAND_UI(ID_DECIDE, &CEzSpiceView::OnUpdateDecide)
	ON_UPDATE_COMMAND_UI(ID_GROUND, &CEzSpiceView::OnUpdateGround)
	ON_COMMAND(ID_EDIT_COPY, &CEzSpiceView::OnEditCopy)
//	ON_COMMAND(ID_USER_MATH, &CEzSpiceView::OnUserMath)
	ON_COMMAND(ID_USER_LINENUMBER, &CEzSpiceView::OnUserLinenumber)
	ON_UPDATE_COMMAND_UI(ID_USER_LINENUMBER, &CEzSpiceView::OnUpdateUserLinenumber)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

// CEzSpiceView 생성/소멸

CEzSpiceView::CEzSpiceView()
{

}

CEzSpiceView::~CEzSpiceView()
{
	m_SelectElement.clear();
}

BOOL CEzSpiceView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CEzSpiceView 그리기

void CEzSpiceView::OnDraw(CDC* pDC)
{
	CEzSpiceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	m_ScrollPos = GetScrollPosition();
	CClientDC dc(this); 
	OnPrepareDC(&dc);
	dc.SetBkMode(TRANSPARENT);

	CRect ClientRect;
	GetClientRect(&ClientRect);

 	Gdiplus::Bitmap mBitmap(ClientRect.Width(), ClientRect.Height());
	Gdiplus::Graphics graphics(dc);
	Gdiplus::Graphics MemDC(&mBitmap);

	Gdiplus::SolidBrush WhiteBrush(Gdiplus::Color(255,255,255));
 	MemDC.FillRectangle(&WhiteBrush, Rect(0, 0, ClientRect.Width(), ClientRect.Height()));
	MemDC.SetSmoothingMode(SmoothingModeHighQuality);
	
	if (!pDC->IsPrinting() && m_Grid) {
		DrawGrid(&MemDC);
	}
	DrawConnectdescription(&MemDC);

	pDoc->Draw(&MemDC, this);
	graphics.DrawImage(&mBitmap, 0, 0); 
}

void CEzSpiceView::DrawConnectdescription(Gdiplus::Graphics* pDC)
{	
	list<CPoint>::iterator pos = m_Connectdescription.begin();
	while (pos != m_Connectdescription.end()) {	
		if (m_ConnerFlag) {
			SolidBrush NewBrush(Color(255, 0, 0 , 255));
			pDC->FillRectangle(&NewBrush, RectF((float)(*pos).x-5, (float)(*pos).y-5, 10.0f, 10.0f));
		}
		else {
			SolidBrush NewBrush(Color(255, 255, 0 , 0));
			pDC->FillEllipse(&NewBrush, RectF((float)(*pos).x-5, (float)(*pos).y-5, 10.0f, 10.0f));
		}
		++pos;
	}
}

void CEzSpiceView::DrawGrid(Gdiplus::Graphics* pDC)
{
	CRect ClientRect;
	GetClientRect(&ClientRect);

	Pen BlackPen(Color(125, 0, 0 , 0), 0.5f);
	REAL arDash[]={1.0f, 9.0f, 1.0f, 9.0f}; 
	BlackPen.SetDashStyle(DashStyleCustom);
	BlackPen.SetDashPattern(arDash,sizeof(arDash)/sizeof(arDash[0]));
	
	int Width = ClientRect.Width();
	for (float Ypos = 0; Ypos < ClientRect.Height(); Ypos += m_GridExtent) {
		pDC->DrawLine(&BlackPen, 0, (int)Ypos, Width, (int)Ypos);
	}
}


//////////////////////////////////////////////////////////////////////////
// CEzSpiceView 인쇄


void CEzSpiceView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CEzSpiceView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CEzSpiceView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CEzSpiceView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}

void CEzSpiceView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CEzSpiceView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CEzSpiceView 진단

#ifdef _DEBUG
void CEzSpiceView::AssertValid() const
{
	CView::AssertValid();
}

void CEzSpiceView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CEzSpiceDoc* CEzSpiceView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEzSpiceDoc)));
	return (CEzSpiceDoc*)m_pDocument;
}
#endif //_DEBUG


// CEzSpiceView 메시지 처리기

void CEzSpiceView::OnInitialUpdate()	//스크롤뷰의 설정///필요하면 블로그에 스크롤 조절법 잇음
{
	CScrollView::OnInitialUpdate();

	m_Grid = TRUE;
	m_nZoomRate = 100;
	m_pSearchDlg = NULL;
	m_pMultimeterDlg = NULL;
	m_DrawState =  selection;
	m_ActivityFlag = TRUE;
	m_ConnerFlag = TRUE;
	m_JunctionFlag = FALSE;
	m_Bitmap = NULL;

//	#ifdef _DEBUG
	m_DecideFlag = TRUE;
	m_GroundFlag = TRUE;
	m_LineStringFlag = TRUE;
//	#endif

	::DragAcceptFiles(this->m_hWnd, TRUE);

	ResizeScrollSizes();
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));


	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->m_wndElementView.ReloadElementData();

	m_CrossCursor = AfxGetApp()->LoadCursor(IDC_CROSS);
	m_OciloscopeFlag = OciloscopeEnd;
}

void CEzSpiceView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDrawTool *pTool = CDrawTool::FindTool(this, m_DrawState);
	if (pTool != NULL) {
		pTool->OnLButtonDown(this, nFlags, point);
	}


	CView::OnLButtonDown(nFlags, point);
}

void CEzSpiceView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDrawTool* pTool = CDrawTool::FindTool(this, m_DrawState);
	if (pTool != NULL)
		pTool->OnLButtonUp(this, nFlags, point);

	CView::OnLButtonUp(nFlags, point);
}

void CEzSpiceView::OnMouseMove(UINT nFlags, CPoint point)
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	
	if (pFrame->GetActiveView() != this) {
		if (!m_ActivityFlag) {
			m_ActivityFlag = TRUE;
			SetFocus();
		}
			
		//커서 재 조정
		if (CBaseElement::m_DrawState == Connectline) {
			if (GetCursor() != AfxGetApp()->LoadStandardCursor(IDC_CROSS)) {
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
			}
		}
		else {
			if (GetCursor() != AfxGetApp()->LoadStandardCursor(IDC_ARROW)) {
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
			}
		}
	}

	CDrawTool* pTool = CDrawTool::FindTool(this, m_DrawState);
	if (pTool != NULL)
		pTool->OnMouseMove(this, nFlags, point);

	CView::OnMouseMove(nFlags, point);
}

void CEzSpiceView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint ConvertPos(point);

	CEzSpiceDoc *pDoc = GetDocument();
	
	CBaseElement* pElement = pDoc->IsObjected(ConvertPos);
	if (pElement != NULL) {

		CPoint ConvertPos(point);
		CDrawTool::GridPostion(ConvertPos);

		CClientDC dc(this);
		OnPrepareDC(&dc);
		dc.DPtoLP(&ConvertPos);

		pElement->OnEditProperties(NULL, ConvertPos);
	}
	CScrollView::OnLButtonDblClk(nFlags, point);
}


void CEzSpiceView::OnUserGridline()
{
	m_Grid = !m_Grid;
	Invalidate(FALSE);	
}

void CEzSpiceView::OnUpdateUserGridline(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_Grid);
}


void CEzSpiceView::OnUserDeviceDisit()
{
	if (m_pMultimeterDlg != NULL) {
		m_pMultimeterDlg->SetFocus();
	}
	else {
		m_pMultimeterDlg = new CDigitalMultimeter;
		m_pMultimeterDlg->Create(IDD_MULTMITER, this);
		m_pMultimeterDlg->SetCEzSpiceViewHandle(this);
		m_pMultimeterDlg->ShowWindow(SW_SHOW);		
	}
}

void CEzSpiceView::OnUserDeviceFunc()
{
	AfxMessageBox(_T("아직 지원하지 않습니다."));
}

void CEzSpiceView::OnUserDevicePower()
{
	AfxMessageBox(_T("아직 지원하지 않습니다."));
}

void CEzSpiceView::OnUserStateUndo()
{
	OnEditUndo();
}

void CEzSpiceView::OnUserStateRedo()
{
	OnEditRedo();
}


void CEzSpiceView::OnUserStateIncrease()
{
	m_nZoomRate += 20;
	if(m_nZoomRate > 200)	m_nZoomRate = 200;
	ResizeScrollSizes();
	RedrawWindow();
	
}

void CEzSpiceView::OnUserStateDecrease()
{
	m_nZoomRate -= 20;
	if(m_nZoomRate < 100)		m_nZoomRate = 100;
	ResizeScrollSizes();
	RedrawWindow();
}

BOOL CEzSpiceView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if((nFlags & MK_CONTROL) != MK_CONTROL)  {
		int Min = 0, Max = 0; 
		GetScrollRange(SB_VERT, &Min, &Max);
		int CurrentPos = GetScrollPos(SB_VERT);
		if(zDelta < 0) {			
			if (CurrentPos + 20 < Max) 
				SetScrollPos(SB_VERT, CurrentPos + 20);
		}
		else {
			if (CurrentPos - 20 > Min)
				SetScrollPos(SB_VERT, CurrentPos - 20);
		}
		Invalidate(FALSE);
		return CView::OnMouseWheel(nFlags, zDelta, pt);
	}
		

	if(zDelta < 0) {
		m_nZoomRate += 10;
		if(m_nZoomRate > 200)	m_nZoomRate = 200;
	}
	else {
		m_nZoomRate -= 10;
		if(m_nZoomRate < 100)		m_nZoomRate = 100;
	}

	ResizeScrollSizes();
	Invalidate(FALSE);
	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CEzSpiceView::ResizeScrollSizes()
{
	CRect rc;
	GetClientRect(&rc);
	CSize sizeViewPage;
	sizeViewPage.cx = rc.right-rc.left;
	sizeViewPage.cy = rc.bottom-rc.top;

	//아래 사이즈를 조절하게 되면 변한다.
	CSize sizeViewPageTotal;
	float ScrollRate = (float)m_nZoomRate;
	ScrollRate /= 100;
	
	sizeViewPageTotal.cx = (LONG)(sizeViewPage.cx * ScrollRate);
	sizeViewPageTotal.cy = (LONG)(sizeViewPage.cy * ScrollRate);

	SetScrollSizes(MM_TEXT, sizeViewPageTotal);
}


void CEzSpiceView::OnUserSrarchDetail()
{
	if (m_pSearchDlg != NULL) {
		m_pSearchDlg->SetFocus();
	}
	else {
		m_pSearchDlg = new CElementSearchDlg;
		m_pSearchDlg->Create(IDD_ELEMENTSEARCH, this);
		m_pSearchDlg->SetCEzSpiceViewHandle(this);
		m_pSearchDlg->ShowWindow(SW_SHOW);		
	}
}

void CEzSpiceView::SetSituation( int state )
{
	m_DrawState = state;				//Tool을 결정하기 위한 변수
	CBaseElement::m_DrawState = state;	//삽입시는 마우스 UP시 Selelct객체에 삽입(활성화), 선택시는 마우스Down시에 select객체에 삽입(활성화)

	if (state == Connectline) {
		CLineTool::m_MergerFlag = FALSE;
	}
}

void CEzSpiceView::RotateElement()
{
	if (m_SelectElement.size() == 1) {
		list<CBaseElement*>::iterator pos = m_SelectElement.begin();
		if ((*pos)->m_Flag != lines && (*pos)->m_Flag != prov && (*pos)->m_Flag != ground) {
			(*pos)->RotationElement();
		}
	}
	Invalidate(FALSE);
}

void CEzSpiceView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((GetKeyState (VK_CONTROL) & 0x8000) && (nChar == 'Y' || nChar == 'y')) {
		OnEditRedo();
	}	
	else if((GetKeyState (VK_CONTROL) & 0x8000) && (nChar == 'A' || nChar == 'a')) {
		AllActivityElement();
	}
	else if((GetKeyState (VK_CONTROL) & 0x8000) && (nChar == 'R' || nChar == 'r')) {
		RotateElement();
	}
	else if((GetKeyState (VK_F1) & 0x8000)) {
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->OnFormulaHelp(this);
	}
	else {
		switch (nChar) {
		case 'w':
		case 'W':
			{
				m_SelectElement.clear();
				m_SelectModityElement.clear();

				if (m_DrawState == selection) {
					SetSituation(Connectline);
					SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
					Invalidate(FALSE);
				}
				else if (m_DrawState == Connectline) {
					SetSituation(selection);
					SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
				}
				Invalidate(FALSE);
			}
			break;

		case VK_TAB:
			{
				CDrawTool* pElement = CDrawTool::FindTool(this, m_DrawState);
				if (pElement != NULL)
					pElement->OnTabDown(this);

			}
			break;

		case VK_ESCAPE:
			{
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
				m_Connectdescription.clear();
				m_SelectElement.clear();
				m_SelectModityElement.clear();
				m_JunctionFlag = FALSE;

				if (CDrawTool::m_pNewFirstLine != NULL) {
					if (CLineTool::m_pFirstDivideElement != NULL && CLineTool::m_pSecondDivideElement != NULL) {
						CLineTool::m_pFirstDivideElement->m_MoveTo = CLineTool::m_pSecondDivideElement->m_MoveTo;

						list<CBaseElement*>::iterator pos = CLineTool::m_pSecondDivideElement->m_ConnectElement.begin();
						while (pos != CLineTool::m_pSecondDivideElement->m_ConnectElement.end()) {
							if ((*pos) != CLineTool::m_pFirstDivideElement) {
								CLineTool::m_pFirstDivideElement->m_ConnectElement.push_back((*pos));
								(*pos)->m_ConnectElement.push_back(CLineTool::m_pFirstDivideElement);
							}
							++pos;
						}
						GetDocument()->DeleteElement(CLineTool::m_pSecondDivideElement);
						CLineTool::m_pFirstDivideElement = NULL;
						CLineTool::m_pSecondDivideElement = NULL;
					}
					GetDocument()->DeleteElement(CDrawTool::m_pNewFirstLine);
					CDrawTool::m_pNewFirstLine = NULL;
					CLineTool::m_pLastElement = NULL;
				}
				
				if (CDrawTool::m_pNewSecondLine != NULL) {
					GetDocument()->DeleteElement(CDrawTool::m_pNewSecondLine);
					CDrawTool::m_pNewSecondLine = NULL;
				}

				if (CDrawTool::m_pNewElement != NULL && m_DrawState == insert) {
					GetDocument()->DeleteElement(CDrawTool::m_pNewElement);
				}
				CDrawTool::m_pNewElement = NULL;
				SetSituation(selection);
				Invalidate(FALSE);
			}
			break;

		case VK_DELETE:
			{
				DeleteAllActivityElemnt();
				CDrawTool::m_pNewElement = NULL;
				//GetDocument()->MergerConnectLine();
				GetDocument()->MoveMergerConnectLine();
			}
			break;
		default:
			break;
		}
	}
	
	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CEzSpiceView::AllActivityElement()
{
	m_SelectElement.clear();
	m_SelectElement.resize(GetDocument()->m_CircuitList.size());
	copy(GetDocument()->m_CircuitList.begin(), GetDocument()->m_CircuitList.end(), m_SelectElement.begin());
	Invalidate(FALSE);
}

BOOL CEzSpiceView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{

	//	KeyDwon시 Setcursor를 해주는데 그냥 TRUE해줘야지 커서 유지됨,
// 	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
	return TRUE;
}

void CEzSpiceView::SetActivityModityElement( CPoint &point )
{
	LineModity NewModity;
	NewModity.ConnectPoint = point;
	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {
		
		if ((*pos)->m_Flag == lines) {
			CPoint ConnectPosition = (*pos)->ComparePosition(point);
			if (ConnectPosition.x != 0 && ConnectPosition.y != 0) {

				ModityInfo Newinfo;
				Newinfo.ConnectElement = (*pos);
				Newinfo.MovePosition = (*pos)->GetPositonDirection(point);
				Newinfo.ConnectCount = 0;

				CPoint RemainPoint;
				(*pos)->m_LineTo == point ? RemainPoint = (*pos)->m_MoveTo : RemainPoint = (*pos)->m_LineTo;
				list<CBaseElement*>::iterator lowpos = GetDocument()->m_CircuitList.begin();
				while (lowpos != GetDocument()->m_CircuitList.end()) {
					if (RemainPoint == (*lowpos)->m_LineTo) Newinfo.ConnectCount++;
					else if (RemainPoint == (*lowpos)->m_MoveTo) Newinfo.ConnectCount++;
					++lowpos;
				}
				NewModity.ConnectList.push_back(Newinfo);
			}
		}
		++pos;
	}
	m_SelectModityElement.push_back(NewModity);

	Invalidate(FALSE);
}

void CEzSpiceView::SetActivityElement( CBaseElement* pElement )
{
	m_SelectElement.push_back(pElement);
	Invalidate(FALSE);
}

void CEzSpiceView::SetNonActivityElement(CBaseElement * pElement)
{
	list<CBaseElement*>::iterator pos = find(m_SelectElement.begin(), m_SelectElement.end(), pElement);
	if (pos == m_SelectElement.end()) {
		m_SelectElement.remove(pElement);
	}
	Invalidate(FALSE);
}


BOOL CEzSpiceView::IsSelected( CBaseElement* pElement )
{
	list<CBaseElement*>::iterator pos = find(m_SelectElement.begin(), m_SelectElement.end(), pElement);
	if (pos == m_SelectElement.end()) {
		return FALSE;
	}
	return TRUE;		
}


void CEzSpiceView::Connectdescription()
{
	typedef struct _ConnectTemp
	{
		list<CBaseElement*> TempList;
	}ConnectTemp;

	list<ConnectTemp> ConnectList;

	list<CPoint>::iterator pos = m_Connectdescription.begin();
	while (pos != m_Connectdescription.end()) {
		
		ConnectTemp NewConnect;
		list<CBaseElement*>::iterator Elementpos = GetDocument()->m_CircuitList.begin();
		while (Elementpos != GetDocument()->m_CircuitList.end()) {
			
			if ((*Elementpos)->m_Flag != lines) {	
				list<CPoint>::iterator Lowpos = (*Elementpos)->m_ConnectPoint.begin();
				while (Lowpos != (*Elementpos)->m_ConnectPoint.end()) {
					
					if ((*Elementpos)->m_Position +(*Lowpos) == (*pos)) {
						NewConnect.TempList.push_back((*Elementpos));
					}
					++Lowpos;
				}
			}
			else {
				CPoint ConnectPosition = (*Elementpos)->ComparePosition((*pos));
				if (ConnectPosition.x != 0 && ConnectPosition.y != 0) {
					NewConnect.TempList.push_back((*Elementpos));
				}
			}
			++Elementpos;
		}

		if (NewConnect.TempList.size() == 2) {
			ConnectList.push_back(NewConnect);
		}
		++pos;
	}


	list<ConnectTemp>::iterator connectpos = ConnectList.begin();
	while (connectpos != ConnectList.end()) {

		list<CBaseElement*>::iterator temppos = (*connectpos).TempList.begin();
		list<CBaseElement*>::iterator lowpos = (*connectpos).TempList.begin();
		while (lowpos != (*connectpos).TempList.end()) {

			if ((*lowpos) != (*temppos)) {
				(*temppos)->m_ConnectElement.push_back((*lowpos));
				(*lowpos)->m_ConnectElement.push_back((*temppos));
			}
			++lowpos;
		}
		++connectpos;
	}
	m_Connectdescription.clear();
}


void CEzSpiceView::MoveActivityModityElement( const CPoint& point )	//라인의 모서리에 의한 라인 이동이 아닌 수정 로직
{
	typedef struct _Delaywork
	{
		CBaseElement *Parent;
		CBaseElement *DeleteChild;
		CBaseElement *InsertChild;
	}Delaywork;
	list<Delaywork> DelayList;

	if (m_SelectModityElement.size() > 0) {
		list<LineModity>::iterator Moditypos = m_SelectModityElement.begin();

		list<ModityInfo>::iterator pos = (*Moditypos).ConnectList.begin();
		while (pos != (*Moditypos).ConnectList.end()) {								//pos 선택된 객체
		
			if ((*pos).MovePosition == MoveToConnect) {	//마우스로 선택한 지점이 해당객체의 MoveTo이다 (여기는 LineTo 영향 로직)

				list<CBaseElement*>::iterator lowpos = (*pos).ConnectElement->m_ConnectElement.begin();
				while (lowpos != (*pos).ConnectElement->m_ConnectElement.end()) {

					ModityInfo CompareInfo;
					CompareInfo.ConnectElement = (*lowpos);
 					list<ModityInfo>::iterator Duplicatepos = find_if((*Moditypos).ConnectList.begin(), (*Moditypos).ConnectList.end(), CompareInfo);
 					if (Duplicatepos == (*Moditypos).ConnectList.end()) { //중복이 없을때

						if ((*lowpos)->m_Flag != lines) { //하지만 소자일때
 
							if ((*pos).ConnectElement->m_Axis == Xaxis && point.y != 0) {
								CElementFactory *pFactory = CMainFrame::CreateElementFactory();
								CBaseElement *pNewElement = pFactory->CreateElement(lines);

								CPoint ConnectRect = (*lowpos)->ConnectRectDirection((*pos).ConnectElement->m_LineTo);
								if (ConnectRect.x != 0 && ConnectRect.y != 0) {
									pNewElement->m_LineTo = ConnectRect;
									ConnectRect.y += point.y;
									pNewElement->m_MoveTo = ConnectRect;
									pNewElement->m_Axis = GetAxis(pNewElement);
								}

								pNewElement->m_ConnectElement.push_back((*pos).ConnectElement);
								pNewElement->m_ConnectElement.push_back((*lowpos));

								Delaywork NewWork;
								NewWork.Parent = (*pos).ConnectElement;
								NewWork.DeleteChild = (*lowpos);
								NewWork.InsertChild = pNewElement;
								DelayList.push_back(NewWork);
							}
							else if ((*pos).ConnectElement->m_Axis == Yaxis && point.x != 0) {
								CElementFactory *pFactory = CMainFrame::CreateElementFactory();
								CBaseElement *pNewElement = pFactory->CreateElement(lines);

								CPoint ConnectRect = (*lowpos)->ConnectRectDirection((*pos).ConnectElement->m_LineTo);
								if (ConnectRect.x != 0 && ConnectRect.y != 0) {
									pNewElement->m_LineTo = ConnectRect;
									ConnectRect.x += point.x;
									pNewElement->m_MoveTo = ConnectRect;
									pNewElement->m_Axis = GetAxis(pNewElement);
								}

								pNewElement->m_ConnectElement.push_back((*pos).ConnectElement);
								pNewElement->m_ConnectElement.push_back((*lowpos));

								Delaywork NewWork;
								NewWork.Parent = (*pos).ConnectElement;
								NewWork.DeleteChild = (*lowpos);
								NewWork.InsertChild = pNewElement;
								DelayList.push_back(NewWork);
							}
						}
						else {
							if ((*lowpos)->m_LineMoveDirection == -1) {
								if ((*pos).ConnectElement->m_LineTo == (*lowpos)->m_LineTo) (*lowpos)->m_LineMoveDirection = LineToConnect;
								else (*lowpos)->m_LineMoveDirection = MoveToConnect;
							}

							if ((*pos).ConnectCount < 3) {
								if ((*lowpos)->m_LineMoveDirection == LineToConnect) {
									(*lowpos)->m_Axis == Xaxis ? (*lowpos)->m_LineTo.x += point.x : (*lowpos)->m_LineTo.y += point.y;
								}
								else {
									(*lowpos)->m_Axis == Xaxis ? (*lowpos)->m_MoveTo.x += point.x : (*lowpos)->m_MoveTo.y += point.y;
								}
							}
						}
 					}
					++lowpos;
				}
				(*pos).ConnectElement->m_Axis == Xaxis 
					? (*pos).ConnectCount > 2 ? NULL : (*pos).ConnectElement->m_LineTo.y += point.y 
					: (*pos).ConnectCount > 2 ? NULL : (*pos).ConnectElement->m_LineTo.x += point.x;
				(*pos).ConnectElement->m_MoveTo += point;		
			}
			else {	//LineTo Duplication

				list<CBaseElement*>::iterator lowpos = (*pos).ConnectElement->m_ConnectElement.begin();
				while (lowpos != (*pos).ConnectElement->m_ConnectElement.end()) {

					ModityInfo CompareInfo;
					CompareInfo.ConnectElement = (*lowpos);
					list<ModityInfo>::iterator Duplicatepos = find_if((*Moditypos).ConnectList.begin(), (*Moditypos).ConnectList.end(), CompareInfo);
					if (Duplicatepos == (*Moditypos).ConnectList.end()) { //중복이 없을때

						if ((*lowpos)->m_Flag != lines) { //하지만 소자일때

							if ((*pos).ConnectElement->m_Axis == Xaxis && point.y != 0) {
								CElementFactory *pFactory = CMainFrame::CreateElementFactory();
								CBaseElement *pNewElement = pFactory->CreateElement(lines);

								CPoint ConnectRect = (*lowpos)->ConnectRectDirection((*pos).ConnectElement->m_MoveTo);
								if (ConnectRect.x != 0 && ConnectRect.y != 0) {
									pNewElement->m_LineTo = ConnectRect;
									ConnectRect.y += point.y;
									pNewElement->m_MoveTo = ConnectRect;
									pNewElement->m_Axis = GetAxis(pNewElement);
								}

								pNewElement->m_ConnectElement.push_back((*pos).ConnectElement);
								pNewElement->m_ConnectElement.push_back((*lowpos));

								Delaywork NewWork;
								NewWork.Parent = (*pos).ConnectElement;
								NewWork.DeleteChild = (*lowpos);
								NewWork.InsertChild = pNewElement;
								DelayList.push_back(NewWork);
							}
							else if ((*pos).ConnectElement->m_Axis == Yaxis && point.x != 0) {
								CElementFactory *pFactory = CMainFrame::CreateElementFactory();
								CBaseElement *pNewElement = pFactory->CreateElement(lines);

								CPoint ConnectRect = (*lowpos)->ConnectRectDirection((*pos).ConnectElement->m_MoveTo);
								if (ConnectRect.x != 0 && ConnectRect.y != 0) {
									pNewElement->m_LineTo = ConnectRect;
									ConnectRect.x += point.x;
									pNewElement->m_MoveTo = ConnectRect;
									pNewElement->m_Axis = GetAxis(pNewElement);
								}

								pNewElement->m_ConnectElement.push_back((*pos).ConnectElement);
								pNewElement->m_ConnectElement.push_back((*lowpos));

								Delaywork NewWork;
								NewWork.Parent = (*pos).ConnectElement;
								NewWork.DeleteChild = (*lowpos);
								NewWork.InsertChild = pNewElement;
								DelayList.push_back(NewWork);
							}
  						}
						else {
							if ((*lowpos)->m_LineMoveDirection == -1) {
								if ((*pos).ConnectElement->m_MoveTo == (*lowpos)->m_LineTo) (*lowpos)->m_LineMoveDirection = LineToConnect;
								else (*lowpos)->m_LineMoveDirection = MoveToConnect;
							}

							if ((*pos).ConnectCount < 3) {
								if ((*lowpos)->m_LineMoveDirection == LineToConnect) {
									(*lowpos)->m_Axis == Xaxis ? (*lowpos)->m_LineTo.x += point.x : (*lowpos)->m_LineTo.y += point.y;
								}
								else {
									(*lowpos)->m_Axis == Xaxis ? (*lowpos)->m_MoveTo.x += point.x : (*lowpos)->m_MoveTo.y += point.y;
								}
							}
						}
					}
					++lowpos;
				}
				(*pos).ConnectElement->m_Axis == Xaxis 
					? (*pos).ConnectCount > 2 ? NULL : (*pos).ConnectElement->m_MoveTo.y += point.y 
					: (*pos).ConnectCount > 2 ? NULL : (*pos).ConnectElement->m_MoveTo.x += point.x;
				(*pos).ConnectElement->m_LineTo += point;		
 			}
			++pos;
		}
		(*Moditypos).ConnectPoint += point;
	}

	list<Delaywork>::iterator Workpos = DelayList.begin();
	while (Workpos != DelayList.end()) {
		(*Workpos).DeleteChild->m_ConnectElement.push_back((*Workpos).InsertChild);
		(*Workpos).DeleteChild->DeleteElement((*Workpos).Parent);

		(*Workpos).Parent->m_ConnectElement.push_back((*Workpos).InsertChild);
		(*Workpos).Parent->DeleteElement((*Workpos).DeleteChild);

		GetDocument()->m_CircuitList.push_back((*Workpos).InsertChild);
		++Workpos;
	}

	ExceptionConnecting();
	Invalidate(FALSE);
}

void CEzSpiceView::ExceptionConnecting()
{
	//좌표로 검색하는 설계가 문제점이 있어서 예외처리

	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {

		if ((*pos)->m_Flag == lines) {	//소자는 무조건 라인과 연결이다

			list<CBaseElement*>::iterator lowpos = (*pos)->m_ConnectElement.begin();
			while (lowpos != (*pos)->m_ConnectElement.end()) {
				
				BOOL nFlag = FALSE;
				
				if ((*lowpos)->m_Flag != lines) {	//라인과 연결된 객체가 소자일때

					list<CPoint>::iterator connectpoint = (*lowpos)->m_ConnectPoint.begin();
					while (connectpoint != (*lowpos)->m_ConnectPoint.end()) {

						if ((*pos)->m_LineTo == (*lowpos)->m_Position + (*connectpoint)
							|| (*pos)->m_MoveTo == (*lowpos)->m_Position + (*connectpoint)) {
								nFlag = TRUE;
						}
						++connectpoint;
					}

					if (nFlag == FALSE) { //예외 발생시 lowpos객체와 가까운 연결점과 연결
						
						vector<int> SortList;
						list<CPoint>::iterator connectpoint = (*lowpos)->m_ConnectPoint.begin();
						while (connectpoint != (*lowpos)->m_ConnectPoint.end()) {

							SortList.push_back(GetDistance((*pos)->m_LineTo, (*lowpos)->m_Position + (*connectpoint)));
							SortList.push_back(GetDistance((*pos)->m_MoveTo, (*lowpos)->m_Position + (*connectpoint)));
							++connectpoint;
						}
						sort(SortList.begin(), SortList.end());
						vector<int>::iterator Minpos = SortList.begin();

						connectpoint = (*lowpos)->m_ConnectPoint.begin();
						while (connectpoint != (*lowpos)->m_ConnectPoint.end()) {

							if ((*Minpos) == GetDistance((*pos)->m_LineTo, (*lowpos)->m_Position + (*connectpoint))) {
								(*pos)->m_LineTo = (*lowpos)->m_Position + (*connectpoint);
								break;
							}
							else if ((*Minpos) == GetDistance((*pos)->m_MoveTo, (*lowpos)->m_Position + (*connectpoint))) {
								(*pos)->m_MoveTo = (*lowpos)->m_Position + (*connectpoint);
								break;
							}
							++connectpoint;
						}
					}
				}
				else {		//라인과 연결된 객체가 라인일때

					if ((*pos)->m_LineTo == (*lowpos)->m_LineTo || (*pos)->m_LineTo == (*lowpos)->m_MoveTo
						|| (*pos)->m_MoveTo == (*lowpos)->m_LineTo || (*pos)->m_MoveTo == (*lowpos)->m_MoveTo) {
							nFlag = TRUE;
					}

					if (nFlag == FALSE) {
						int LineToLine = GetDistance((*pos)->m_LineTo, (*lowpos)->m_LineTo);
						int LineToMove = GetDistance((*pos)->m_LineTo, (*lowpos)->m_MoveTo);
						int MoveToLine = GetDistance((*pos)->m_MoveTo, (*lowpos)->m_LineTo);
						int MoveToMove = GetDistance((*pos)->m_MoveTo, (*lowpos)->m_MoveTo);

						vector<int> SortList;
						SortList.push_back(LineToLine);
						SortList.push_back(LineToMove);
						SortList.push_back(MoveToLine);
						SortList.push_back(MoveToMove);

						sort(SortList.begin(), SortList.end());
						
						vector<int>::iterator Minpos = SortList.begin();
						if ((*Minpos) == LineToLine) {
							(*lowpos)->m_LineTo = (*pos)->m_LineTo;
						}
						else if ((*Minpos) == LineToMove) {
							(*lowpos)->m_MoveTo = (*pos)->m_LineTo;
						}
						else if ((*Minpos) == MoveToLine) {
							(*lowpos)->m_LineTo = (*pos)->m_MoveTo;
						}
						else if ((*Minpos) == MoveToMove) {
							(*lowpos)->m_MoveTo = (*pos)->m_MoveTo;
						}
					}
				}
				++lowpos;
			}
		}
		++pos;
	}
}

int	CEzSpiceView::GetDistance(CPoint FirstPosition, CPoint SecondPosition)
{
	return (int)sqrt(pow((float)FirstPosition.x - SecondPosition.x, 2) + pow((float)FirstPosition.y - SecondPosition.y, 2));
}

int CEzSpiceView::GetAxis( CBaseElement *pElement )
{
								
	
	//축을 반대로 변경
	float Angleaxis = 0;
	Angleaxis = abs((float)(pElement->m_LineTo.y - pElement->m_MoveTo.y) / abs((float)(pElement->m_LineTo.x - pElement->m_MoveTo.x)));

	int Axis;
	if (Angleaxis == 0) {
		if (pElement->m_MoveTo.x == pElement->m_LineTo.x && pElement->m_MoveTo.y != pElement->m_LineTo.y) {
			Axis = Yaxis;
		}
		else {
			Axis = Xaxis;
		}
	}
	else {
		if (Angleaxis < 0.45 && Angleaxis > -0.45) {
			Axis = Xaxis;
		}
		else {
			Axis = Yaxis;
		}
	}
	return Axis;
}

void CEzSpiceView::DecisionAxis()
{
	//축 결정
	list<CBaseElement*>::iterator Axispos = GetDocument()->m_CircuitList.begin();
	while (Axispos != GetDocument()->m_CircuitList.end()) {
		(*Axispos)->m_Axis = GetAxis((*Axispos));
		(*Axispos)->m_LineMoveDirection = -1;
		++Axispos;
	}
}

void CEzSpiceView::MoveActivityElement( const CPoint& point )
{
	if (point.x ==0 && point.y == 0) {
		return;
	}
	//선택된 객체가 소자일시 
	//기본적으로 링크를 두번타서 두개 다 이동한다
	//하지만 두번째 링크가 소자라면 새로운 객체가 추가되서 그 객체가 이동된다.

	//선택된 객체가 라인일시
	//링크를 한번타서 이동시킨다.

	//기본적으로 해당 객체의 하부링크들이 선택이 안됬을때 하부 객체의 값이 변경된다.

	typedef struct _Delaywork
	{
		CBaseElement *Parent;
		CBaseElement *DeleteChild;
		CBaseElement *InsertChild;
	}Delaywork;
	list<Delaywork> DelayList;

	list<CBaseElement*>::iterator pos = m_SelectElement.begin();
	while (pos != m_SelectElement.end()) {

		if ((*pos)->m_Flag != lines) {	//선택된 객체가 소자일때.

			list<CBaseElement*>::iterator Lowpos = (*pos)->m_ConnectElement.begin();
			while (Lowpos != (*pos)->m_ConnectElement.end()) {

				if ((*Lowpos) != (*pos) && IsSelected((*Lowpos)) == FALSE) {	//자신을 빼고

					list<CBaseElement*>::iterator Underpos = (*Lowpos)->m_ConnectElement.begin();
					while (Underpos != (*Lowpos)->m_ConnectElement.end()) {

						if (IsSelected((*Underpos)) == FALSE) {	//활성화 않되어 있나면?
							if ((*Underpos)->m_Flag != lines) {	//연결놈이 소자일때 객체 추가

								CPoint ConvertPoint(point);
								if (ConvertPoint.x != 0 && ConvertPoint.y != 0) {
									if ((*Lowpos)->m_Axis == Xaxis) ConvertPoint.x = 0;
									else ConvertPoint.y = 0;
								}

								CElementFactory *pFactory = CMainFrame::CreateElementFactory();
								CBaseElement *pNewElement = pFactory->CreateElement(lines);

								CPoint ConnectRect = (*Underpos)->ConnectRectDirection((*Lowpos)->m_LineTo);
								if (ConnectRect.x != 0 && ConnectRect.y != 0) {
									pNewElement->m_LineTo = ConnectRect;
									pNewElement->m_MoveTo = ConnectRect + ConvertPoint;
								}

								ConnectRect = (*Underpos)->ConnectRectDirection((*Lowpos)->m_MoveTo);
								if (ConnectRect.x != 0 && ConnectRect.y != 0) {
									pNewElement->m_LineTo = ConnectRect + ConvertPoint;
									pNewElement->m_MoveTo = ConnectRect;
								}

								pNewElement->m_ConnectElement.push_back((*Lowpos));
								pNewElement->m_ConnectElement.push_back((*Underpos));
								
								pNewElement->m_LineMoveDirection = -1;
								pNewElement->m_Axis = GetAxis(pNewElement);

								if ((*Lowpos)->m_Axis != pNewElement->m_Axis) {
									Delaywork NewWork;
									NewWork.Parent = (*Lowpos);
									NewWork.DeleteChild = (*Underpos);
									NewWork.InsertChild = pNewElement;
									DelayList.push_back(NewWork);
								}
								else {
									delete pNewElement;
								}
							}
							else {	//이동

 								int ConnectCount = (*Lowpos)->m_ConnnectCount;

								if ((*Lowpos)->m_LineMoveDirection == -1) {
									if ((*Lowpos)->m_LineTo == (*Underpos)->m_MoveTo) ConnectCount = GetElementCountToPoint((*Lowpos)->m_LineTo);
									else ConnectCount = GetElementCountToPoint((*Lowpos)->m_MoveTo);
									(*Lowpos)->m_ConnnectCount = ConnectCount;
								}

								if (ConnectCount < 3) {
									if ((*Underpos)->m_LineMoveDirection == -1) {
										(*Underpos)->m_LineMoveDirection = GetConnectDirection((*Lowpos), (*Underpos));
									}

									if ((*Underpos)->m_LineMoveDirection == LineToConnect) {		
										(*Underpos)->m_Axis == Xaxis ? (*Underpos)->m_LineTo.x += point.x : (*Underpos)->m_LineTo.y += point.y;
									}
									else {	//MoveToConnect
										(*Underpos)->m_Axis == Xaxis ? (*Underpos)->m_MoveTo.x += point.x : (*Underpos)->m_MoveTo.y += point.y;
									}
								}
							}
						}
						++Underpos;
					}

					if ((*Lowpos)->m_LineMoveDirection == -1) {
						CPoint ConnectRect = (*pos)->ConnectRectDirection((*Lowpos)->m_LineTo);
						if (ConnectRect.x != 0 && ConnectRect.y != 0) (*Lowpos)->m_LineMoveDirection = LineToConnect;
						else (*Lowpos)->m_LineMoveDirection = MoveToConnect;
					}

					if ((*Lowpos)->m_Axis == Xaxis) {
						if ((*Lowpos)->m_LineMoveDirection == LineToConnect) {
							(*Lowpos)->m_LineTo += point;
							if ((*Lowpos)->m_ConnnectCount < 3) (*Lowpos)->m_MoveTo.y += point.y;
						}
						else {
							(*Lowpos)->m_MoveTo += point;
							if ((*Lowpos)->m_ConnnectCount < 3) (*Lowpos)->m_LineTo.y += point.y;
						}
						
					}
					else  if ((*Lowpos)->m_Axis == Yaxis) { 
						if ((*Lowpos)->m_LineMoveDirection == LineToConnect) {
							(*Lowpos)->m_LineTo += point;
							if ((*Lowpos)->m_ConnnectCount < 3) (*Lowpos)->m_MoveTo.x += point.x;
						}
						else {
							(*Lowpos)->m_MoveTo += point;
							if ((*Lowpos)->m_ConnnectCount < 3) (*Lowpos)->m_LineTo.x += point.x;
						}
					}
				}
				++Lowpos;
			}
		}
		else {	//선택된 객체가 라인인일때
			//pos내부에 연결된 놈이 현재 활성화 인지 체크
			list<CBaseElement*>::iterator Lowpos = (*pos)->m_ConnectElement.begin();
			while (Lowpos != (*pos)->m_ConnectElement.end()) {

				if (IsSelected((*Lowpos)) == FALSE) {	//활성화 않되어 있나면?
					if ((*Lowpos)->m_Flag != lines) {	//연결놈이 소자일때

						CElementFactory *pFactory = CMainFrame::CreateElementFactory();
						CBaseElement *pNewElement = pFactory->CreateElement(lines);

						CPoint ConnectRect = (*Lowpos)->ConnectRectDirection((*pos)->m_LineTo);
						if (ConnectRect.x != 0 && ConnectRect.y != 0) {
							pNewElement->m_LineTo = ConnectRect;
							pNewElement->m_MoveTo = ConnectRect + point;
						}

						ConnectRect = (*Lowpos)->ConnectRectDirection((*pos)->m_MoveTo);
						if (ConnectRect.x != 0 && ConnectRect.y != 0) {
							pNewElement->m_LineTo = ConnectRect + point;
							pNewElement->m_MoveTo = ConnectRect;
						}

						pNewElement->m_ConnectElement.push_back((*pos));
						pNewElement->m_ConnectElement.push_back((*Lowpos));

						Delaywork NewWork;
						NewWork.Parent = (*pos);
						NewWork.DeleteChild = (*Lowpos);
						NewWork.InsertChild = pNewElement;
						DelayList.push_back(NewWork);
					}
					else {

						if (GetConnectDirection((*pos), (*Lowpos)) == LineToConnect) {
							(*Lowpos)->m_LineTo +=  point;		
						}
						else {	//MoveToConnect
							(*Lowpos)->m_MoveTo +=  point;
						}
					}
				}
				++Lowpos;
			}
		}
		(*pos)->m_LineTo +=point;
		(*pos)->m_MoveTo +=point;
		(*pos)->m_Position += point;

		m_Connectdescription.clear();
		if ((*pos)->m_Flag != lines && (*pos)->m_Flag != ground) {	//소자 이동만으로 라인에 붙이기?
	
			CEzSpiceDoc *pDoc = GetDocument();
			list<CPoint>::iterator pointpos = (*pos)->m_ConnectPoint.begin();	//이동소자의 내부 연결 위치를 전체 리스트중 라인 객체의 끝과 비교
			while (pointpos != (*pos)->m_ConnectPoint.end()) {

				if ((*pos)->IsDuplicateConnectPoint((*pointpos) + (*pos)->m_Position) == FALSE) {
					list<CBaseElement*>::iterator Elementpos = pDoc->m_CircuitList.begin();
					while (Elementpos != pDoc->m_CircuitList.end()) {

						if ((*Elementpos) != (*pos) && (*Elementpos)->m_Flag == lines) {

							list<CBaseElement*>::iterator Duplicatepos = find((*pos)->m_ConnectElement.begin(), (*pos)->m_ConnectElement.end(), (*Elementpos));
							if (Duplicatepos == (*pos)->m_ConnectElement.end()) {

								CPoint ConnectPosition = (*Elementpos)->ComparePosition((*pointpos) + (*pos)->m_Position);
								if (ConnectPosition.x != 0 && ConnectPosition.y != 0) {

									DuplicateCPoint DupliPoint;
									DupliPoint.point = ConnectPosition;
									list<CPoint>::iterator DuplicatePoint = find_if(m_Connectdescription.begin(), m_Connectdescription.end(), DupliPoint);
									if (DuplicatePoint == m_Connectdescription.end()) {
										m_Connectdescription.push_back(ConnectPosition);
									}
								}
							}
						}
						++Elementpos;
					}
				}
				++pointpos;
			}
		}
		++pos;
	}

	list<Delaywork>::iterator Workpos = DelayList.begin();
	while (Workpos != DelayList.end()) {
		(*Workpos).DeleteChild->m_ConnectElement.push_back((*Workpos).InsertChild);
		(*Workpos).DeleteChild->DeleteElement((*Workpos).Parent);

		(*Workpos).Parent->m_ConnectElement.push_back((*Workpos).InsertChild);
		(*Workpos).Parent->DeleteElement((*Workpos).DeleteChild);
		
		GetDocument()->m_CircuitList.push_back((*Workpos).InsertChild);
		++Workpos;
	}
	ExceptionConnecting();
	Invalidate(FALSE);
}

int	CEzSpiceView::GetElementCountToPoint(CPoint point)
{
	int iCount = 0;
	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {
		if ((*pos)->m_LineTo == point) iCount++;
		else if ((*pos)->m_MoveTo == point) iCount++;
		++pos;
	}
	return iCount;
}


int CEzSpiceView::GetConnectDirection(CBaseElement *pBaseElement, CBaseElement *pCompareElement)
{
	int Result = LineToConnect;

	if (pBaseElement->m_Flag != lines ) {	//소자 주변이 라인일때
		
		
		list<CPoint>::iterator pos = pBaseElement->m_ConnectPoint.begin();
		while (pos != pBaseElement->m_ConnectPoint.end()) {
			
			CPoint Position = CPoint(pBaseElement->m_Position.x + (*pos).x, pBaseElement->m_Position.y + (*pos).y);
			if (Position == pCompareElement->m_MoveTo) {
				Result = MoveToConnect;
			}
			else if (Position == pCompareElement->m_LineTo) {
				Result = LineToConnect;
			}
 			++pos;
		}
	}
	else {
		if (pBaseElement->m_MoveTo == pCompareElement->m_MoveTo) {
			Result = MoveToConnect;
		}
		else if (pBaseElement->m_MoveTo == pCompareElement->m_LineTo) {
			Result = LineToConnect;
		}
		else if (pBaseElement->m_LineTo == pCompareElement->m_LineTo) {
			Result = LineToConnect;
		}
		else if (pBaseElement->m_LineTo == pCompareElement->m_MoveTo) {
			Result = MoveToConnect;
		}
	}
	return Result;
}


void CEzSpiceView::SetActivityInRect( const CPoint& down, const CPoint& last )
{
	//전달되는 Point 2개는 기본적인 드래그 방식, 나머지 3개 방향 드래그 감지
	CRect ConvertRect(down, last);	
	if (down.x > last.x) {
		if (last.y > down.y) {									//우측 위에서 좌측 하단으로 드래그
			ConvertRect.SetRect(last.x, down.y, down.x, last.y);
		}
		else {													//우측 하단에서 좌측 상단으로 드래그
			ConvertRect.SetRect(last, down);
		}
	}
	else {
		if (down.y > last.y) {									//좌측 하단에서 우측 상단으로 드래그
			ConvertRect.SetRect(down.x, last.y, last.x, down.y);
		}
	}

	//DPtoLP
	CEzSpiceDoc *pDoc = GetDocument();
	list<CBaseElement*>::iterator pos = pDoc->m_CircuitList.begin();
	while (pos != pDoc->m_CircuitList.end()) {

		if ((*pos)->m_Flag != lines) {	//소자일때
			if ((*pos)->m_Position.x > ConvertRect.left && (*pos)->m_Position.y > ConvertRect.top
				&& (*pos)->m_Position.x < ConvertRect.right && (*pos)->m_Position.y < ConvertRect.bottom) {
					m_SelectElement.push_back((*pos));
			}
		}
		else {	//라인일때
			if (isCrossBoundBox(CPoint(ConvertRect.left, ConvertRect.top), CPoint(ConvertRect.left, ConvertRect.bottom),
						(*pos)->m_LineTo, (*pos)->m_MoveTo)
				|| isCrossBoundBox(CPoint(ConvertRect.left, ConvertRect.bottom), CPoint(ConvertRect.right, ConvertRect.bottom),
						(*pos)->m_LineTo, (*pos)->m_MoveTo)
				|| isCrossBoundBox(CPoint(ConvertRect.right, ConvertRect.top), CPoint(ConvertRect.right, ConvertRect.bottom),
						(*pos)->m_LineTo, (*pos)->m_MoveTo)
				|| isCrossBoundBox(CPoint(ConvertRect.left, ConvertRect.top), CPoint(ConvertRect.right, ConvertRect.top),
						(*pos)->m_LineTo, (*pos)->m_MoveTo)
				|| (*pos)->m_LineTo.x > ConvertRect.left && (*pos)->m_LineTo.y > ConvertRect.top
						&& (*pos)->m_LineTo.x < ConvertRect.right && (*pos)->m_LineTo.y < ConvertRect.bottom
				|| (*pos)->m_MoveTo.x > ConvertRect.left && (*pos)->m_MoveTo.y > ConvertRect.top
						&& (*pos)->m_MoveTo.x < ConvertRect.right && (*pos)->m_MoveTo.y < ConvertRect.bottom) {
				m_SelectElement.push_back((*pos));
			}
		}
		++pos;
	}

	Invalidate(FALSE);
}

BOOL CEzSpiceView::isCrossBoundBox(CPoint FirstLineTo, CPoint FirstMoveTo, CPoint SecondLineTo, CPoint SecondMoveTo ) 
{ 
	return max( FirstLineTo.x, FirstMoveTo.x ) >= min( SecondLineTo.x, SecondMoveTo.x ) 
		&& max( SecondLineTo.x, SecondMoveTo.x ) >= min( FirstLineTo.x, FirstMoveTo.x ) 
		&& max( FirstLineTo.y, FirstMoveTo.y ) >= min( SecondLineTo.y, SecondMoveTo.y ) 
		&& max( SecondLineTo.y, SecondMoveTo.y ) >= min( FirstLineTo.y, FirstMoveTo.y ); 
} 

BOOL CEzSpiceView::CheckIntersect(CPoint FirstLineTo, CPoint FirstMoveTo, CPoint SecondLineTo, CPoint SecondMoveTo ) 
{ 
	BOOL nResult = FALSE; 
	BOOL nFlag = isCrossBoundBox( FirstLineTo, FirstMoveTo, SecondLineTo, SecondMoveTo ); 
	int z1 = 0, z2 = 0, z3 = 0, z4 = 0;
	if( nFlag ) 
	{ 
		z1 = ( SecondLineTo.x - FirstLineTo.x ) * ( FirstMoveTo.y - FirstLineTo.y ) - ( SecondLineTo.y - FirstLineTo.y ) * ( FirstMoveTo.x - FirstLineTo.x ); 
		z2 = ( SecondMoveTo.x - FirstLineTo.x ) * ( FirstMoveTo.y - FirstLineTo.y ) - ( SecondMoveTo.y - FirstLineTo.y ) * ( FirstMoveTo.x - FirstLineTo.x ); 
		z3 = ( FirstLineTo.x - SecondLineTo.x ) * ( SecondMoveTo.y - SecondLineTo.y ) - ( FirstLineTo.y - SecondLineTo.y ) * ( SecondMoveTo.x - SecondLineTo.x ); 
		z4 = ( FirstMoveTo.x - SecondLineTo.x ) * ( SecondMoveTo.y - SecondLineTo.y ) - ( FirstMoveTo.y - SecondLineTo.y ) * ( SecondMoveTo.x - SecondLineTo.x ); 
		if( ( (z1 * z2) <= 0 ) && ( (z3 * z4) <= 0 ) )
			nResult = true; 
	} 
	return nResult; 
}



void CEzSpiceView::DeleteAllActivityElemnt()
{
	CEzSpiceDoc *pDoc = GetDocument();
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();

	list<CBaseElement*>::iterator pos = m_SelectElement.begin();
	while (pos != m_SelectElement.end()) {
		list<CBaseElement*>::iterator Findpos = find(pDoc->m_CircuitList.begin(), pDoc->m_CircuitList.end(), (*pos));

		if ((*Findpos)->m_Flag != lines) {
			pFrame->DeleteClassView((*Findpos));
		}
		pDoc->DeleteElement((*Findpos));
		++pos;
	}
	m_SelectElement.clear();
	Invalidate(FALSE);
}

BOOL CEzSpiceView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}


void CEzSpiceView::InsertElement(int ElementNumber)
{
	if (CDrawTool::m_pNewElement != NULL) {
		GetDocument()->DeleteElement(CDrawTool::m_pNewElement);
		CDrawTool::m_pNewElement = NULL;
	}
	m_SelectElement.clear();
	SetSituation(insert);
	CDrawTool::m_InsertKind = ElementNumber;
}

void CEzSpiceView::OnSetFocus(CWnd* pOldWnd)
{
	CScrollView::OnSetFocus(pOldWnd);

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateClassView(GetDocument());

	if (CBaseElement::m_DrawState == Connectline) {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
	}
	else {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
	Invalidate(FALSE);
}

void CEzSpiceView::OnUserSimulProv1()
{
	if (GetProvCount() < 3) {
		InsertElement(prov);
	}
	else {
		MessageBox(_T("더이상 프로브를 추가할수 없습니다."), _T("프로브 초과"), MB_OK);
	}
}

int CEzSpiceView::GetProvCount()
{
	int TotalCount = 0;
	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {
		if ((*pos)->m_Flag == prov) {
			TotalCount++;
		}
		++pos;
	}
	return TotalCount;
}

int CEzSpiceView::NextProvColor()
{
	BOOL nFlag[3] = {False,};

	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {
		if ((*pos)->m_Flag == prov) {
			
			list<ElementData> GetList;
			(*pos)->GetProperties(GetList);

			list<ElementData>::iterator datapos = GetList.begin();
			while (datapos != GetList.end()) {
				
				if((*datapos).Text == CString(_T("ProvColor"))) {
					nFlag[(int)(*datapos).Value] = TRUE;
				}
				++datapos;
			}
		}
		++pos;
	}

	for (int iCount = 0; iCount < 3; iCount++) {
		if (nFlag[iCount] == FALSE) {
			return iCount;
		}
	}
	return 0;
}


void CEzSpiceView::OnUserSimulProv2()
{
	AfxMessageBox(_T("아직 지원이 되지 않습니다."));
}

void CEzSpiceView::OnUserDeviceOscil()
{
	TCHAR strPath[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, strPath);
	_tcscat_s(strPath, _T("\\Ociloscope.exe"));

	SHELLEXECUTEINFO sei;// 구조체 선언.
	memset(&sei, 0, sizeof(sei)); // 구조체를 0으로 채움

	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT; 
	sei.lpFile = strPath;
	sei.hwnd = NULL;
	sei.lpVerb = _T("open"); // open, print 등등 사용하실 수 있어요.!
	sei.nShow = SW_SHOWMINIMIZED;
	ShellExecuteEx(&sei); //  (구조체)프로그램 실행
}

void CEzSpiceView::OnUserSimulStart()
{
	//쓰레드로 변경하기
	SetSituation(selection);
	m_SelectElement.clear();
	CDrawTool::ProvProcessing(this);

	CCircultSimulation Simulation;
	if (Simulation.Initialize(GetDocument()->m_CircuitList) == FALSE) {
		return;
	}

	list<CBaseElement*> ProvList;
	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {
		if ((*pos)->m_Flag == prov) {
			ProvList.push_back((*pos));
		}
		++pos;
	}

	m_SectorList.clear();
	m_IPCDataList.clear();
	if (Simulation.FormulaConversion(m_SectorList, m_IPCDataList, ProvList)) {
		m_Header.DataType = VoltageData;
		m_Header.DataCount = m_IPCDataList.size();

		if (m_OciloscopeFlag == OciloscopeEnd) {
			if (MessageBox(_T("결과를 오실로스코프에 출력합니까?"), _T("시뮬레이션 알림"), MB_ACTIVE) != IDCANCEL) {
				OnUserDeviceOscil();
			}
		}
		else if (m_OciloscopeFlag == OciloscopeStart) {
			SendOciloscope();
		}
	}
	Invalidate(FALSE);
}

void CEzSpiceView::MultimeterSimulStart()
{
	//쓰레드로 변경하기
	SetSituation(selection);
	m_SelectElement.clear();
	CDrawTool::ProvProcessing(this);

	CCircultSimulation Simulation;
	if (Simulation.Initialize(GetDocument()->m_CircuitList) == FALSE) {
		return;
	}

	list<CBaseElement*> ProvList;
	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {
		if ((*pos)->m_Flag == prov) {
			ProvList.push_back((*pos));
		}
		++pos;
	}

	m_SectorList.clear();
	m_IPCDataList.clear();
	if (Simulation.FormulaConversion(m_SectorList, m_IPCDataList, ProvList)) {
		if (m_pMultimeterDlg != NULL) {
			m_pMultimeterDlg->InitializeProvList(m_IPCDataList);
		}
	}
	Invalidate(FALSE);
}

void CEzSpiceView::OnEditUndo()
{
	CEzSpiceDoc *pDoc = GetDocument();
	if (pDoc->Undo() == TRUE) {
		m_SelectElement.clear();
		Invalidate(FALSE);
	}
}

void CEzSpiceView::OnEditRedo()
{
	CEzSpiceDoc *pDoc = GetDocument();
	if (pDoc->Redo() == TRUE) {
		m_SelectElement.clear();
		Invalidate(FALSE);
	}
}



void CEzSpiceView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CScrollView::OnPrepareDC(pDC, pInfo);

	pDC->SetMapMode(MM_ISOTROPIC);					
	pDC->SetWindowExt(100, 100);					//논리적인 화면DC의 크기를 100*100으로 고정한다.
	pDC->SetViewportExt(m_nZoomRate, m_nZoomRate);	//뷰포트의 크기를 변경한다.

	CPoint pos = GetScrollPosition();
	pDC->SetViewportOrg(-pos.x, -pos.y);			//픽셀단위 좌표로 400, 400이 논리적 좌표0, 0이 되도록 한다
}


////////////////////////////////////////////////////////////////////////// IPC 통신


LRESULT CEzSpiceView::OnRecvEvent(WPARAM wParam, LPARAM lParam)
{
	IPCHeader Header;
	memcpy_s(&Header, sizeof(IPCHeader), theApp.m_pSharedMemory, sizeof(IPCHeader));

	switch (Header.DataType) {
		case OciloscopeStart:
			{
				m_OciloscopeFlag = OciloscopeStart;
				SendOciloscope();
			}
			break;

		case OciloscopeEnd:
			m_OciloscopeFlag = OciloscopeEnd;
			break;

		default:
			break;
	}
	return 0;
}

void CEzSpiceView::SendOciloscope()
{
	if(!UpdateData(TRUE)) {
		return;
	}

	if(theApp.m_Mutex.Lock(1000)) {
		CEvent	EventSend(FALSE, TRUE, TEXT("IPC_READ_SHAREDMEMORY"));
	
		//Header copy
		CMainFrame *pFrmae = (CMainFrame*)AfxGetMainWnd();
		m_Header.SecMax = pFrmae->m_SecMax;
		m_Header.VoltMax = pFrmae->m_VoltMax;
		memcpy_s(theApp.m_pSharedMemory, sizeof(IPCHeader), &m_Header, sizeof(m_Header));

		//Data copy
		int iDataCount = 0;
		list<IPCData>::iterator pos = m_IPCDataList.begin();
		while (pos != m_IPCDataList.end()) {

			m_SectorList[iDataCount].DataColor = GetProvColor((*pos).ProvNumber);
			memcpy_s(theApp.m_pSharedMemory + sizeof(IPCHeader) + sizeof(IPCSectorHeader)*iDataCount + sizeof(IPCData)*iDataCount
				, sizeof(IPCSectorHeader), &m_SectorList[iDataCount], sizeof(IPCSectorHeader));

			memcpy_s(theApp.m_pSharedMemory + sizeof(IPCHeader) + sizeof(IPCSectorHeader)*(iDataCount+1) + sizeof(IPCData)*iDataCount, sizeof(IPCData), &(*pos), sizeof(IPCData));
			++iDataCount;
			++pos;
		}
		m_IPCDataList.clear();

		EventSend.SetEvent();
		::Sleep(1);
		EventSend.ResetEvent();
		theApp.m_Mutex.Unlock();
	}
	else {
		AfxMessageBox(_T("ERROR: Lock() return FALSE!"));
		return;
	}

	UpdateData(FALSE);
}

int CEzSpiceView::GetProvColor(int ProvNumber)
{
	list<CBaseElement*>::iterator pos = GetDocument()->m_CircuitList.begin();
	while (pos != GetDocument()->m_CircuitList.end()) {
		if ((*pos)->m_Flag == prov && (*pos)->m_ElementNumber == ProvNumber) {
			
			list<ElementData> GetList;
			(*pos)->GetProperties(GetList);

			list<ElementData>::iterator datapos = GetList.begin();
			while (datapos != GetList.end()) {
				if ((*datapos).Text == CString(_T("ProvColor"))) {
					return (int)(*datapos).Value;
				}
				++datapos;
			}
		}
		++pos;
	}
	return -1;
}


void CEzSpiceView::OnJunction() //교차점 판단 및 적용
{
	m_JunctionFlag = TRUE;
	m_ActivityFlag = FALSE;
}

void CEzSpiceView::OnGround()
{
 	m_GroundFlag = !m_GroundFlag;
	Invalidate(FALSE);
}

void CEzSpiceView::OnUpdateGround(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_GroundFlag);
}

void CEzSpiceView::OnDecide()
{
	m_DecideFlag = !m_DecideFlag;
	Invalidate(FALSE);
}

void CEzSpiceView::OnUpdateDecide(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_DecideFlag);
}



void CEzSpiceView::OnEditCopy()
{
	if (m_SelectElement.size() == 1) {
		list<CBaseElement*>::iterator Elementpos = m_SelectElement.begin();

		if ((*Elementpos)->m_Flag != lines && (*Elementpos)->m_Flag != prov && (*Elementpos)->m_Flag != acpower) {
			InsertElement((*Elementpos)->m_Flag);
		}
	}
}

void CEzSpiceView::CopyDrawImage()
{
	if (m_Bitmap != NULL) {
		delete m_Bitmap;
		m_Bitmap = NULL;
	}

	CClientDC dc(this); // device context for painting
	dc.SetBkMode(TRANSPARENT);

	CRect ClientRect;
	GetClientRect(&ClientRect);

	m_Bitmap = new Gdiplus::Bitmap(ClientRect.Width(), ClientRect.Height());
	Gdiplus::Graphics graphics(dc);
	Gdiplus::Graphics MemDC(m_Bitmap);

	Gdiplus::SolidBrush WhiteBrush(Gdiplus::Color(0,0,0));
	MemDC.FillRectangle(&WhiteBrush, Rect(0, 0, ClientRect.Width(), ClientRect.Height()));
	MemDC.SetSmoothingMode(SmoothingModeHighQuality);

	DrawGrid(&MemDC);
}


void CEzSpiceView::OnUserLinenumber()
{
	m_LineStringFlag = !m_LineStringFlag;
	Invalidate(FALSE);
}

void CEzSpiceView::OnUpdateUserLinenumber(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LineStringFlag);
}

void CEzSpiceView::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szFileName[_MAX_PATH];
	int nCount=0;

	nCount = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	for (int i=0; i<nCount; i++) {
		::DragQueryFile(hDropInfo, i, szFileName, _MAX_PATH);
		
		GetDocument()->LoadDorgFile(szFileName);
	}
	CScrollView::OnDropFiles(hDropInfo);
}
