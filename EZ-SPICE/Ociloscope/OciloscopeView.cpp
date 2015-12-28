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

// OciloscopeView.cpp : COciloscopeView 클래스의 구현
//

#include "stdafx.h"
#include "Ociloscope.h"

#include "MainFrm.h"
#include "OciloscopeDoc.h"
#include "OciloscopeView.h"
#include <math.h>
#include "StructInformation.h"
#include "Plotter.h"

// #ifdef _DEBUG
// #define new DEBUG_NEW
// #endif


// COciloscopeView

IMPLEMENT_DYNCREATE(COciloscopeView, CFormView)

BEGIN_MESSAGE_MAP(COciloscopeView, CFormView)
	ON_MESSAGE(UM_RECV_EVENT, &COciloscopeView::OnRecvEvent)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_COMMAND(ID_MAXFREQUENCY, &COciloscopeView::OnMaxfrequency)
	ON_COMMAND(ID_MAXVOLTAGE, &COciloscopeView::OnMaxvoltage)
	ON_COMMAND(ID_FLOTTER, &COciloscopeView::OnFlotter)
	ON_COMMAND(ID_FLOTTERA, &COciloscopeView::OnFlottera)
	ON_COMMAND(ID_FLOTTERB, &COciloscopeView::OnFlotterb)
	ON_COMMAND(ID_FLOTTERC, &COciloscopeView::OnFlotterc)
	ON_MESSAGE(UM_VIEW_REDRAW, &COciloscopeView::OnReDraw)
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_REFLESH, &COciloscopeView::OnReflesh)
END_MESSAGE_MAP()

// COciloscopeView 생성/소멸


LRESULT COciloscopeView::OnReDraw(WPARAM wParam, LPARAM lParam)
{
	CopyDrawImage();
	Invalidate(FALSE);
	RedrawWindow(NULL);
	return 0;
}

COciloscopeView::COciloscopeView()
	: CFormView(COciloscopeView::IDD)\
{
	m_Bitmap = NULL;
}

COciloscopeView::~COciloscopeView()
{
	if (m_Bitmap != NULL) {
		delete m_Bitmap;
	}
}

void COciloscopeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL COciloscopeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CFormView::PreCreateWindow(cs);
}

void COciloscopeView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	CMainFrame *pFrame = (CMainFrame*)GetParentFrame();
	pFrame->SetOutwndViewpoint(this);

	//하드웨어와 연결이 됬는지 체크 해야 한다
	m_HardwareFlag = FALSE;
	m_Header.DataType = OciloscopeStart;

	m_PlotterDlg = NULL;
	m_SelectPlotter = NULL;
	m_Bitmap = NULL;

	m_Hertz = 100;
	m_Frequency = 20;
	m_MaxVoltage = 5;
	m_MinVoltage = -5;
	SendEzSpice();
	CopyDrawImage();
}


void COciloscopeView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void COciloscopeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// COciloscopeView 진단

#ifdef _DEBUG
void COciloscopeView::AssertValid() const
{
	CFormView::AssertValid();
}

void COciloscopeView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

COciloscopeDoc* COciloscopeView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COciloscopeDoc)));
	return (COciloscopeDoc*)m_pDocument;
}



#endif //_DEBUG


// COciloscopeView 메시지 처리기

void COciloscopeView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	dc.SetBkMode(TRANSPARENT);

	if (m_Bitmap != NULL) {

		CRect ClientRect;
		GetClientRect(&ClientRect);

		Gdiplus::Bitmap *mBitmap = m_Bitmap->Clone(0, 0, m_Bitmap->GetWidth(), m_Bitmap->GetHeight(), PixelFormatDontCare);
		Gdiplus::Graphics graphics(dc);
		Gdiplus::Graphics MemDC(mBitmap);

		GetDocument()->Draw(&MemDC, this);
		graphics.DrawImage(mBitmap, 0, 0); 	

		delete mBitmap;
	}
}



LRESULT COciloscopeView::OnRecvEvent(WPARAM wParam, LPARAM lParam)
{
	GetDocument()->RemoveAll();
	if (m_PlotterDlg != NULL) {
		m_PlotterDlg->Reset();
	}

	IPCHeader Header;
	memcpy_s(&Header, sizeof(IPCHeader), theApp.m_pSharedMemory, sizeof(IPCHeader));

	m_Hertz = Header.SecMax;
	SetHertz(m_Hertz);
	m_MaxVoltage = Header.VoltMax;
	m_MinVoltage = -Header.VoltMax;

	m_FrequencySpinValue = Header.SecMax;
	m_VoltageSpinValue = Header.VoltMax;

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CString CurrentString;

	if (m_FrequencySpinValue < 0) m_FrequencySpinValue = 20;
	if (m_VoltageSpinValue < 0) m_VoltageSpinValue = 5;
	
	CurrentString.Format(_T("%d"), m_FrequencySpinValue);
	pFrame->m_pEditIndentLeft->SetEditText(CurrentString);

	CurrentString.Format(_T("%d"), m_VoltageSpinValue);
	pFrame->m_pEditIndentRight->SetEditText(CurrentString);

	switch (Header.DataType) {
		case VoltageData:
			{
				m_SectorList.clear();
				m_IPCDataList.clear();
				for (int iCount = 0; iCount < Header.DataCount; iCount++) {

					IPCSectorHeader NewHeader;
					memcpy_s(&NewHeader, sizeof(IPCSectorHeader)
						, theApp.m_pSharedMemory + sizeof(IPCHeader) + sizeof(IPCSectorHeader)*iCount + sizeof(IPCData)*iCount, sizeof(IPCSectorHeader));
					m_Frequency = NewHeader.Frequency;
					m_SectorList.push_back(NewHeader);

					IPCData NewData;
					memcpy_s(&NewData, sizeof(IPCData), theApp.m_pSharedMemory + sizeof(IPCHeader) + sizeof(IPCSectorHeader)*(iCount +1) + sizeof(IPCData)*iCount, sizeof(IPCData));
					m_IPCDataList.push_back(NewData);
				}

				m_Anniversary = 1000/m_Frequency;
				CopyDrawImage();
				Invalidate(FALSE);
			}
			break;

		default:
			break;
	}
	return 0;
}


void COciloscopeView::SendEzSpice()
{
	if(!UpdateData(TRUE)) {
		return;
	}

	if(theApp.m_Mutex.Lock(1000)) {
		CEvent	EventSend(FALSE, TRUE, TEXT("IPC_READ_SHAREDMEMORY"));
		memcpy_s(theApp.m_pSharedMemory, sizeof(IPCHeader), &m_Header, sizeof(IPCHeader));

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

void COciloscopeView::DrawData( Gdiplus::Graphics* pDC )
{
	Gdiplus::Font font(_T("Arial"), 10.0f, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::White);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	int DataCount = 0;
	list<IPCData>::iterator pos = m_IPCDataList.begin();
	while (pos != m_IPCDataList.end()) {

		Pen RedPen(Color(255, 171, 40, 38), 3.0f);
		switch (m_SectorList[DataCount].DataColor) {
			case darkgreen:
				RedPen.SetColor(Color(255, 74, 186, 26));
				break;

			case darkyellow:
				RedPen.SetColor(Color(255, 200, 130, 35));
				break;

			case darkblue:
				RedPen.SetColor(Color(255, 35, 99, 200));
				break;
		}

		CRect ClientRect;
		GetClientRect(&ClientRect);

		int real_height = ClientRect.Height() - 40;
		double xstep = 0;

		double TotalVolateExtent = fabs(m_MaxVoltage) + fabs(m_MinVoltage);
		double OnePos = (real_height-40)/TotalVolateExtent;
		double RepetitionCount = m_Hertz/m_Anniversary;

		if (m_SectorList[DataCount].Circitcase == linearDC || m_SectorList[DataCount].Circitcase == nonlinearDC) {
			RepetitionCount = 1;
		}

		if (RepetitionCount <= 1) { //한주기보다 작을때

			int MaxCount = (int)(DivideSector*RepetitionCount) -2;
			xstep = (ClientRect.Width() - 140)/double(MaxCount);

			for (int iCount = 0; iCount < MaxCount; iCount++) {
				double Ystep = real_height - (((*pos).GrapehValue[iCount] + fabs(m_MinVoltage)) * OnePos);
				double nextYstep =  real_height - (((*pos).GrapehValue[iCount +1] + fabs(m_MinVoltage)) * OnePos);

				if (Ystep >= 40 && Ystep <= real_height) {
					pDC->DrawLine(&RedPen, PointF((float)(iCount * xstep + 100), (float)Ystep)
						, PointF((float)((iCount + 1) * xstep + 100), (float)nextYstep));
				}
			}

		}
		else {
			int MaxCount = (int)(DivideSector*RepetitionCount) -2;
			xstep = (ClientRect.Width() - 140)/double(MaxCount);
			int ConvertCount = 0;

			int DividedCount = 1;
			for (int iCount = 0; iCount < MaxCount; iCount++) {
				double Ystep = real_height - (((*pos).GrapehValue[iCount -ConvertCount] + fabs(m_MinVoltage)) * OnePos);
				double nextYstep =  real_height - (((*pos).GrapehValue[iCount +1 -ConvertCount] + fabs(m_MinVoltage)) * OnePos);

				if (Ystep >= 40 && Ystep <= real_height) {
					pDC->DrawLine(&RedPen, PointF((float)(iCount * xstep + 100), (float)Ystep)
						, PointF((float)((iCount + 1) * xstep + 100), (float)nextYstep));
				}

				if (iCount != 0 && iCount%(DivideSector-2) == 0) {
					ConvertCount = (DivideSector-2)*DividedCount;
					DividedCount++;
				}
			}
		}
		if ((*pos).GrapehValue[0] != 0) {
			CString PrintString;
			PrintString.Format(_T("%.2fV"), (*pos).GrapehValue[0]);

			double Ystep = real_height - (((*pos).GrapehValue[0] + fabs(m_MinVoltage)) * OnePos);
			pDC->DrawString(PrintString, -1, &font, RectF((float)xstep + 40, float(Ystep-5), 40, 40), &format, &sbrush);
		}

		++DataCount;
		++pos;
	}
}

void COciloscopeView::DrawGrid( Gdiplus::Graphics* pDC )
{
	CRect ClientRect;
	GetClientRect(&ClientRect);

	Pen DefaultPen(Color(180, 255, 255 , 255), 1.0f);
	Pen WhitePen(Color(255, 255, 255, 255),1.0f);
	REAL arDash[]={0.3f, 0.5f, 0.3f, 0.5f}; 
	WhitePen.SetDashStyle(DashStyleCustom);
	WhitePen.SetDashPattern(arDash,sizeof(arDash)/sizeof(arDash[0]));

	int real_width = ClientRect.Width() - 40;
	int real_height = ClientRect.Height() - 40;

	double ystep = (real_height - 40)/double(20);

	// 수평 그리드를 그린다. 
	for(int iCount = 0; iCount < 20; iCount++){		
		if ((iCount+1) %5 == 0) {
			pDC->DrawLine(&DefaultPen, 90, int(real_height -(iCount+ 1) *ystep), real_width, int(real_height -(iCount + 1) *ystep));
		}
		else pDC->DrawLine(&WhitePen, 100, int(real_height -(iCount + 1) *ystep), real_width, int(real_height -(iCount + 1) *ystep));
	}

	// 수직 그리드를 그린다.
	double xstep = (ClientRect.Width() - 140)/double(50);
	for(int iCount = 0; iCount < 50 ; iCount++){	
		if ((iCount+1) %5 == 0) {
			pDC->DrawLine(&DefaultPen, int((iCount + 1) * xstep + 100 ), 40, int((iCount + 1) * xstep + 100), real_height+10);
		}
		pDC->DrawLine(&WhitePen, int((iCount + 1) * xstep + 100 ), 40, int((iCount + 1) * xstep + 100), real_height);
	}
}

void COciloscopeView::DrawAxis( Gdiplus::Graphics* pDC )
{
	CRect ClientRect;
	GetClientRect(&ClientRect);

	Pen WhitePen(Color(255, 255, 255 , 255), 1.0f);

	int real_width = ClientRect.Width(); 
	int real_height = ClientRect.Height();

	pDC->DrawLine(&WhitePen, 80, real_height -40, real_width -20, real_height -40);
	pDC->DrawLine(&WhitePen, 100, 40, 100, real_height -20);
	pDC->DrawLine(&WhitePen, real_width -40, 40, real_width -40, real_height -20);
}

void COciloscopeView::DrawTexts( Gdiplus::Graphics* pDC )
{
	CRect ClientRect;
	GetClientRect(&ClientRect);

	Pen WhitePen(Color(255, 255, 255 , 255), 1.0f);

	Gdiplus::Font font(_T("Arial"), 10.0f, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::White);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	int real_width = ClientRect.Width() - 40;
	int real_height = ClientRect.Height() - 40;

	double ystep = (real_height - 40)/double(20);

	double MinValue = m_MinVoltage, MaxValue = m_MaxVoltage; //임시
	double Tum = (fabs(MinValue) + fabs(MaxValue))/4.0f;


	int TrueCount = 0;
	CString PrintString;
	for (int iCount = 0; iCount <= 20; iCount++) {
		if ( iCount ==0 || iCount%5 == 0) {
			PrintString.Format(_T("%.1lfV"), MinValue + (Tum *TrueCount));
			TrueCount++;
			pDC->DrawString(PrintString, -1, &font, RectF(47, float(real_height - 5 -iCount *ystep), 40, 40), &format, &sbrush);
		}
	}

	Tum = m_Hertz/10.0f;
	TrueCount = 0;
	double xstep = (ClientRect.Width() - 140)/double(50);
	for (int iCount = 0; iCount <=50; iCount++) {
		if ( iCount ==0 || iCount%5 == 0) {
			PrintString.Format(_T("%.1lfms"), Tum *TrueCount);
			TrueCount++;
			pDC->DrawString(PrintString, -1, &font, RectF(float((iCount + 1) * xstep + 55) , float(real_height+20), 40, 40), &format, &sbrush);
		}
	}
}

BOOL COciloscopeView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}


void COciloscopeView::OnDestroy()
{
	m_Header.DataType = OciloscopeEnd;
	SendEzSpice();

	CFormView::OnDestroy();
}


CPlotter* COciloscopeView::CreatePlotter()
{
	CPlotter *NewPlotter = NULL;

	if (GetDocument()->m_PlotterList.size() < 3) {
		NewPlotter = new CPlotter;
	}
	return NewPlotter;
}

void COciloscopeView::CopyDrawImage()
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

	DrawAxis(&MemDC);
	DrawGrid(&MemDC);
	DrawTexts(&MemDC);
	DrawData(&MemDC);

	GetDocument()->MappingObject((float)ClientRect.Width() / (float)CPlotter::m_ClientRect.Width(), ClientRect);
	CPlotter::m_ClientRect = ClientRect;
}

void COciloscopeView::ReDrawPlotterDlg()
{
	if (m_PlotterDlg != NULL) {

		list<PlotterData> SendList;
		list<CPlotter*>::iterator pos = GetDocument()->m_PlotterList.begin();
		while (pos != GetDocument()->m_PlotterList.end()) {

			PlotterData NewData;
			NewData.PlotterName = (*pos)->m_FlotterName;
			NewData.XValue = (*pos)->m_RealXaxis;
			NewData.YValue = (*pos)->m_RealYaxis;
			SendList.push_back(NewData);
			++pos;	
		}

		m_PlotterDlg->SetPlotterData(SendList);
	}
}

void COciloscopeView::SetHertz( double hertz )
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();

	CString SendString;
	SendString.Format(_T("%d"), (int)hertz);
	pFrame->m_pEditIndentLeft->SetEditText(SendString);
}


void COciloscopeView::OnMaxfrequency()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();

	CString CurrentString = pFrame->m_pEditIndentLeft->GetEditText();
	int Value = _ttoi(CurrentString);

	if (abs(Value - m_Hertz) == 1) {
		if (Value > m_Hertz) {
			m_Hertz += 5;
		}
		else if (Value < m_Hertz) {
			m_Hertz -= 5;
		}
	}
	else {
		m_Hertz = Value;
	}

	CurrentString.Format(_T("%d"), (int)m_Hertz);
	pFrame->m_pEditIndentLeft->SetEditText(CurrentString);

	CopyDrawImage();
	Invalidate(FALSE);
}

void COciloscopeView::OnMaxvoltage()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();

	CString CurrentString = pFrame->m_pEditIndentRight->GetEditText();
	m_MaxVoltage = _ttoi(CurrentString);

	m_MinVoltage = -m_MaxVoltage;
	CopyDrawImage();
	Invalidate(FALSE);
}

void COciloscopeView::OnFlotter()
{
	if (m_PlotterDlg!= NULL) {
		m_PlotterDlg->SetFocus();
	}
	else {
		m_PlotterDlg = new CPlotterDlg;
		m_PlotterDlg->Create(IDD_DIALOG1, this);
		m_PlotterDlg->ShowWindow(SW_SHOW);	
		ReDrawPlotterDlg();
	}
}

void COciloscopeView::OnFlottera()
{
	if (m_SectorList.size() >= 1 && GetDocument()->DuplicateObject(CString(_T("A"))) == FALSE) {

		GetClientRect(&CPlotter::m_ClientRect);
		CPlotter *NewFlotterA = CreatePlotter();
		if (NewFlotterA != NULL) {

			NewFlotterA->m_FlotterName = CString(_T("A"));
			SetPlotterInformation(NewFlotterA, 1);
			GetDocument()->m_PlotterList.push_back(NewFlotterA);
		}
		Invalidate(FALSE);
		RedrawWindow(NULL);
	}
}

void COciloscopeView::OnFlotterb()
{
	if (m_SectorList.size() > 1 && GetDocument()->DuplicateObject(CString(_T("B"))) == FALSE) {

		GetClientRect(&CPlotter::m_ClientRect);
		CPlotter *NewFlotterB = CreatePlotter();
		if (NewFlotterB != NULL) {

			NewFlotterB->m_FlotterName = CString(_T("B"));
			SetPlotterInformation(NewFlotterB, 2);
			GetDocument()->m_PlotterList.push_back(NewFlotterB);
		}
		Invalidate(FALSE);
		RedrawWindow(NULL);
	}
}

void COciloscopeView::OnFlotterc()
{
	if (m_SectorList.size() > 2 && GetDocument()->DuplicateObject(CString(_T("C"))) == FALSE) {

		GetClientRect(&CPlotter::m_ClientRect);
		CPlotter *NewFlotterC = CreatePlotter();
		if (NewFlotterC != NULL) {

			NewFlotterC->m_FlotterName = CString(_T("C"));
			SetPlotterInformation(NewFlotterC, 3);
			GetDocument()->m_PlotterList.push_back(NewFlotterC);
		}
		Invalidate(FALSE);
		RedrawWindow(NULL);
	}
}

void COciloscopeView::SetPlotterInformation(CPlotter *NewPlotter, int DataNumber)
{
	int iCount = 0;
	list<IPCData>::iterator pos = m_IPCDataList.begin();
	while (pos != m_IPCDataList.end()) {

		if (iCount == DataNumber -1) {
			NewPlotter->m_GrapehValue = ((*pos).GrapehValue);
			NewPlotter->m_PlotterColor = m_SectorList[iCount].DataColor;
			NewPlotter->m_SectorNumber = iCount;
		}
		++iCount;
		++pos;
	}
}

void COciloscopeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
	case VK_DELETE:
		{
			if (m_SelectPlotter != NULL) {
				GetDocument()->DeleteObject(m_SelectPlotter);
				m_SelectPlotter = NULL;
				Invalidate(FALSE);
			}
		}
		break;
	}

	CFormView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void COciloscopeView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if (m_Bitmap != NULL) {
		CopyDrawImage();
		RedrawWindow(NULL);
	}
}
void COciloscopeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	CPlotter *CurrentPlotter = GetDocument()->IsObject(point);
	if (CurrentPlotter != NULL) {
		
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		ResetEvent(pFrame->m_hEvent);
		m_SelectPlotter = CurrentPlotter;
	}
	CFormView::OnLButtonDown(nFlags, point);
}

void COciloscopeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	if (m_SelectPlotter != NULL) {
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		SetEvent(pFrame->m_hEvent);
		m_SelectPlotter = NULL;
	}
	CFormView::OnLButtonUp(nFlags, point);
}

void COciloscopeView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_SelectPlotter != NULL) {

		if (point.x <= 100 ) {
			m_SelectPlotter->m_XAxis = 100;
		}		
		else if (point.x >= m_SelectPlotter->m_ClientRect.Width() - 40) {
			m_SelectPlotter->m_XAxis = m_SelectPlotter->m_ClientRect.Width() - 40;
		}
		else {
			m_SelectPlotter->m_XAxis = point.x;
		}
		ReDrawPlotterDlg();
		Invalidate(FALSE);
	}
	CFormView::OnMouseMove(nFlags, point);
}



void COciloscopeView::OnReflesh()
{
	CopyDrawImage();
	Invalidate(FALSE);
}
