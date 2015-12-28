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

// OciloscopeDoc.cpp : COciloscopeDoc 클래스의 구현
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


// COciloscopeDoc 생성/소멸

COciloscopeDoc::COciloscopeDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.

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

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}


// COciloscopeDoc serialization

void COciloscopeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// COciloscopeDoc 진단

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


// COciloscopeDoc 명령
