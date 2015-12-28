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

#pragma once

#include "ViewTree.h"

class CEzSpiceDoc;
class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CClassView : public CDockablePane
{
public:
	CClassView();
	virtual ~CClassView();

	void AdjustLayout();
	void OnChangeVisualStyle();

protected:
	CClassToolBar		m_wndToolBar;
	CViewTree			m_wndClassView;
	CImageList			m_ClassViewImages;
	UINT				m_nCurrSort;

	UINT				m_CircultCount;
	UINT				m_ChildCount;
	CEzSpiceDoc			*m_LastDoc;

// �������Դϴ�.
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	void ReloadElementData(CEzSpiceDoc *pDoc, BOOL ClearFlag = FALSE);
	void Reconstruction(HTREEITEM parent, list<CBaseElement*>::iterator &pos);
	void DeleteTreeItem(CBaseElement *pElement);
	

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

