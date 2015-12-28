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
#include "EzSpice.h"

#include "MainFrm.h"
#include "ElementFactory.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include <fstream>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_USER_NOTEPED, &CMainFrame::OnUserNoteped)
	ON_COMMAND(ID_USER_SENDMAIL, &CMainFrame::OnUserSendmail)
	ON_COMMAND(ID_USER_VISITSITE, &CMainFrame::OnUserVisitsite)
	ON_COMMAND(ID_USER_DOCKING_ELETREEVIEW, &CMainFrame::OnUserDockingEletreeview)
	ON_COMMAND(ID_USER_DOCKING_ELEVIEW, &CMainFrame::OnUserDockingEleview)
	ON_COMMAND(ID_USER_DOCKING_OUTPUTVIEW, &CMainFrame::OnUserDockingOutputview)
	ON_COMMAND(ID_USER_ELEMENT_EDITDLG, &CMainFrame::OnUserElementEditdlg)
	ON_COMMAND(ID_USER_SEARCH_DIRECT, &CMainFrame::OnUserSearchDirect)
	ON_WM_CLOSE()
	ON_MESSAGE(UM_RECV_EVENT, &CMainFrame::OnRecvEvent)
	
	ON_COMMAND(ID_EDIT_TRANSIENT, &CMainFrame::OnEditTransient)
	ON_COMMAND(ID_SIMUAL_SETTING, &CMainFrame::OnSimualSetting)
	ON_COMMAND(ID_USER_MATH, &CMainFrame::OnUserMath)
END_MESSAGE_MAP()

// CMainFrame ����/�Ҹ�

//���� ������
CElementFactory	*pElementFactory = NULL;

// enum ElementKind {
// 	lines, dcpower, acpower, resistance, ground, inductor, capacity, transistor, prov, diode, leq, geq,
// 
// };

CMainFrame::CMainFrame()
{
	m_pElementEditDlg = NULL;
	m_pTransientDlg = NULL;
	m_pNetworkTheroyDlg = NULL;
	m_pFormuladlg = NULL;
	m_SecMax = 20;
	m_VoltMax = 5;
}

CMainFrame::~CMainFrame()
{
	if (pElementFactory != NULL) {
		delete pElementFactory;
	}
}

CElementFactory* CMainFrame::CreateElementFactory()
{
	if (pElementFactory != NULL) {
		return pElementFactory;
	}

	pElementFactory = new CElementFactory;
	return pElementFactory;	
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	// ��� ����� �������̽� ��Ҹ� �׸��� �� ����ϴ� ���־� �����ڸ� �����մϴ�.
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

	// ���־� �����ڿ��� ����ϴ� ���־� ��Ÿ���� �����մϴ�.
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // ����� �� �ִ� �ٸ� ��Ÿ��...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // FALSE�� �����Ͽ� �� ���� �����ʿ� �ݱ� ���߸� ��ġ�մϴ�.
	mdiTabParams.m_bTabIcons = FALSE;    // TRUE�� �����Ͽ� MDI ���� ���� �������� Ȱ��ȭ�մϴ�.
	mdiTabParams.m_bAutoColor = TRUE;    // FALSE�� �����Ͽ� MDI ���� �ڵ� �� ������ ��Ȱ��ȭ�մϴ�.
	mdiTabParams.m_bDocumentMenu = TRUE; // �� ������ ������ �����ڸ��� ���� �޴��� Ȱ��ȭ�մϴ�.
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	//���� �� ������ �ʱ�ȭ 
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

	// �޴� �׸� �̹����� �ε��մϴ�(ǥ�� ���� ������ ����).
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// ��ŷ â�� ����ϴ�.
	if (!CreateDockingWindows())
	{
		TRACE0("��ŷ â�� ������ ���߽��ϴ�.\n");
		return -1;
	}

	CDockablePane* pTabbedBar = NULL;
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
	m_wndClassView.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndClassView);


	// ���� â ���� ��ȭ ���ڸ� Ȱ��ȭ�մϴ�.
	EnableWindowsDialog(ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);
	InitializeElementInfo();
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;

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

	Add_MainPanel();
	Add_Category1();
	Add_Category2();
	Add_Category3();
	Add_Qat();
	m_wndRibbonBar.AddToTabs(new CMFCRibbonButton(ID_APP_ABOUT, _T("\na"), m_PanelImages.ExtractIcon (0)));
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;

	// ���� Ʈ�� �並 ����ϴ�.
	CString strClassView;
	bNameValid = strClassView.LoadString(IDS_CLASS_VIEW);
	ASSERT(bNameValid);
	if (!m_wndClassView.Create(strClassView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CLASSVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Ŭ���� �� â�� ������ ���߽��ϴ�.\n");
		return FALSE; // ������ ���߽��ϴ�.
	}

	// ��� â�� ����ϴ�.
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("��� â�� ������ ���߽��ϴ�.\n");
		return FALSE; // ������ ���߽��ϴ�.
	}


	//���� �並 ����ϴ�.
	CTabbedPane *pTabbedPane = new CTabbedPane(TRUE); //TRUE�ָ� �ڵ����� �޸� ���� ��

	if (!pTabbedPane->Create(_T(""), this, CRect(0, 0, 250, 250), TRUE, (UINT)-1,  // 5��° ���ڰ� ��ŷ ���� ID (�ݵ�� -1 �ؾ� �Ѵ�. p839����)
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI)) {
			return -1;
	}

	m_wndElementView.CreateEx(NULL, _T("����"), this, CRect(0, 0, 0, 0), TRUE, 1234,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI);

	m_wndElementView.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndElementView);

	pTabbedPane->AddTab(&m_wndElementView);
	pTabbedPane->EnableDocking(CBRS_ALIGN_ANY);
	DockPane(pTabbedPane);

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndClassView.SetIcon(hClassViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();
}

// CMainFrame ����

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame �޽��� ó����

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}



////////////////////////////////////////////////////////////////////////// �г� ����
void CMainFrame::Add_MainPanel() 
{
	BOOL bNameValid;
	CString strTemp;

	// �ʱ�ȭ �� �����Դϴ�.
	m_MainButton.SetImage(IDB_EZMAIN); 
	m_MainButton.SetText(_T("\nf"));	//����Ű ���
	m_MainButton.SetToolTipText(strTemp);

	m_wndRibbonBar.SetApplicationButton(&m_MainButton, CSize (45, 45)); //CMFCRibbonApplicationButton Ŭ���� ��ü�� m_MainButton ����� ���� ��ư���� ����
	CMFCRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory(strTemp, IDB_FILESMALL, IDB_FILELARGE);  //���� �г� ����

	bNameValid = strTemp.LoadString(IDS_RIBBON_NEW);
	ASSERT(bNameValid);
	pMainPanel->Add(new CMFCRibbonButton(ID_FILE_NEW, strTemp, 0, 0));
	bNameValid = strTemp.LoadString(IDS_RIBBON_OPEN);
	ASSERT(bNameValid);
	pMainPanel->Add(new CMFCRibbonButton(ID_FILE_OPEN, strTemp, 1, 1));
	bNameValid = strTemp.LoadString(IDS_RIBBON_SAVE);
	ASSERT(bNameValid);
	pMainPanel->Add(new CMFCRibbonButton(ID_FILE_SAVE, strTemp, 2, 2));
	bNameValid = strTemp.LoadString(IDS_RIBBON_SAVEAS);
	ASSERT(bNameValid);
	pMainPanel->Add(new CMFCRibbonButton(ID_FILE_SAVE_AS, strTemp, 3, 3));

	bNameValid = strTemp.LoadString(IDS_RIBBON_PRINT);
	ASSERT(bNameValid);
	CMFCRibbonButton* pBtnPrint = new CMFCRibbonButton(ID_FILE_PRINT, strTemp, 6, 6);
	pBtnPrint->SetKeys(_T("p"), _T("w"));
	bNameValid = strTemp.LoadString(IDS_RIBBON_PRINT_LABEL);
	ASSERT(bNameValid);
	pBtnPrint->AddSubItem(new CMFCRibbonLabel(strTemp));
	bNameValid = strTemp.LoadString(IDS_RIBBON_PRINT_QUICK);
	ASSERT(bNameValid);
	pBtnPrint->AddSubItem(new CMFCRibbonButton(ID_FILE_PRINT_DIRECT, strTemp, 7, 7, TRUE));
	bNameValid = strTemp.LoadString(IDS_RIBBON_PRINT_PREVIEW);
	ASSERT(bNameValid);
	pBtnPrint->AddSubItem(new CMFCRibbonButton(ID_FILE_PRINT_PREVIEW, strTemp, 8, 8, TRUE));
	bNameValid = strTemp.LoadString(IDS_RIBBON_PRINT_SETUP);
	ASSERT(bNameValid);
	pBtnPrint->AddSubItem(new CMFCRibbonButton(ID_FILE_PRINT_SETUP, strTemp, 11, 11, TRUE));
	pMainPanel->Add(pBtnPrint);
	pMainPanel->Add(new CMFCRibbonSeparator(TRUE));

	bNameValid = strTemp.LoadString(IDS_RIBBON_CLOSE);
	ASSERT(bNameValid);
	pMainPanel->Add(new CMFCRibbonButton(ID_FILE_CLOSE, strTemp, 9, 9));

	bNameValid = strTemp.LoadString(IDS_RIBBON_RECENT_DOCS);
	ASSERT(bNameValid);
	pMainPanel->AddRecentFilesList(strTemp);

	bNameValid = strTemp.LoadString(IDS_RIBBON_EXIT);
	ASSERT(bNameValid);
	pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_EXIT, strTemp, 15));
}

void CMainFrame::Add_Qat()
{
	CList<UINT, UINT> lstQATCmds;

	lstQATCmds.AddTail(ID_FILE_NEW);
	lstQATCmds.AddTail(ID_FILE_OPEN);
	lstQATCmds.AddTail(ID_FILE_SAVE);
	lstQATCmds.AddTail(ID_FILE_PRINT_DIRECT);

	m_wndRibbonBar.SetQuickAccessCommands(lstQATCmds);
}

void CMainFrame::Add_Category1()
{
	BOOL bNameValid;
	CString strTemp;

	// "Ŭ������" �г��� ����Ͽ� "Ȩ" ���ָ� �߰��մϴ�.
	bNameValid = strTemp.LoadString(IDS_RIBBON_HOME);
	ASSERT(bNameValid);
	CMFCRibbonCategory* pCategoryHome = m_wndRibbonBar.AddCategory(strTemp, IDB_SMALL, IDB_MLARGE);

	
	// "Ŭ������" �г��� ����ϴ�.
	bNameValid = strTemp.LoadString(IDS_RIBBON_CLIPBOARD);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelClipboard = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(27));

	bNameValid = strTemp.LoadString(IDS_RIBBON_PASTE);
	ASSERT(bNameValid);
	CMFCRibbonButton* pBtnPaste = new CMFCRibbonButton(ID_EDIT_PASTE, strTemp, -1, 16);
	pPanelClipboard->Add(pBtnPaste);

	bNameValid = strTemp.LoadString(IDS_RIBBON_CUT);
	ASSERT(bNameValid);
	pPanelClipboard->Add(new CMFCRibbonButton(ID_EDIT_CUT, strTemp, 17));

	bNameValid = strTemp.LoadString(IDS_RIBBON_COPY);
	ASSERT(bNameValid);
	pPanelClipboard->Add(new CMFCRibbonButton(ID_EDIT_COPY, strTemp, 18));

	bNameValid = strTemp.LoadString(IDS_RIBBON_SELECTALL);
	ASSERT(bNameValid);
	pPanelClipboard->Add(new CMFCRibbonButton(ID_EDIT_SELECT_ALL, strTemp, 19));


	//���� �г�
	bNameValid = strTemp.LoadString(IDS_RIBBON_STATE);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelState = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(7));

	bNameValid = strTemp.LoadString(IDS_RIBBON_UNDO);
	ASSERT(bNameValid);
	pPanelState->Add(new CMFCRibbonButton(ID_USER_STATE_UNDO, strTemp, 13, 13)); 
	
	bNameValid = strTemp.LoadString(IDS_RIBBON_REDO);
	ASSERT(bNameValid);
	pPanelState->Add(new CMFCRibbonButton(ID_USER_STATE_REDO, strTemp, 10, 10)); 

	pPanelState->Add(new CMFCRibbonSeparator(FALSE));

	bNameValid = strTemp.LoadString(IDS_RIBBON_INCREASE);
	ASSERT(bNameValid);
	pPanelState->Add(new CMFCRibbonButton(ID_USER_STATE_INCREASE, strTemp, 14, 14)); 

	bNameValid = strTemp.LoadString(IDS_RIBBON_DECREASE);
	ASSERT(bNameValid);
	pPanelState->Add(new CMFCRibbonButton(ID_USER_STATE_DECREASE, strTemp, 15, 15)); 

	pPanelState->Add(new CMFCRibbonButton(ID_JUNCTION, _T("������"), -1, 18));


	//���� �˻� �г�
	bNameValid = strTemp.LoadString(IDS_RIBBON_NEWELEMENT);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelSearch = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(7));
	pPanelSearch->Add(new CMFCRibbonButton(ID_EDIT_CUT, _T(""), -1)); // �̳��� �ƿ� ���־� �ϴϱ� �� UserMessage�ϳ� �Ҵ��ϱ� 

	bNameValid = strTemp.LoadString(IDS_RIBBON_DIRECTSEARCH);
	ASSERT(bNameValid);
	m_DirectSearch = new CMFCRibbonComboBox(ID_USER_SEARCH_DIRECT, TRUE, 150, _T("NAME:"), 2);

	pPanelSearch->Add(m_DirectSearch);

	bNameValid = strTemp.LoadString(IDS_RIBBON_DETAILSEARCH);
	ASSERT(bNameValid);
	pPanelSearch->Add(new CMFCRibbonButton(ID_USER_SRARCH_DETAIL, strTemp, 0, 0));

	//�ùķ��̼� �г�

	bNameValid = strTemp.LoadString(IDS_RIBBON_SIMULATION);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelSimulation = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(7));

	bNameValid = strTemp.LoadString(IDS_RIBBON_OPERATION);
	ASSERT(bNameValid);

	CMFCRibbonButton *pSimualButton = new CMFCRibbonButton(ID_USER_SIMUL_START, strTemp, 11, 11);
	pSimualButton->SetMenu(IDR_SIMUAL_MENU, TRUE);
	pPanelSimulation->Add(pSimualButton);

	bNameValid = strTemp.LoadString(IDS_RIBBON_SINGLEPROV);
	ASSERT(bNameValid);
	pPanelSimulation->Add(new CMFCRibbonButton(ID_USER_SIMUL_PROV1, strTemp, 12, 12)); 

	bNameValid = strTemp.LoadString(IDS_RIBBON_MULTPROV);
	ASSERT(bNameValid);
	pPanelSimulation->Add(new CMFCRibbonButton(ID_USER_SIMUL_PROV2, strTemp, 5, 5)); 


	//���� ��� �г�

	bNameValid = strTemp.LoadString(IDS_RIBBON_VIRTUALDEVICE);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelVirtual = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(7));

	bNameValid = strTemp.LoadString(IDS_RIBBON_OSCILOSCOPE);
	ASSERT(bNameValid);
	pPanelVirtual->Add(new CMFCRibbonButton(ID_USER_DEVICE_OSCIL, strTemp, 7, 7)); 

	bNameValid = strTemp.LoadString(IDS_RIBBON_MULTIMETER);
	ASSERT(bNameValid);
	pPanelVirtual->Add(new CMFCRibbonButton(ID_USER_DEVICE_DISIT, strTemp, 1, 1)); 

	bNameValid = strTemp.LoadString(IDS_RIBBON_FUNCTION);
	ASSERT(bNameValid);
	pPanelVirtual->Add(new CMFCRibbonButton(ID_USER_DEVICE_FUNC, strTemp, 4, 4)); 

	bNameValid = strTemp.LoadString(IDS_RIBBON_POWERSUPPLY);
	ASSERT(bNameValid);
	pPanelVirtual->Add(new CMFCRibbonButton(ID_USER_DEVICE_POWER, strTemp, 9, 9));


	CMFCRibbonPanel* pPanelMath = pCategoryHome->AddPanel(_T("����"), m_PanelImages.ExtractIcon(7));
	CMFCRibbonButton* pBtnMath = new CMFCRibbonButton(ID_USER_MATH, _T("���ĺ���"), 20, 20);
	pPanelMath->Add(pBtnMath);
	

	//��ũ �г�

	bNameValid = strTemp.LoadString(IDS_RIBBON_LINK);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelLink = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(7));

	pPanelLink->Add(new CMFCRibbonLinkCtrl(ID_USER_SENDMAIL, _T("Send e-mail"), _T("mailto:njuhb@naver.com")));
	pPanelLink->Add(new CMFCRibbonLinkCtrl(ID_USER_VISITSITE, _T("Data Sheet"), _T("http://www.alldatasheet.co.kr")));
	pPanelLink->Add(new CMFCRibbonLinkCtrl(ID_USER_NOTEPED, _T("Launch Notepad"), _T("notepad")));
}

void CMainFrame::Add_Category2()
{
	BOOL bNameValid;
	CString strTemp;

	// "Ŭ������" �г��� ����Ͽ� "Ȩ" ���ָ� �߰��մϴ�.
	bNameValid = strTemp.LoadString(IDS_RIBBON_SHOW);
	ASSERT(bNameValid);
	CMFCRibbonCategory* pCategoryView = m_wndRibbonBar.AddCategory(strTemp, IDB_SMALL, IDB_MLARGE);

	// "����" �г��� ����� �߰��մϴ�.
	bNameValid = strTemp.LoadString(IDS_RIBBON_VIEW);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelView = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon (7));

	bNameValid = strTemp.LoadString(IDS_RIBBON_STATUSBAR);
	ASSERT(bNameValid);
	CMFCRibbonCheckBox* pBtnStatusBar = new CMFCRibbonCheckBox(ID_VIEW_STATUS_BAR, strTemp);
	pPanelView->Add(pBtnStatusBar);

	bNameValid = strTemp.LoadString(IDS_RIBBON_GRIDVIEW);
	ASSERT(bNameValid);
	CMFCRibbonCheckBox* pBtnGridLine = new CMFCRibbonCheckBox(ID_USER_GRIDLINE, strTemp);
	pPanelView->Add(pBtnGridLine);

	bNameValid = strTemp.LoadString(IDS_RIBBON_DECIDE);
	ASSERT(bNameValid);
	CMFCRibbonCheckBox* pBtnDecide = new CMFCRibbonCheckBox(ID_DECIDE, strTemp);
	pPanelView->Add(pBtnDecide);

	CMFCRibbonCheckBox* pBtnLineNumber = new CMFCRibbonCheckBox(ID_USER_LINENUMBER, _T("���� ��ȣ"));
	pPanelView->Add(pBtnLineNumber);

	bNameValid = strTemp.LoadString(IDS_RIBBON_GROUND);
	ASSERT(bNameValid);
	CMFCRibbonCheckBox* pBtnGround = new CMFCRibbonCheckBox(ID_GROUND, strTemp);
	pPanelView->Add(pBtnGround);

	// "â" �г��� ����� �߰��մϴ�.
	bNameValid = strTemp.LoadString(IDS_RIBBON_WINDOW);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelWindow = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon (7));

	bNameValid = strTemp.LoadString(IDS_RIBBON_NEW);
	ASSERT(bNameValid);
	pPanelWindow->Add(new CMFCRibbonButton(ID_FILE_NEW, strTemp, 6, 6));

	bNameValid = strTemp.LoadString(IDS_RIBBON_WINDOWS);
	ASSERT(bNameValid);
	CMFCRibbonButton* pBtnWindows = new CMFCRibbonButton(ID_WINDOW_MANAGER, strTemp, 17, 17);
	pBtnWindows->SetMenu(IDR_WINDOWS_MENU, TRUE);
	pPanelWindow->Add(pBtnWindows);

	// ��ŷ ����
	bNameValid = strTemp.LoadString(IDS_RIBBON_DOCKING);
	ASSERT(bNameValid);
	CMFCRibbonPanel* pPanelDocking = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon (7));

	bNameValid = strTemp.LoadString(IDS_RIBBON_ELEMENTPANE);
	ASSERT(bNameValid);
	pPanelDocking->Add(new CMFCRibbonButton(ID_USER_DOCKING_ELEVIEW, strTemp, 3)); 

	bNameValid = strTemp.LoadString(IDS_RIBBON_OUTPUTPANE);
	ASSERT(bNameValid);
	pPanelDocking->Add(new CMFCRibbonButton(ID_USER_DOCKING_OUTPUTVIEW, strTemp, 8));

	bNameValid = strTemp.LoadString(IDS_RIBBON_ELEMENTTREEVIEW);
	ASSERT(bNameValid);
	pPanelDocking->Add(new CMFCRibbonButton(ID_USER_DOCKING_ELETREEVIEW, strTemp, 2)); 

}

void CMainFrame::Add_Category3()
{
	BOOL bNameValid;
	CString strTemp;

	// ���� ���� �߰�
	bNameValid = strTemp.LoadString(IDS_RIBBON_SETTING);
	ASSERT(bNameValid);
	CMFCRibbonCategory* pCategorySetting = m_wndRibbonBar.AddCategory(strTemp,  IDB_SMALL, IDB_MLARGE);
	CMFCRibbonPanel* pPanelSettingView = pCategorySetting->AddPanel(_T("�Ӽ�â"), m_PanelImages.ExtractIcon (7));

	pPanelSettingView->Add(new CMFCRibbonButton(ID_USER_ELEMENT_EDITDLG, _T("����"), 3, 3)); 
}

void CMainFrame::OnUserNoteped()
{
	OnLink(ID_USER_NOTEPED);
}

void CMainFrame::OnUserSendmail()
{
	OnLink(ID_USER_SENDMAIL);
}

void CMainFrame::OnUserVisitsite()
{
	OnLink(ID_USER_VISITSITE);
}

void CMainFrame::OnLink (UINT nID)
{
	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> ar;
	m_wndRibbonBar.GetElementsByID (nID, ar);

	if (ar.GetSize () >= 1) {
		CMFCRibbonLinkCtrl* pLink = DYNAMIC_DOWNCAST (CMFCRibbonLinkCtrl, ar[0]);
		if (pLink != NULL) {
			if (!pLink->OpenLink ()) {
				AfxMessageBox (_T("Link clicked."));
			}
		}
	}
}



void CMainFrame::OnUserDockingEletreeview()
{
	m_wndClassView.ShowPane(TRUE, TRUE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUserDockingEleview()
{
	m_wndElementView.ShowPane(TRUE, TRUE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUserDockingOutputview()
{
	m_wndOutput.ShowPane(TRUE, TRUE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUserElementEditdlg()
{
	if (m_pElementEditDlg != NULL) {
		m_pElementEditDlg->SetFocus();
	}
	else {
		m_pElementEditDlg = new CElementEditDlg;
		m_pElementEditDlg->Create(IDD_ELEMENTEDITDLG, this);
		m_pElementEditDlg->ShowWindow(SW_SHOW);		
	}
}

void CMainFrame::SaveUserElementInfo()
{
	typedef struct _SaveElementGroup
	{
		TCHAR	GroupName[MAX_STRING];
	}SaveElementGroup;

	typedef struct _SaveElementInfo
	{
		TCHAR		ElementGroupName[MAX_STRING];
		TCHAR		ElementName[MAX_STRING];
		UINT		ElementNumber;
		UINT		ElementImageNumber;
		TCHAR		ElementIntroduceString[MAX_STRING*10];
	}SaveElementInfo;

	ofstream ElementFile("Element.ros", ios::binary);
	
	ElementFile << m_UserGroupList.size();
	list<ElementGroup>::iterator pos = m_UserGroupList.begin();
	while (pos != m_UserGroupList.end()) {

		SaveElementGroup NewGroup;
		::ZeroMemory(&NewGroup, sizeof(SaveElementGroup));
		_stprintf_s(NewGroup.GroupName, _T("%s"), (*pos).GroupName);
		ElementFile.write((char*)&NewGroup, sizeof(SaveElementGroup));

		if ((*pos).ElementList.size() > 0) {
			ElementFile << (*pos).ElementList.size();

			list<ElementInfo>::iterator lowpos = (*pos).ElementList.begin();
			while (lowpos != (*pos).ElementList.end()) {

				SaveElementInfo NewInfo;
				::ZeroMemory(&NewInfo, sizeof(SaveElementInfo));
				_stprintf_s(NewInfo.ElementGroupName, _T("%s"), (*lowpos).ElementGroupName);
				_stprintf_s(NewInfo.ElementName, _T("%s"), (*lowpos).ElementName);
				_stprintf_s(NewInfo.ElementIntroduceString, _T("%s"), (*lowpos).ElementIntroduceString);
				NewInfo.ElementImageNumber = (*lowpos).ElementImageNumber;
				NewInfo.ElementNumber = (*lowpos).ElementNumber;

				ElementFile.write((char*)&NewInfo, sizeof(SaveElementInfo));
				++lowpos;
			}
		}
		++pos;
	}
	ElementFile.close();
}

void CMainFrame::InitializeElementInfo()
{
	if (::GetElementInfoListToDll(m_DefaultGroupList) == FALSE) {
		AfxMessageBox(_T("Element.DLL�κ��� ���� ������ �ε����� ���߽��ϴ�."));
	}

	typedef struct _SaveElementGroup
	{
		TCHAR	GroupName[MAX_STRING];
	}SaveElementGroup;

	typedef struct _SaveElementInfo
	{
		TCHAR		ElementGroupName[MAX_STRING];
		TCHAR		ElementName[MAX_STRING];
		UINT		ElementNumber;
		UINT		ElementImageNumber;
		TCHAR		ElementIntroduceString[MAX_STRING*10];
	}SaveElementInfo;

 	ifstream UserElementFile("Element.ros", ios::binary);
	int GroupCount = 0;

	UserElementFile >> GroupCount;
	for (int iCount = 0; iCount < GroupCount; iCount++) {

		SaveElementGroup NewBuffer;
		UserElementFile.read((char*)&NewBuffer, sizeof(SaveElementGroup));

		ElementGroup NewGroup;
		NewGroup.GroupName.Format(_T("%s"), NewBuffer.GroupName);
		
 		int ElementCount = 0; 
 		UserElementFile >> ElementCount;
		for (int jCount = 0; jCount < ElementCount; jCount++) {
			
			SaveElementInfo NewInfoBuffer;
			::ZeroMemory(&NewInfoBuffer, sizeof(SaveElementInfo));
			UserElementFile.read((char*)&NewInfoBuffer, sizeof(SaveElementInfo));

			ElementInfo NewInfo;
			NewInfo.ElementGroupName.Format(_T("%s"), NewInfoBuffer.ElementGroupName);
			NewInfo.ElementName.Format(_T("%s"), NewInfoBuffer.ElementName);
			NewInfo.ElementIntroduceString.Format(_T("%s"), NewInfoBuffer.ElementIntroduceString);
			NewInfo.ElementImageNumber = NewInfoBuffer.ElementImageNumber;
			NewInfo.ElementNumber = NewInfoBuffer.ElementNumber;
			NewGroup.ElementList.push_back(NewInfo);
		}
		m_UserGroupList.push_back(NewGroup);
	}
  	UserElementFile.close();
}



void CMainFrame::UpdateElementView()
{
	//���� ������Ʈ
	m_wndElementView.ReloadElementData();
}

void CMainFrame::UpdateClassView(CEzSpiceDoc *pDoc, BOOL ClearFlag)
{
	m_wndClassView.ReloadElementData(pDoc, ClearFlag);
}

void CMainFrame::DeleteClassView(CBaseElement *pElement)
{
	m_wndClassView.DeleteTreeItem(pElement);
}

void CMainFrame::OnClose()
{
	SaveUserElementInfo();
	CMDIFrameWndEx::OnClose();
}

void CMainFrame::OnUserSearchDirect()
{
	CString Searchstring = m_DirectSearch->GetEditText();
	Searchstring = Searchstring.MakeUpper();

	int MaxCount;
	Searchstring.GetLength() > 4 ? MaxCount = 4 : MaxCount = Searchstring.GetLength();

	list<ElementGroup>::iterator pos = m_DefaultGroupList.begin();
	while (pos != m_DefaultGroupList.end()) {

		list<ElementInfo>::iterator elementpos = (*pos).ElementList.begin();
		while (elementpos != (*pos).ElementList.end()) {
			BOOL nFlag = TRUE;
			for (int iCount = 0; iCount < MaxCount; iCount++) {
				if (Searchstring.GetAt(iCount) != (*elementpos).ElementName.GetAt(iCount)) {
					nFlag = FALSE;
				}
			}		

			if (nFlag) {
				m_DirectSearch->RemoveAllItems();
				CString VisualString = (*elementpos).ElementName;
				m_DirectSearch->AddItem(VisualString, (*elementpos).ElementNumber);
				break;
			}
			++elementpos;
		}
		++pos;
	}

	if (m_DirectSearch->GetCount() > 0) {
		m_DirectSearch->SelectItem(0);

		int ElementNumber = m_DirectSearch->GetItemData(0);
		CEzSpiceView *pView = (CEzSpiceView*)MDIGetActive()->GetActiveView();
		pView->InsertElement(ElementNumber);
	}
}

LRESULT CMainFrame::OnRecvEvent( WPARAM wParam, LPARAM lParam )
{
	MDIGetActive()->GetActiveView()->PostMessage(UM_RECV_EVENT);
	return 0;
}


void CMainFrame::OnEditTransient()
{
	if (m_pTransientDlg!= NULL) {
		m_pTransientDlg->SetFocus();
	}
	else {
		m_pTransientDlg = new CTransientDlg;
		m_pTransientDlg->m_SecValue = m_SecMax;
		m_pTransientDlg->m_VoltValue = m_VoltMax;
		m_pTransientDlg->Create(IDD_TRANSIENT, this);
		m_pTransientDlg->ShowWindow(SW_SHOW);		
	}
}

void CMainFrame::OnSimualSetting()
{
	if (m_pTransientDlg!= NULL) {
		m_pTransientDlg->SetFocus();
	}
	else {
		m_pTransientDlg = new CTransientDlg;
		m_pTransientDlg->m_SecValue = m_SecMax;
		m_pTransientDlg->m_VoltValue = m_VoltMax;
		m_pTransientDlg->Create(IDD_TRANSIENT, this);
		m_pTransientDlg->ShowWindow(SW_SHOW);		
	}
}

void CMainFrame::OnUserMath()
{
	if (m_pNetworkTheroyDlg != NULL) {
		m_pNetworkTheroyDlg->SetFocus();
	}
	else {
		m_pNetworkTheroyDlg = new CNetworkTheroy;
		m_pNetworkTheroyDlg->Create(IDD_NETWORKTHEROY, this);
		m_pNetworkTheroyDlg->ShowWindow(SW_SHOW);		
	}
}


void CMainFrame::OnFormulaHelp(CEzSpiceView *pView)
{
	if (m_pFormuladlg != NULL) {
		m_pFormuladlg->SetFocus();
	}
	else {
		m_pFormuladlg = new CHelp;
		m_pFormuladlg->Create(IDD_HELPLE, this);
		m_pFormuladlg->ShowWindow(SW_SHOW);	
		m_pFormuladlg->SetEzView(pView);
	}
}