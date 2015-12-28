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

// EzSpice.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
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
	// ǥ�� ������ ���ʷ� �ϴ� ���� ����Դϴ�.
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// ǥ�� �μ� ���� ����Դϴ�.
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CEzSpiceApp ����

CEzSpiceApp::CEzSpiceApp()
:m_Mutex(FALSE, TEXT("IPC_TEST_MUTEX")),
m_ExitEvent(FALSE, TRUE),
m_ReadEvent(FALSE, TRUE, _T("IPC_READ_SHAREDMEMORY"))
{

	m_hMap = NULL;
	m_pSharedMemory = NULL;
	m_bHiColorIcons = TRUE;

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}

// ������ CEzSpiceApp ��ü�Դϴ�.

CEzSpiceApp theApp;
ULONG_PTR gdiplusToken;


// CEzSpiceApp �ʱ�ȭ

BOOL CEzSpiceApp::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�. 
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if (::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
	{
		AfxMessageBox(_T("ERROR: Failed to initialize GDI+ library!"));
		return FALSE;
	}

	// OLE ���̺귯���� �ʱ�ȭ�մϴ�.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));
	
	
	if(!InitSharedMemory()) {
		return FALSE;
	}
	AfxBeginThread(CEzSpiceApp::ThreadReadSharedMemory, NULL);

	LoadStdProfileSettings(0);  // ǥ�� INI ���� �ɼ��� �ε��մϴ�.

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// ���� ���α׷��� ���� ���ø��� ����մϴ�. ���� ���ø���
	//  ����, ������ â �� �� ������ ���� ������ �մϴ�.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_EzSpiceTYPE,
		RUNTIME_CLASS(CEzSpiceDoc),
		RUNTIME_CLASS(CChildFrame), // ����� ���� MDI �ڽ� �������Դϴ�.
		RUNTIME_CLASS(CEzSpiceView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// �� MDI ������ â�� ����ϴ�.
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// ���̻簡 ���� ��쿡�� DragAcceptFiles�� ȣ���մϴ�.
	//  MDI ���� ���α׷������� m_pMainWnd�� ������ �� �ٷ� �̷��� ȣ���� �߻��ؾ� �մϴ�.


	// ǥ�� �� ���, DDE, ���� ���⿡ ���� ������� ���� �м��մϴ�.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// ����ٿ� ������ ����� ����ġ�մϴ�.
	// ���� ���α׷��� /RegServer, /Register, /Unregserver �Ǵ� /Unregister�� ���۵� ��� FALSE�� ��ȯ�մϴ�.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// �� â�� �ʱ�ȭ�Ǿ����Ƿ� �̸� ǥ���ϰ� ������Ʈ�մϴ�.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}



// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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

// ��ȭ ���ڸ� �����ϱ� ���� ���� ���α׷� ����Դϴ�.
void CEzSpiceApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CEzSpiceApp ����� ���� �ε�/���� �޼���

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



BOOL CEzSpiceApp::InitSharedMemory()	//���� �޸��� ���� �����͸� �����ϴ� ��
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




