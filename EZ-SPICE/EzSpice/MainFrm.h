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
	CMFCRibbonBar				m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages			m_PanelImages;
	CMFCRibbonStatusBar			m_wndStatusBar;

public:
	//���� ��
	CClassView					m_wndClassView;
	CElementView				m_wndElementView;
	COutputWnd					m_wndOutput;
	CMFCRibbonComboBox*			m_DirectSearch;

public:
	//���� ���̾�α�
	CElementEditDlg				*m_pElementEditDlg;
	CTransientDlg				*m_pTransientDlg;
	CNetworkTheroy				*m_pNetworkTheroyDlg;
	CHelp						*m_pFormuladlg;

public:
	int							m_SecMax;		//�ֱ� ����
	int							m_VoltMax;

// ������ �޽��� �� �Լ�
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	DECLARE_MESSAGE_MAP()

	void InitializeRibbon();
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

public:
	//������ �г� ����
	void Add_MainPanel();
	void Add_Qat();
	void Add_Category1();
	void Add_Category2();
	void Add_Category3();
	
public:
	//��ũ �Լ��� 
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
	
public: //�Լ�
	static CElementFactory* CreateElementFactory();
	
public: //���� ���� 
	list<ElementGroup>		m_UserGroupList;
	list<ElementGroup>		m_DefaultGroupList;

	void InitializeElementInfo();
	void UpdateElementView();
	void UpdateClassView(CEzSpiceDoc *pDoc, BOOL ClearFlag = FALSE);
	void DeleteClassView(CBaseElement *pElement);
	void SaveUserElementInfo();


	//IPC ���
public:
	afx_msg LRESULT OnRecvEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditTransient();
	afx_msg void OnSimualSetting();
	afx_msg void OnUserMath();
	void OnFormulaHelp(CEzSpiceView *pView);
};


