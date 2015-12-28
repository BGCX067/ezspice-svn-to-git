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

// EzSpiceView.h : CEzSpiceView 클래스의 인터페이스
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
protected: // serialization에서만 만들어집니다.
	CEzSpiceView();
	DECLARE_DYNCREATE(CEzSpiceView)

// 특성입니다.
public:
	CEzSpiceDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	// 구현입니다.
public:
	virtual ~CEzSpiceView();
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
	list<CBaseElement*>		m_SelectElement;		//기본 선택 (기본 이동)
	list<LineModity>		m_SelectModityElement;	//모서리 선택 (특수 이동 , 값수정)
	list<CPoint>			m_Connectdescription;   //연결 정보 표시
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
	
	void DrawGrid(Gdiplus::Graphics* pDC);								//그리드 그리기
	void SetSituation(int state);										//마우스 상태 변경
	void SetActivityElement(CBaseElement* pElement);					//객체 선택 리스트에 삽입 및 다시 그리기
	BOOL IsSelected(CBaseElement* pElement);							//현재 객체가 선택리스트에 있는지
	void MoveActivityElement(const CPoint& point);						//선택된 객체들 모두 이동
	void SetActivityInRect(const CPoint& down, const CPoint& last);		//사각영역안에 잇는 객체들 모두 활성화
	void SetNonActivityElement(CBaseElement * pElement);				//해당 객체 비 활성화
	void DeleteAllActivityElemnt();										//활성화 객체 모두 삭제
	void ResizeScrollSizes();
	void InsertElement(int ElementNumber);
	void AllActivityElement();											//모든 객체 활성화
	void ExceptionConnecting();											//잘못된 설계의 예외처리 (라인 동적생성 예외처리)
	
																
	int GetConnectDirection(CBaseElement *pBaseElement, CBaseElement *pCompareElement);							//객체 이동시 이동 축 결정 검사
	BOOL isCrossBoundBox(CPoint FirstLineTo, CPoint FirstMoveTo, CPoint SecondLineTo, CPoint SecondMoveTo );	//사격형 영역에 객체검사
	BOOL CheckIntersect(CPoint FirstLineTo, CPoint FirstMoveTo, CPoint SecondLineTo, CPoint SecondMoveTo );		//사격형 영역에 객체검사
	void DrawConnectdescription(Gdiplus::Graphics* pDC);														//연결부위 그리기
	void SetActivityModityElement( CPoint &point );																//라인 이동이 아닌 수정 활성화
	void MoveActivityModityElement( const CPoint& point );														//라인 이동이 아닌 수정 로직
	void Connectdescription();																					//연결부위에 연결이 안된 객체가 2개 이상이면 서로 연결

	int GetAxis( CBaseElement *pElement );						//해당 소자의 축 결정
	int GetElementCountToPoint(CPoint point);					//현재좌표가 있는 모든 소자 갯수
	void DecisionAxis();										//모든 소자의 축 결정
	void SendOciloscope();										//IPC 통신으로 데이터 전송
	void OnEditRedo();	
	int GetProvCount();											//현재의 프로브 갯수 파악
	void MultimeterSimulStart();								//디지털 멀티미터 실행
	void RotateElement();										//객체 회전
	int GetProvColor(int ProvNumber);							//프로브 색 추출
	int NextProvColor();										//다음 프로브 색상 검색
	void CopyDrawImage();										//배경 이미지 생성
	int GetDistance(CPoint FirstPosition, CPoint SecondPosition);


};

#ifndef _DEBUG  // EzSpiceView.cpp의 디버그 버전
inline CEzSpiceDoc* CEzSpiceView::GetDocument() const
   { return reinterpret_cast<CEzSpiceDoc*>(m_pDocument); }

#endif

