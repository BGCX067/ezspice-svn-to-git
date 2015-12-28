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

// OciloscopeView.h : COciloscopeView Ŭ������ �������̽�
//


#pragma once
#include "StructInformation.h"
#include "PlotterDlg.h"
#include <list>
#include <vector>

class COciloscopeView : public CFormView
{
protected: // serialization������ ��������ϴ�.
	COciloscopeView();
	DECLARE_DYNCREATE(COciloscopeView)

public:
	enum{ IDD = IDD_OCILOSCOPE_FORM };

// Ư���Դϴ�.
public:
	COciloscopeDoc* GetDocument() const;

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual void OnInitialUpdate(); // ���� �� ó�� ȣ��Ǿ����ϴ�.

// �����Դϴ�.
public:
	virtual ~COciloscopeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
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
	double					m_MinVoltage;		//������ ����
	double					m_MaxVoltage;		//�ֻ��� ����
	double					m_Anniversary;		//���ֱ�
	double					m_Hertz;			//�ִ� �ֱ�
	double					m_Frequency;		//�� �ֱ�
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

#ifndef _DEBUG  // OciloscopeView.cpp�� ����� ����
inline COciloscopeDoc* COciloscopeView::GetDocument() const
   { return reinterpret_cast<COciloscopeDoc*>(m_pDocument); }
#endif

