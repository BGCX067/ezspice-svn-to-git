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

// OciloscopeDoc.h : COciloscopeDoc Ŭ������ �������̽�
//


#pragma once

#include <list>
class CPlotter;
class COciloscopeView;

class COciloscopeDoc : public CDocument
{
protected: // serialization������ ��������ϴ�.
	COciloscopeDoc();
	DECLARE_DYNCREATE(COciloscopeDoc)

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
	virtual ~COciloscopeDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()

public:
	list<CPlotter*>			m_PlotterList;

public:
	void Draw(Gdiplus::Graphics* pDC, COciloscopeView *pView);
	CPlotter* IsObject( CPoint point );
	BOOL DuplicateObject(CString plottername);
	void MappingObject(float ratio, CRect NewClientRect);
	void RemoveAll();
	void DeleteObject(CPlotter *pobject);
};


