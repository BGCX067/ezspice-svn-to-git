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
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// COutputList â

class COutputList : public CListBox
{
// �����Դϴ�.
public:
	COutputList();

// �����Դϴ�.
public:
	virtual ~COutputList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditClear();
	afx_msg void OnEditCopy();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnLbnSelchange();
//	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

class COutputWnd : public CDockablePane
{
// �����Դϴ�.
public:
	COutputWnd();

// Ư���Դϴ�.
protected:
	CFont m_Font;
	CMFCTabCtrl	m_wndTabs;
	COutputList m_wndOutputBuild;

	void AdjustHorzScroll(CListBox& wndListBox);

// �����Դϴ�.
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

public:
	void InsertString(CString str);
	void InsertString(vector<CString> list);
	void ClearString();
protected:
//	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

