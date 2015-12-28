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

// EzSpiceView.h : CEzSpiceView Ŭ������ �������̽�
//


#pragma once

class CElementSearchDlg;
class CBaseElement;
class CDigitalMultimeter;

#include <gdiplus.h>
#include "StructInformation.h"

enum DrawLineType{
	Xaxis, Yaxis
};

typedef struct _ModityInfo
{
public:
	int				MovePosition;
	int				ConnectCount;
	CBaseElement*	ConnectElement;

	bool operator()(struct _ModityInfo &rs) const {
		return ConnectElement == rs.ConnectElement;
	}

}ModityInfo;

typedef struct _LineModity
{
	CPoint				ConnectPoint;
	list<ModityInfo>	ConnectList;
}LineModity;

class CEzSpiceView : public CScrollView
{
protected: // serialization������ ��������ϴ�.
	CEzSpiceView();
	DECLARE_DYNCREATE(CEzSpiceView)

// Ư���Դϴ�.
public:
	CEzSpiceDoc* GetDocument() const;

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	// �����Դϴ�.
public:
	virtual ~CEzSpiceView();
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
	afx_msg LRESULT OnRecvEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnUserGridline();
	afx_msg void OnUpdateUserGridline(CCmdUI *pCmdUI);
	virtual void OnInitialUpdate();

	afx_msg void OnUserDeviceDisit();
	afx_msg void OnUserDeviceFunc();
	afx_msg void OnUserDeviceOscil();
	afx_msg void OnUserDevicePower();
	afx_msg void OnUserStateUndo();
	afx_msg void OnUserStateRedo();
	afx_msg void OnUserStateIncrease();
	afx_msg void OnUserStateDecrease();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	afx_msg void OnUserSrarchDetail();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnUserSimulProv1();
	afx_msg void OnUserSimulProv2();
	afx_msg void OnUserSimulStart();
	afx_msg void OnEditUndo();
	afx_msg void OnJunction();	
	afx_msg void OnGround();
	afx_msg void OnDecide();
	afx_msg void OnUpdateDecide(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGround(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();			
	afx_msg void OnUserLinenumber();
	afx_msg void OnUpdateUserLinenumber(CCmdUI *pCmdUI);
	afx_msg void OnDropFiles(HDROP hDropInfo);

public:

	BOOL					m_ActivityFlag;
	HCURSOR					m_CrossCursor;
	BOOL					m_Grid;			
	int						m_nZoomRate;
	CElementSearchDlg		*m_pSearchDlg;
	CDigitalMultimeter		*m_pMultimeterDlg;

	UINT					m_DrawState;			// defalut(selection), 'w' keydown(line), element click(insert)
	list<CBaseElement*>		m_SelectElement;		//�⺻ ���� (�⺻ �̵�)
	list<LineModity>		m_SelectModityElement;	//�𼭸� ���� (Ư�� �̵� , ������)
	list<CPoint>			m_Connectdescription;   //���� ���� ǥ��
	vector<IPCSectorHeader>	m_SectorList;
		
	CPoint					m_ScrollPos;
	static int				m_GridExtent;
	BOOL					m_ConnerFlag;

	IPCHeader				m_Header;
	list<IPCData>			m_IPCDataList;
	int						m_OciloscopeFlag;
	BOOL					m_JunctionFlag;

	BOOL					m_DecideFlag;
	BOOL					m_GroundFlag;
	BOOL					m_LineStringFlag;

	Gdiplus::Bitmap			*m_Bitmap;

public:
	
	void DrawGrid(Gdiplus::Graphics* pDC);								//�׸��� �׸���
	void SetSituation(int state);										//���콺 ���� ����
	void SetActivityElement(CBaseElement* pElement);					//��ü ���� ����Ʈ�� ���� �� �ٽ� �׸���
	BOOL IsSelected(CBaseElement* pElement);							//���� ��ü�� ���ø���Ʈ�� �ִ���
	void MoveActivityElement(const CPoint& point);						//���õ� ��ü�� ��� �̵�
	void SetActivityInRect(const CPoint& down, const CPoint& last);		//�簢�����ȿ� �մ� ��ü�� ��� Ȱ��ȭ
	void SetNonActivityElement(CBaseElement * pElement);				//�ش� ��ü �� Ȱ��ȭ
	void DeleteAllActivityElemnt();										//Ȱ��ȭ ��ü ��� ����
	void ResizeScrollSizes();
	void InsertElement(int ElementNumber);
	void AllActivityElement();											//��� ��ü Ȱ��ȭ
	void ExceptionConnecting();											//�߸��� ������ ����ó�� (���� �������� ����ó��)
	
																
	int GetConnectDirection(CBaseElement *pBaseElement, CBaseElement *pCompareElement);							//��ü �̵��� �̵� �� ���� �˻�
	BOOL isCrossBoundBox(CPoint FirstLineTo, CPoint FirstMoveTo, CPoint SecondLineTo, CPoint SecondMoveTo );	//����� ������ ��ü�˻�
	BOOL CheckIntersect(CPoint FirstLineTo, CPoint FirstMoveTo, CPoint SecondLineTo, CPoint SecondMoveTo );		//����� ������ ��ü�˻�
	void DrawConnectdescription(Gdiplus::Graphics* pDC);														//������� �׸���
	void SetActivityModityElement( CPoint &point );																//���� �̵��� �ƴ� ���� Ȱ��ȭ
	void MoveActivityModityElement( const CPoint& point );														//���� �̵��� �ƴ� ���� ����
	void Connectdescription();																					//��������� ������ �ȵ� ��ü�� 2�� �̻��̸� ���� ����

	int GetAxis( CBaseElement *pElement );						//�ش� ������ �� ����
	int GetElementCountToPoint(CPoint point);					//������ǥ�� �ִ� ��� ���� ����
	void DecisionAxis();										//��� ������ �� ����
	void SendOciloscope();										//IPC ������� ������ ����
	void OnEditRedo();	
	int GetProvCount();											//������ ���κ� ���� �ľ�
	void MultimeterSimulStart();								//������ ��Ƽ���� ����
	void RotateElement();										//��ü ȸ��
	int GetProvColor(int ProvNumber);							//���κ� �� ����
	int NextProvColor();										//���� ���κ� ���� �˻�
	void CopyDrawImage();										//��� �̹��� ����
	int GetDistance(CPoint FirstPosition, CPoint SecondPosition);


};

#ifndef _DEBUG  // EzSpiceView.cpp�� ����� ����
inline CEzSpiceDoc* CEzSpiceView::GetDocument() const
   { return reinterpret_cast<CEzSpiceDoc*>(m_pDocument); }

#endif

