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

// OciloscopeDoc.cpp : COciloscopeDoc Ŭ������ ����
//

#include "stdafx.h"
#include "Ociloscope.h"
#include "OciloscopeDoc.h"
#include "Plotter.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COciloscopeDoc

IMPLEMENT_DYNCREATE(COciloscopeDoc, CDocument)

BEGIN_MESSAGE_MAP(COciloscopeDoc, CDocument)
END_MESSAGE_MAP()


// COciloscopeDoc ����/�Ҹ�

COciloscopeDoc::COciloscopeDoc()
{
	// TODO: ���⿡ ��ȸ�� ���� �ڵ带 �߰��մϴ�.

}

COciloscopeDoc::~COciloscopeDoc()
{
	list<CPlotter*>::iterator pos = m_PlotterList.begin();
	while (pos != m_PlotterList.end()) {
		delete (*pos);
		++pos;
	}
}

void COciloscopeDoc::Draw( Gdiplus::Graphics* pDC, COciloscopeView *pView)
{
	list<CPlotter*>::iterator pos = m_PlotterList.begin();
	while (pos != m_PlotterList.end()) {
		(*pos)->Draw(pDC, pView);
		++pos;
	}
}

CPlotter* COciloscopeDoc::IsObject( CPoint point )
{
	list<CPlotter*>::iterator pos = m_PlotterList.begin();
	while (pos != m_PlotterList.end()) {
		if ((*pos)->IsObject(point)) {
			return (*pos);
		}
		++pos;
	}
	return NULL;
}

BOOL COciloscopeDoc::DuplicateObject(CString plottername)
{
	list<CPlotter*>::iterator pos = m_PlotterList.begin();
	while (pos != m_PlotterList.end()) {
		if ((*pos)->m_FlotterName == plottername) {
			return TRUE;
		}
		++pos;
	}

	return FALSE;
}

void COciloscopeDoc::MappingObject( float ratio, CRect NewClientRect)
{
	list<CPlotter*>::iterator pos = m_PlotterList.begin();
	while (pos != m_PlotterList.end()) {
		(*pos)->m_XAxis = (int)((float)(*pos)->m_XAxis * ratio);	

		if ((*pos)->m_XAxis <= 100 ) {
			(*pos)->m_XAxis = 100;
		}		
		else if ((*pos)->m_XAxis >= NewClientRect.Width() - 40) {
			(*pos)->m_XAxis = (*pos)->m_ClientRect.Width() - 40;
		}
		++pos;
	}
}

void COciloscopeDoc::RemoveAll()
{
	list<CPlotter*>::iterator pos = m_PlotterList.begin();
	while (pos != m_PlotterList.end()) {
		delete (*pos);
		++pos;
	}
	m_PlotterList.clear();
}

void COciloscopeDoc::DeleteObject( CPlotter *pobject )
{
	list<CPlotter*>::iterator searchpos = find(m_PlotterList.begin(), m_PlotterList.end(), pobject);
	if (searchpos != m_PlotterList.end()) {
		m_PlotterList.erase(searchpos);
	}
}


BOOL COciloscopeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: ���⿡ ���ʱ�ȭ �ڵ带 �߰��մϴ�.
	// SDI ������ �� ������ �ٽ� ����մϴ�.

	return TRUE;
}


// COciloscopeDoc serialization

void COciloscopeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	}
	else
	{
		// TODO: ���⿡ �ε� �ڵ带 �߰��մϴ�.
	}
}


// COciloscopeDoc ����

#ifdef _DEBUG
void COciloscopeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COciloscopeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}







#endif //_DEBUG


// COciloscopeDoc ���
