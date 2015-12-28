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

// EzSpice.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "EzSpice.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include "StructInformation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEzSpiceApp

BEGIN_MESSAGE_MAP(CEzSpiceApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CEzSpiceApp::OnAppAbout)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// 표준 인쇄 설정 명령입니다.
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CEzSpiceApp 생성

CEzSpiceApp::CEzSpiceApp()
:m_Mutex(FALSE, TEXT("IPC_TEST_MUTEX")),
m_ExitEvent(FALSE, TRUE),
m_ReadEvent(FALSE, TRUE, _T("IPC_READ_SHAREDMEMORY"))
{

	m_hMap = NULL;
	m_pSharedMemory = NULL;
	m_bHiColorIcons = TRUE;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 CEzSpiceApp 개체입니다.

CEzSpiceApp theApp;
ULONG_PTR gdiplusToken;


// CEzSpiceApp 초기화

BOOL CEzSpiceApp::InitInstance()
{
	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다. 
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if (::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
	{
		AfxMessageBox(_T("ERROR: Failed to initialize GDI+ library!"));
		return FALSE;
	}

	// OLE 라이브러리를 초기화합니다.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));
	
	
	if(!InitSharedMemory()) {
		return FALSE;
	}
	AfxBeginThread(CEzSpiceApp::ThreadReadSharedMemory, NULL);

	LoadStdProfileSettings(0);  // 표준 INI 파일 옵션을 로드합니다.

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 응용 프로그램의 문서 템플릿을 등록합니다. 문서 템플릿은
	//  문서, 프레임 창 및 뷰 사이의 연결 역할을 합니다.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_EzSpiceTYPE,
		RUNTIME_CLASS(CEzSpiceDoc),
		RUNTIME_CLASS(CChildFrame), // 사용자 지정 MDI 자식 프레임입니다.
		RUNTIME_CLASS(CEzSpiceView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// 주 MDI 프레임 창을 만듭니다.
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// 접미사가 있을 경우에만 DragAcceptFiles를 호출합니다.
	//  MDI 응용 프로그램에서는 m_pMainWnd를 설정한 후 바로 이러한 호출이 발생해야 합니다.


	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// 명령줄에 지정된 명령을 디스패치합니다.
	// 응용 프로그램이 /RegServer, /Register, /Unregserver 또는 /Unregister로 시작된 경우 FALSE를 반환합니다.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 주 창이 초기화되었으므로 이를 표시하고 업데이트합니다.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}



// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CEzSpiceApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CEzSpiceApp 사용자 지정 로드/저장 메서드

void CEzSpiceApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CEzSpiceApp::LoadCustomState()
{
}

void CEzSpiceApp::SaveCustomState()
{
}

int CEzSpiceApp::ExitInstance()
{
	::GdiplusShutdown(gdiplusToken);

	if (m_pSharedMemory != NULL) {
		::UnmapViewOfFile(m_pSharedMemory);
	}

	if (m_hMap != NULL) {
		::CloseHandle(m_hMap);
	}

	Sleep(100);

	return CWinApp::ExitInstance();
}



BOOL CEzSpiceApp::InitSharedMemory()	//공유 메모리의 접근 포인터를 생성하는 곳
{
	m_hMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(IPCHeader) + (sizeof(IPCSectorHeader) *5) + (sizeof(IPCData)*5), _T("IPC_TEST_SHARED_MEMORY"));
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		m_hMap = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("IPC_TEST_SHARED_MEMORY"));
	}

	if (m_hMap == NULL) {
		AfxMessageBox(_T("Error : Failed to create file mapping object!!"));
		return FALSE;
	}

	m_pSharedMemory = (TCHAR*)::MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(IPCHeader) + (sizeof(IPCSectorHeader) *5) + (sizeof(IPCData)*5));

	if (m_pSharedMemory == NULL) {
		AfxMessageBox(_T("Error : Failed to get shared memory"));
		return FALSE;
	}

	return TRUE;
}

UINT CEzSpiceApp::ThreadReadSharedMemory(LPVOID pParam)
{
	DWORD dwResult = WAIT_OBJECT_0 + 1;
	HANDLE arhList[2];
	arhList[0] = theApp.m_ExitEvent;
	arhList[1] = theApp.m_ReadEvent;

	while(dwResult == WAIT_OBJECT_0 + 1) {
		dwResult = ::WaitForMultipleObjects(2, arhList, FALSE, INFINITE);

		if(dwResult == WAIT_OBJECT_0) {
			break;
		}
		else if(dwResult == WAIT_OBJECT_0 + 1) {
			theApp.m_pMainWnd->PostMessage(UM_RECV_EVENT);
			::Sleep(10);
		}
	}
	return 0;
}




