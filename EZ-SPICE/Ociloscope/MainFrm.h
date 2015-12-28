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

// MainFrm.h : CMainFrame 클래스의 인터페이스
//

#pragma once
#include "OutputWnd.h"
#include "hidapi.h"
#include "FunctionGenerator.h"
#include "DigitalMultimeter.h"
#include "PowerSupply.h"

#define HID_BUFFERSIZE 4
#define HID_VID 0x0483
#define HID_PID 0x5750

class CMainFrame : public CFrameWndEx
{
	
protected: // serialization에서만 만들어집니다.
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 특성입니다.
public:

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 구현입니다.
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 컨트롤 모음이 포함된 멤버입니다.
	CMFCRibbonBar					m_wndRibbonBar;
	CMFCRibbonApplicationButton		m_MainButton;
	CMFCToolBarImages				m_PanelImages;
	CMFCRibbonStatusBar				m_wndStatusBar;
	
public:
	COutputWnd						m_wndSimualString;

	CMFCRibbonEdit					*m_pEditIndentLeft;
	CMFCRibbonEdit					*m_pEditIndentRight;

	CMFCRibbonButton				*m_pBtnHardwareConnect;

public:
	HANDLE							m_hEvent;
	CWinThread						*m_UpdataThread;
	bool							m_ExitThread;
	int								m_SummonsSpeed;

// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnRecvEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHardwareOcil();

	DECLARE_MESSAGE_MAP()

	void InitializeRibbon();
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	

public:
	void Add_Category1();
	void SetOutwndViewpoint(COciloscopeView *pView);
	static UINT UpdateInfoThreadFunc(LPVOID pParam);
	afx_msg void OnDestroy();


public:
	//장비
	CFunctionGenerator	*m_pFunctionDlg;
	CDigitalMultimeter	*m_pMultimeterDlg;
	CPowerSupply		*m_pPowerSupplyDlg;

	//USB_HID 통신
	unsigned char		m_HIDBuffer[HID_BUFFERSIZE];
	hid_device			*m_HIDhandle;
	BOOL				m_USBFlag;

	BOOL				m_HardWareFlag;
	BOOL				m_HardWareOciloscopeFlag;
	double				m_AmplificationRate;

public: 
	BOOL USB_HID_Create();
	void USB_HID_Destory();
	void USB_HID_Sendreport(int reportID, char Mode, char HighByte, char LowByte);
	afx_msg void OnHardwareConnect();
	afx_msg void OnHardwareDisit();
	afx_msg void OnHardwarePower();
	afx_msg void OnHardwareFunc();
	afx_msg void OnAmpli();
};


