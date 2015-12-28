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

// EzSpiceDoc.h : CEzSpiceDoc Ŭ������ �������̽�
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
protected: // serialization������ ��������ϴ�.
	CEzSpiceDoc();
	DECLARE_DYNCREATE(CEzSpiceDoc)

// Ư���Դϴ�.
public:

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// �����Դϴ�.
public:
	virtual ~CEzSpiceDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
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
	void LoadDataElementConnect();																	//�ҷ��� ������ �ٽ� ����
	BOOL IsElementsConnecting(CBaseElement* firstelement, CBaseElement* secondelement);				//�ΰ��� ��ü�� ���� �����ΰ�?

	CBaseElement* IsObjected(const CPoint& point, list<CBaseElement*> *ExceptionList = NULL);		// �ش� ��ġ�� ��ü �˻�
	void DeleteElement( CBaseElement *pElement);													//�ش� ��ü ���� �� �ش� ��ü�� ��ũ���� ����
	void Draw(Gdiplus::Graphics* pDC, CEzSpiceView* pView);

	BOOL DuplicateElement(CBaseElement *pParent, CBaseElement *pChild);								//�ߺ� �˻�
	void RemoveAllCircultList(list<CBaseElement*> &CircultList);									//��� ����
	void ExChangeCircult(list<CBaseElement*> &Circult);												//redo,undo�� circult ��ü
	void CopyConnectToLink(list<CBaseElement*> &ParentList, list<CBaseElement*> &ChildList);		//��ũ������ �θ� ���� number�� ���� ����
	double Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);		//
	void InsertReUndo();																				

	CPoint GetCrossPosition( CPoint &point, vector<CBaseElement*> &Junctionlist);					//Junction ��ġ �� ��ü ��ȯ
	int GetPositionCount(CBaseElement* CurrentElement, CPoint &point, vector<CBaseElement*> &Copylist);		//���� ��ǥ�� ��ġ�Ǵ� ��� ������ ��
	void LinkTransfer(CBaseElement *ReceiveElement, CBaseElement *SendElement );				//���õ� ���ΰ� ���� ������ ��ġ�Ͽ� ��ũ ����
	int GetAxis( CBaseElement *pElement );														//���� ������ ���� Ȯ���Ѵ�.
	void MergerLinkConnect(CBaseElement *CurrentElement);
	void MoveMergerConnectLine();																//�̵��Ҷ� ��� ��ü�� Ư�� ���ǿ� ���� ����
	void MergerConnectLine();																	//���� �׸��� ��� ���� ��ü�� Ư�� ���ǿ� ���� �����Ѵ�.
	BOOL GetIntersectPoint(CBaseElement* pFirstElement, CBaseElement* pSecodeElement, CPoint &ReturnPoint);
	int GetElementCount(int ElementFlag);


};


