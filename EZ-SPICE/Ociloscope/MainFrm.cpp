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

// MainFrm.cpp : CMainFrame 클래스의 구현
//

#include "stdafx.h"
#include "Ociloscope.h"

#include "MainFrm.h"
//#include <mat.h>
#include "Amplification.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_MESSAGE(UM_RECV_EVENT, &CMainFrame::OnRecvEvent)
	ON_COMMAND(ID_HARDWARE_OCIL, &CMainFrame::OnHardwareOcil)
	ON_WM_DESTROY()
	ON_COMMAND(ID_HARDWARE_CONNECT, &CMainFrame::OnHardwareConnect)
	ON_COMMAND(ID_HARDWARE_DISIT, &CMainFrame::OnHardwareDisit)
	ON_COMMAND(ID_HARDWARE_POWER, &CMainFrame::OnHardwarePower)
	ON_COMMAND(ID_HARDWARE_FUNC, &CMainFrame::OnHardwareFunc)
	ON_COMMAND(ID_AMPLI, &CMainFrame::OnAmpli)
END_MESSAGE_MAP()

// CMainFrame 생성/소멸

CMainFrame::CMainFrame()
{
	m_HardWareFlag = FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	// 모든 사용자 인터페이스 요소를 그리는 데 사용하는 비주얼 관리자를 설정합니다.
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

	// 비주얼 관리자에서 사용하는 비주얼 스타일을 설정합니다.
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);

	m_wndRibbonBar.Create(this);
	InitializeRibbon();

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}

	CString strTitlePane1;
	CString strTitlePane2;
	bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);

	// Visual Studio 2005 스타일 도킹 창 동작을 활성화합니다.
	CDockingManager::SetDockingMode(DT_SMART);
	// Visual Studio 2005 스타일 도킹 창 자동 숨김 동작을 활성화합니다.
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// 도킹 창을 만듭니다.
	if (!CreateDockingWindows())
	{
		TRACE0("도킹 창을 만들지 못했습니다.\n");
		return -1;
	}

	m_wndSimualString.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndSimualString);
	m_wndSimualString.SetAutoHideMode(TRUE, CBRS_ALIGN_BOTTOM);

	m_SummonsSpeed = 100;
	m_ExitThread = FALSE;
	m_HIDhandle = NULL;
	m_HardWareOciloscopeFlag = FALSE;
	m_pFunctionDlg = FALSE;
	m_pMultimeterDlg = NULL;
	m_pPowerSupplyDlg = NULL;
	m_USBFlag = FALSE;
	m_AmplificationRate = 10;


	m_hEvent = CreateEvent(NULL, TRUE, FALSE, _T("DECIDE_COMPACT_EVENT"));
	m_UpdataThread = AfxBeginThread(UpdateInfoThreadFunc, this);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;

	cs.cx = ::GetSystemMetrics(SM_CXMAXIMIZED)/5*3;
	cs.cy = ::GetSystemMetrics(SM_CYMAXIMIZED)/5*4;

	cs.style &= ~FWS_ADDTOTITLE;
	this->SetTitle(_T("Ociloscope"));

	return TRUE;
}

void CMainFrame::InitializeRibbon()
{
	BOOL bNameValid;

	CString strTemp;
	bNameValid = strTemp.LoadString(IDS_RIBBON_FILE);
	ASSERT(bNameValid);

	// 창 이미지를 로드합니다.
	m_PanelImages.SetImageSize(CSize(16, 16));
	m_PanelImages.Load(IDB_BUTTONS);

	Add_Category1();
}

BOOL CMainFrame::CreateDockingWindows()
{
	// 출력 창을 만듭니다.

	int yWin = GetSystemMetrics(SM_CYSCREEN);
	if (!m_wndSimualString.Create(_T("하드웨어 연동"), this, CRect(0, 0, 100, yWin/3), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS 
		| WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI, AFX_CBRS_FLOAT))
	{
		TRACE0("출력 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndSimualString.SetIcon(hOutputBarIcon, FALSE);

}


LRESULT CMainFrame::OnRecvEvent(WPARAM wParam, LPARAM lParam)
{
	GetActiveView()->PostMessage(UM_RECV_EVENT);
	return 0;
}



void CMainFrame::Add_Category1()
{
	BOOL bNameValid;
	CString strTemp;
	// "클립보드" 패널을 사용하여 "홈" 범주를 추가합니다.
	bNameValid = strTemp.LoadString(IDS_RIBBON_HOME);
	ASSERT(bNameValid);
	CMFCRibbonCategory* pCategoryHome = m_wndRibbonBar.AddCategory(strTemp, IDB_LARGE, IDB_LARGE);

	// "보기" 패널을 만듭니다.
	CMFCRibbonPanel* pPanelView = pCategoryHome->AddPanel(_T("보기"), m_PanelImages.ExtractIcon(27));
	CMFCRibbonButton* pBtnZoomIn = new CMFCRibbonButton(ID_ZOOMIN, _T("확대"), 14, 14);
	pPanelView->Add(pBtnZoomIn);
	CMFCRibbonButton* pBtnZoomOut = new CMFCRibbonButton(ID_ZOOMOUT, _T("축소"), 15, 15);
	pPanelView->Add(pBtnZoomOut);
	CMFCRibbonButton* pBtnReflesh = new CMFCRibbonButton(ID_REFLESH, _T("리플래쉬"), 19, 19);
	pPanelView->Add(pBtnReflesh);

	//"상세보기" 패널을 만듭니다
	CMFCRibbonPanel* pPanelDetailView = pCategoryHome->AddPanel(_T("상세보기"), m_PanelImages.ExtractIcon(27));
	CMFCRibbonButton* pBtnFlotter = new CMFCRibbonButton(ID_FLOTTER, _T("플로터"), 21, 21);
	pBtnFlotter->SetMenu(IDR_FLOTTER_MENU, TRUE);
	pPanelDetailView->Add(pBtnFlotter);

	// "설정" 패널을 만들고 추가합니다.
	CMFCRibbonPanel* pPanelSetting = pCategoryHome->AddPanel(_T("설정"), m_PanelImages.ExtractIcon (7));
	bNameValid = strTemp.LoadString(IDS_RIBBON_STATUSBAR);
	ASSERT(bNameValid);
	CMFCRibbonButton* pBtnStatusBar = new CMFCRibbonCheckBox(ID_VIEW_STATUS_BAR, strTemp);
	pPanelSetting->Add(pBtnStatusBar);

	CMFCRibbonPanel* pPanelChange = pCategoryHome->AddPanel(_T("변경"), m_PanelImages.ExtractIcon (7));
	pPanelChange->Add(new CMFCRibbonLabel(_T("Indent")));
	m_pEditIndentLeft = new CMFCRibbonEdit(ID_MAXFREQUENCY, 72, _T("Hez : \nil"), -1);
	m_pEditIndentLeft->EnableSpinButtons(1, 500);

	m_pEditIndentLeft->SetEditText(_T("0"));
	pPanelChange->Add(m_pEditIndentLeft);

	m_pEditIndentRight = new CMFCRibbonEdit(ID_MAXVOLTAGE, 72, _T("Volt : \nir"), -1);
	m_pEditIndentRight->EnableSpinButtons(1, 10);

	m_pEditIndentRight->SetEditText(_T("1"));
	pPanelChange->Add(m_pEditIndentRight);
	pPanelChange->SetJustifyColumns();

	CMFCRibbonPanel* pPanelHardware = pCategoryHome->AddPanel(_T("장비"), m_PanelImages.ExtractIcon (7));
	m_pBtnHardwareConnect = new CMFCRibbonButton(ID_HARDWARE_CONNECT, _T("연결"), 22, 22);
	CMFCRibbonButton* pBtnHardware2 = new CMFCRibbonButton(ID_HARDWARE_OCIL, _T("오실로스코프"), 7, 7);
	pBtnHardware2->SetMenu(IDR_OCLIL_MENU, TRUE);

	CMFCRibbonButton* pBtnHardware3 = new CMFCRibbonButton(ID_HARDWARE_DISIT, _T("디지털 멀티미터"), 1, 1);
	CMFCRibbonButton* pBtnHardware4 = new CMFCRibbonButton(ID_HARDWARE_FUNC, _T("함수발생기"), 4, 4);
	CMFCRibbonButton* pBtnHardware5 = new CMFCRibbonButton(ID_HARDWARE_POWER, _T("파워서플라이"), 9, 9);
	pPanelHardware->Add(m_pBtnHardwareConnect);
	pPanelHardware->Add(pBtnHardware2);
	pPanelHardware->Add(pBtnHardware3);
	pPanelHardware->Add(pBtnHardware4);
	pPanelHardware->Add(pBtnHardware5);
}

void CMainFrame::SetOutwndViewpoint( COciloscopeView *pView )
{
	m_wndSimualString.SetView(pView);
}


// CMainFrame 진단

#ifdef _DEBUG

void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}


#endif //_DEBUG


// CMainFrame 메시지 처리기

void CMainFrame::OnHardwareOcil()
{
	if (!m_HardWareOciloscopeFlag) {
		m_wndSimualString.SetAutoHideMode(FALSE, CBRS_ALIGN_BOTTOM);	
		m_HardWareOciloscopeFlag = !m_HardWareOciloscopeFlag;
		USB_HID_Sendreport(OCIL_REPOERID, (char)TRUE, 0, 0);
	}
	else {
		m_wndSimualString.SetAutoHideMode(TRUE, CBRS_ALIGN_BOTTOM);	
		m_HardWareOciloscopeFlag = !m_HardWareOciloscopeFlag;
		USB_HID_Sendreport(OCIL_REPOERID, (char)FALSE, 0, 0);
	}
	GetActiveView()->SendMessage(UM_VIEW_REDRAW);
}


UINT CMainFrame::UpdateInfoThreadFunc(LPVOID pParam)
{
	CMainFrame *pFrame = (CMainFrame*)pParam;

	int res = 0;
	static int i = 0;	//debug
	while (!pFrame->m_ExitThread) {

		WaitForSingleObject(pFrame->m_hEvent, INFINITE);

		if (pFrame->m_USBFlag) {
			res = hid_read(pFrame->m_HIDhandle, pFrame->m_HIDBuffer, HID_BUFFERSIZE);

			//test
			if (res > 0) {

				int Mode = pFrame->m_HIDBuffer[1];
				int CurrentData = 0;
				((char*)&CurrentData)[1] = pFrame->m_HIDBuffer[2];
				((char*)&CurrentData)[0] = pFrame->m_HIDBuffer[3];

				if (Mode == 1) {	//오실로 데이터
					double ConvertData = CurrentData;
					ConvertData /= 100;
					ConvertData -= 13;

					if (ConvertData >= -13 && ConvertData <= 13) {
						ConvertData *= pFrame->m_AmplificationRate;
						pFrame->m_wndSimualString.SetGraphData(ConvertData);
					}
					
				}
				else {	//멀티미터 데이터
					if (pFrame->m_pMultimeterDlg != NULL) {
						pFrame->m_pMultimeterDlg->SetMultimeterData(Mode, CurrentData);
					}
				}
			}
		}
		Sleep(10);
	}
	return 0;
}
void CMainFrame::OnDestroy()
{
	m_ExitThread = TRUE;
	SetEvent(m_hEvent);
	Sleep(1000);

	if (m_HardWareFlag) {
		USB_HID_Destory();
	}

	hid_exit();
	WaitForSingleObject(m_UpdataThread, INFINITE);

	CFrameWndEx::OnDestroy();
}

BOOL CMainFrame::USB_HID_Create()
{
	if (m_HIDhandle == NULL) {
		m_HIDhandle = hid_open(HID_VID, HID_PID, NULL);
		if (!m_HIDhandle) {
			AfxMessageBox(_T("USB 연결을 확인하십시오"));
			return FALSE;
		}

		m_USBFlag = TRUE;

		hid_set_nonblocking(m_HIDhandle, 1);
		int res = hid_read(m_HIDhandle, m_HIDBuffer, HID_BUFFERSIZE);

		memset(m_HIDBuffer, 0, sizeof(m_HIDBuffer));
	}
	return TRUE;
}

void CMainFrame::USB_HID_Destory()
{
	if (m_HIDhandle != NULL) {
		hid_close(m_HIDhandle);
		m_HIDhandle = NULL;
		m_USBFlag = FALSE;
	}
}

void CMainFrame::USB_HID_Sendreport(int reportID, char Mode, char HighByte, char LowByte)
{
	m_HIDBuffer[0] = reportID;
	m_HIDBuffer[1] = Mode;
	m_HIDBuffer[2] = HighByte;
	m_HIDBuffer[3] = LowByte;

	if (m_HIDhandle != NULL) {
		int res = hid_write(m_HIDhandle, m_HIDBuffer, HID_BUFFERSIZE);
		if (res < 0) {
			AfxMessageBox(_T("Unable to write"));
		}
	}	
}


void CMainFrame::OnHardwareConnect()
{
	if (!m_HardWareFlag) {
		if (USB_HID_Create()) {
			SetEvent(m_hEvent);
			m_pBtnHardwareConnect->SetText(_T("해제"));
			m_HardWareFlag = !m_HardWareFlag;
			AfxMessageBox(_T("장치와 연결이 되었습니다."));
		}
	}
	else {
		ResetEvent(m_hEvent);
		USB_HID_Destory();
		m_pBtnHardwareConnect->SetText(_T("연결"));
		m_HardWareFlag = !m_HardWareFlag;
		AfxMessageBox(_T("장치와 연결이 해제 되었습니다."));
	}
}

void CMainFrame::OnHardwareDisit()
{
	if (m_pMultimeterDlg != NULL) {
		m_pMultimeterDlg->SetFocus();
	}
	else {
		m_pMultimeterDlg = new CDigitalMultimeter;
		m_pMultimeterDlg->Create(IDD_MULTMETER, this);
		m_pMultimeterDlg->ShowWindow(SW_SHOW);		
	}
}

void CMainFrame::OnHardwarePower()
{
	if (m_pPowerSupplyDlg != NULL) {
		m_pPowerSupplyDlg->SetFocus();
	}
	else {
		m_pPowerSupplyDlg = new CPowerSupply;
		m_pPowerSupplyDlg->Create(IDD_POWER, this);
		m_pPowerSupplyDlg->ShowWindow(SW_SHOW);		
	}
}

void CMainFrame::OnHardwareFunc()
{
	if (m_pFunctionDlg != NULL) {
		m_pFunctionDlg->SetFocus();
	}
	else {
		m_pFunctionDlg = new CFunctionGenerator;
		m_pFunctionDlg->Create(IDD_FUNC, this);
		m_pFunctionDlg->ShowWindow(SW_SHOW);		
	}
}

void CMainFrame::OnAmpli()
{
	CAmplification Dlg;
	Dlg.m_RateValue = m_AmplificationRate;
	if (Dlg.DoModal() == IDOK) {
		if (Dlg.m_RateValue > 0 && Dlg.m_RateValue <= 10) {
			m_AmplificationRate = Dlg.m_RateValue;
		}
	}
}
