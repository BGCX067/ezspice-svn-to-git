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

// OciloscopeView.h : COciloscopeView 클래스의 인터페이스
//


#pragma once
#include "StructInformation.h"
#include "PlotterDlg.h"
#include <list>
#include <vector>

class COciloscopeView : public CFormView
{
protected: // serialization에서만 만들어집니다.
	COciloscopeView();
	DECLARE_DYNCREATE(COciloscopeView)

public:
	enum{ IDD = IDD_OCILOSCOPE_FORM };

// 특성입니다.
public:
	COciloscopeDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.

// 구현입니다.
public:
	virtual ~COciloscopeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg LRESULT OnRecvEvent(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg void OnMaxfrequency();
	afx_msg void OnMaxvoltage();
	afx_msg void OnDestroy();	

	afx_msg void OnFlotter();
	afx_msg void OnFlottera();
	afx_msg void OnFlotterb();
	afx_msg void OnFlotterc();

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

public:
	//IPC
	void SendEzSpice();
public:
	//Wave Draw
	double					m_MinVoltage;		//최하위 전압
	double					m_MaxVoltage;		//최상의 전압
	double					m_Anniversary;		//한주기
	double					m_Hertz;			//최대 주기
	double					m_Frequency;		//한 주기
	int						m_CircuitCase;

	IPCHeader				m_Header;
	vector<IPCSectorHeader>	m_SectorList;
	list<IPCData>			m_IPCDataList;
	BOOL					m_HardwareFlag;

	//thread

	int						m_DataIndex;

	CPlotterDlg				*m_PlotterDlg;
	CPlotter				*m_SelectPlotter;
	Gdiplus::Bitmap			*m_Bitmap;

public:
	int						m_VoltageSpinValue;
	int						m_FrequencySpinValue;


	void DrawGrid(Gdiplus::Graphics* pDC);
	void DrawAxis(Gdiplus::Graphics* pDC);
	void DrawTexts(Gdiplus::Graphics* pDC);
	void DrawData(Gdiplus::Graphics* pDC);

	CPlotter* CreatePlotter();
	void SetPlotterInformation(CPlotter *NewPlotter, int DataNumber);
	void CopyDrawImage();
	void ReDrawPlotterDlg();
	void SetHertz(double hertz);	
	LRESULT OnReDraw(WPARAM wParam, LPARAM lParam);

	afx_msg void OnReflesh();
};

#ifndef _DEBUG  // OciloscopeView.cpp의 디버그 버전
inline COciloscopeDoc* COciloscopeView::GetDocument() const
   { return reinterpret_cast<COciloscopeDoc*>(m_pDocument); }
#endif

