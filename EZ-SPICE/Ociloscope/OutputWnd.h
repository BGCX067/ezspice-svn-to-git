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

#pragma once


/////////////////////////////////////////////////////////////////////////////
// COutputList 창
class COciloscopeView;
 
static const int MAXDATA  = 512;

class CHardWareView : public CStatic
{
	// 생성입니다.
public:
	CHardWareView();

	// 구현입니다.
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
// 생성입니다.
public:
	COutputWnd();

// 특성입니다.
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

// 구현입니다.
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

