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

// EzSpiceDoc.h : CEzSpiceDoc 클래스의 인터페이스
//


#pragma once

class CBaseElement;
class CDrawTool;
class CEzSpiceView;
class Graphics;

#include <vector>
#include <list>
#include <gdiplus.h>

typedef struct _ReUndo
{
	list<CBaseElement*>			m_CircuitList;
}ReUndo;

class CEzSpiceDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
	CEzSpiceDoc();
	DECLARE_DYNCREATE(CEzSpiceDoc)

// 특성입니다.
public:

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// 구현입니다.
public:
	virtual ~CEzSpiceDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()



public:
	vector<CDrawTool*>			m_ToolList;
	list<CBaseElement*>			m_CircuitList;
	
	vector<ReUndo>				m_ReUndoList;
	int							m_ReUndoNumber;
public:

	void LoadDorgFile(CString CurrentFileAdress);

	BOOL Redo();
	BOOL Undo();
	void LoadDataElementConnect();																	//불러온 데이터 다시 연결
	BOOL IsElementsConnecting(CBaseElement* firstelement, CBaseElement* secondelement);				//두개의 객체가 서로 연결인가?

	CBaseElement* IsObjected(const CPoint& point, list<CBaseElement*> *ExceptionList = NULL);		// 해당 위치에 객체 검색
	void DeleteElement( CBaseElement *pElement);													//해당 객체 삭제 및 해당 객체의 링크에도 영향
	void Draw(Gdiplus::Graphics* pDC, CEzSpiceView* pView);

	BOOL DuplicateElement(CBaseElement *pParent, CBaseElement *pChild);								//중복 검사
	void RemoveAllCircultList(list<CBaseElement*> &CircultList);									//모두 삭제
	void ExChangeCircult(list<CBaseElement*> &Circult);												//redo,undo시 circult 교체
	void CopyConnectToLink(list<CBaseElement*> &ParentList, list<CBaseElement*> &ChildList);		//링크연결을 부모 내부 number를 통해 연결
	double Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);		//
	void InsertReUndo();																				

	CPoint GetCrossPosition( CPoint &point, vector<CBaseElement*> &Junctionlist);					//Junction 위치 및 객체 반환
	int GetPositionCount(CBaseElement* CurrentElement, CPoint &point, vector<CBaseElement*> &Copylist);		//현재 좌표와 일치되는 모든 라인의 수
	void LinkTransfer(CBaseElement *ReceiveElement, CBaseElement *SendElement );				//선택된 라인과 현재 라인이 일치하여 링크 이전
	int GetAxis( CBaseElement *pElement );														//현재 소자의 축을 확인한다.
	void MergerLinkConnect(CBaseElement *CurrentElement);
	void MoveMergerConnectLine();																//이동할때 모든 객체를 특정 조건에 의해 병합
	void MergerConnectLine();																	//라인 그릴때 모든 라인 객체를 특정 조건에 의해 병합한다.
	BOOL GetIntersectPoint(CBaseElement* pFirstElement, CBaseElement* pSecodeElement, CPoint &ReturnPoint);
	int GetElementCount(int ElementFlag);


};


