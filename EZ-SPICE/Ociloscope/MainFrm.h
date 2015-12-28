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

// MainFrm.h : CMainFrame Ŭ������ �������̽�
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
	
protected: // serialization������ ��������ϴ�.
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Ư���Դϴ�.
public:

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// �����Դϴ�.
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // ��Ʈ�� ������ ���Ե� ����Դϴ�.
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

// ������ �޽��� �� �Լ�
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
	//���
	CFunctionGenerator	*m_pFunctionDlg;
	CDigitalMultimeter	*m_pMultimeterDlg;
	CPowerSupply		*m_pPowerSupplyDlg;

	//USB_HID ���
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


