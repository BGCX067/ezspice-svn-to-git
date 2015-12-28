// �� MFC ���� �ҽ� �ڵ�� MFC Microsoft Office Fluent ����� �������̽�("Fluent UI")�� 
// ����ϴ� ����� ���� �ָ�, MFC C++ ���̺귯�� ����Ʈ��� ���Ե� 
// Microsoft Foundation Classes Reference �� ���� ���� ������ ���� 
// �߰������� �����Ǵ� �����Դϴ�.  
// Fluent UI�� ����, ��� �Ǵ� �����ϴ� �� ���� ��� ����� ������ �����˴ϴ�.  
// Fluent UI ���̼��� ���α׷��� ���� �ڼ��� ������ 
// http://msdn.microsoft.com/officeui�� �����Ͻʽÿ�.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// MainFrm.cpp : CMainFrame Ŭ������ ����
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

// CMainFrame ����/�Ҹ�

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

	// ��� ����� �������̽� ��Ҹ� �׸��� �� ����ϴ� ���־� �����ڸ� �����մϴ�.
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

	// ���־� �����ڿ��� ����ϴ� ���־� ��Ÿ���� �����մϴ�.
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);

	m_wndRibbonBar.Create(this);
	InitializeRibbon();

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("���� ǥ������ ������ ���߽��ϴ�.\n");
		return -1;      // ������ ���߽��ϴ�.
	}

	CString strTitlePane1;
	CString strTitlePane2;
	bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);

	// Visual Studio 2005 ��Ÿ�� ��ŷ â ������ Ȱ��ȭ�մϴ�.
	CDockingManager::SetDockingMode(DT_SMART);
	// Visual Studio 2005 ��Ÿ�� ��ŷ â �ڵ� ���� ������ Ȱ��ȭ�մϴ�.
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// ��ŷ â�� ����ϴ�.
	if (!CreateDockingWindows())
	{
		TRACE0("��ŷ â�� ������ ���߽��ϴ�.\n");
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

	// â �̹����� �ε��մϴ�.
	m_PanelImages.SetImageSize(CSize(16, 16));
	m_PanelImages.Load(IDB_BUTTONS);

	Add_Category1();
}

BOOL CMainFrame::CreateDockingWindows()
{
	// ��� â�� ����ϴ�.

	int yWin = GetSystemMetrics(SM_CYSCREEN);
	if (!m_wndSimualString.Create(_T("�ϵ���� ����"), this, CRect(0, 0, 100, yWin/3), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS 
		| WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI, AFX_CBRS_FLOAT))
	{
		TRACE0("��� â�� ������ ���߽��ϴ�.\n");
		return FALSE; // ������ ���߽��ϴ�.
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
	// "Ŭ������" �г��� ����Ͽ� "Ȩ" ���ָ� �߰��մϴ�.
	bNameValid = strTemp.LoadString(IDS_RIBBON_HOME);
	ASSERT(bNameValid);
	CMFCRibbonCategory* pCategoryHome = m_wndRibbonBar.AddCategory(strTemp, IDB_LARGE, IDB_LARGE);

	// "����" �г��� ����ϴ�.
	CMFCRibbonPanel* pPanelView = pCategoryHome->AddPanel(_T("����"), m_PanelImages.ExtractIcon(27));
	CMFCRibbonButton* pBtnZoomIn = new CMFCRibbonButton(ID_ZOOMIN, _T("Ȯ��"), 14, 14);
	pPanelView->Add(pBtnZoomIn);
	CMFCRibbonButton* pBtnZoomOut = new CMFCRibbonButton(ID_ZOOMOUT, _T("���"), 15, 15);
	pPanelView->Add(pBtnZoomOut);
	CMFCRibbonButton* pBtnReflesh = new CMFCRibbonButton(ID_REFLESH, _T("���÷���"), 19, 19);
	pPanelView->Add(pBtnReflesh);

	//"�󼼺���" �г��� ����ϴ�
	CMFCRibbonPanel* pPanelDetailView = pCategoryHome->AddPanel(_T("�󼼺���"), m_PanelImages.ExtractIcon(27));
	CMFCRibbonButton* pBtnFlotter = new CMFCRibbonButton(ID_FLOTTER, _T("�÷���"), 21, 21);
	pBtnFlotter->SetMenu(IDR_FLOTTER_MENU, TRUE);
	pPanelDetailView->Add(pBtnFlotter);

	// "����" �г��� ����� �߰��մϴ�.
	CMFCRibbonPanel* pPanelSetting = pCategoryHome->AddPanel(_T("����"), m_PanelImages.ExtractIcon (7));
	bNameValid = strTemp.LoadString(IDS_RIBBON_STATUSBAR);
	ASSERT(bNameValid);
	CMFCRibbonButton* pBtnStatusBar = new CMFCRibbonCheckBox(ID_VIEW_STATUS_BAR, strTemp);
	pPanelSetting->Add(pBtnStatusBar);

	CMFCRibbonPanel* pPanelChange = pCategoryHome->AddPanel(_T("����"), m_PanelImages.ExtractIcon (7));
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

	CMFCRibbonPanel* pPanelHardware = pCategoryHome->AddPanel(_T("���"), m_PanelImages.ExtractIcon (7));
	m_pBtnHardwareConnect = new CMFCRibbonButton(ID_HARDWARE_CONNECT, _T("����"), 22, 22);
	CMFCRibbonButton* pBtnHardware2 = new CMFCRibbonButton(ID_HARDWARE_OCIL, _T("���Ƿν�����"), 7, 7);
	pBtnHardware2->SetMenu(IDR_OCLIL_MENU, TRUE);

	CMFCRibbonButton* pBtnHardware3 = new CMFCRibbonButton(ID_HARDWARE_DISIT, _T("������ ��Ƽ����"), 1, 1);
	CMFCRibbonButton* pBtnHardware4 = new CMFCRibbonButton(ID_HARDWARE_FUNC, _T("�Լ��߻���"), 4, 4);
	CMFCRibbonButton* pBtnHardware5 = new CMFCRibbonButton(ID_HARDWARE_POWER, _T("�Ŀ����ö���"), 9, 9);
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


// CMainFrame ����

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


// CMainFrame �޽��� ó����

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

				if (Mode == 1) {	//���Ƿ� ������
					double ConvertData = CurrentData;
					ConvertData /= 100;
					ConvertData -= 13;

					if (ConvertData >= -13 && ConvertData <= 13) {
						ConvertData *= pFrame->m_AmplificationRate;
						pFrame->m_wndSimualString.SetGraphData(ConvertData);
					}
					
				}
				else {	//��Ƽ���� ������
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
			AfxMessageBox(_T("USB ������ Ȯ���Ͻʽÿ�"));
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
			m_pBtnHardwareConnect->SetText(_T("����"));
			m_HardWareFlag = !m_HardWareFlag;
			AfxMessageBox(_T("��ġ�� ������ �Ǿ����ϴ�."));
		}
	}
	else {
		ResetEvent(m_hEvent);
		USB_HID_Destory();
		m_pBtnHardwareConnect->SetText(_T("����"));
		m_HardWareFlag = !m_HardWareFlag;
		AfxMessageBox(_T("��ġ�� ������ ���� �Ǿ����ϴ�."));
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
