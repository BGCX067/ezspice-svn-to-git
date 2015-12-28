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
#include "ClassView.h"
#include "OutputWnd.h"
#include "ElementView.h"
#include "ElementEditDlg.h"
#include "TransientDlg.h"
#include "NetworkTheroy.h"
#include "Help.h"

class CElementFactory;
class CEzSpiceView;

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

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
	CMFCRibbonBar				m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages			m_PanelImages;
	CMFCRibbonStatusBar			m_wndStatusBar;

public:
	//패인 뷰
	CClassView					m_wndClassView;
	CElementView				m_wndElementView;
	COutputWnd					m_wndOutput;
	CMFCRibbonComboBox*			m_DirectSearch;

public:
	//편집 다이얼로그
	CElementEditDlg				*m_pElementEditDlg;
	CTransientDlg				*m_pTransientDlg;
	CNetworkTheroy				*m_pNetworkTheroyDlg;
	CHelp						*m_pFormuladlg;

public:
	int							m_SecMax;		//주기 관련
	int							m_VoltMax;

// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	DECLARE_MESSAGE_MAP()

	void InitializeRibbon();
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

public:
	//리본바 패널 구성
	void Add_MainPanel();
	void Add_Qat();
	void Add_Category1();
	void Add_Category2();
	void Add_Category3();
	
public:
	//링크 함수들 
	void OnLink (UINT nID);
	afx_msg void OnUserNoteped();
	afx_msg void OnUserSendmail();
	afx_msg void OnUserVisitsite();
	afx_msg void OnUserDockingEletreeview();
	afx_msg void OnUserDockingEleview();
	afx_msg void OnUserDockingOutputview();
	afx_msg void OnUserElementEditdlg();
	afx_msg void OnClose();
	afx_msg void OnUserSearchDirect();
	
public: //함수
	static CElementFactory* CreateElementFactory();
	
public: //소자 정보 
	list<ElementGroup>		m_UserGroupList;
	list<ElementGroup>		m_DefaultGroupList;

	void InitializeElementInfo();
	void UpdateElementView();
	void UpdateClassView(CEzSpiceDoc *pDoc, BOOL ClearFlag = FALSE);
	void DeleteClassView(CBaseElement *pElement);
	void SaveUserElementInfo();


	//IPC 통신
public:
	afx_msg LRESULT OnRecvEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditTransient();
	afx_msg void OnSimualSetting();
	afx_msg void OnUserMath();
	void OnFormulaHelp(CEzSpiceView *pView);
};


