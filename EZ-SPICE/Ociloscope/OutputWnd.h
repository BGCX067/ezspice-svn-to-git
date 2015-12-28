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


/////////////////////////////////////////////////////////////////////////////
// COutputList â
class COciloscopeView;
 
static const int MAXDATA  = 512;

class CHardWareView : public CStatic
{
	// �����Դϴ�.
public:
	CHardWareView();

	// �����Դϴ�.
public:
	virtual ~CHardWareView();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnViewOutput();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

public:
	void DrawAxis(CDC *parm_dc);
	void DrawGrid(CDC *parm_dc);
	void DrawGraph(CDC *parm_dc);
	void SetCpuvalue(double Cpuvalue);

public:
	BOOL				m_Flag;
	double				m_Data[MAXDATA];
	CDC					*m_DisplayMemory;
	CBitmap				*m_Bitmap, *m_OldBitmap;
	int					m_Index, m_Gridcount;
	CRect				m_ClientRect;
	int					m_CenterPostion;
	CPaintDC			*m_PaintDC;

	COciloscopeView		*m_pView;
};

class COutputWnd : public CDockablePane
{
// �����Դϴ�.
public:
	COutputWnd();

// Ư���Դϴ�.
protected:
	CFont				m_Font;
	CMFCTabCtrl			m_wndTabs;
	CHardWareView		m_wndHardView;
	COciloscopeView		*m_pView;

	double				m_PastData;

protected:
	void AdjustHorzScroll(CListBox& wndListBox);
public:
	void SetView(COciloscopeView *pView);
	void SetGraphData(double data = 0);

// �����Դϴ�.
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

