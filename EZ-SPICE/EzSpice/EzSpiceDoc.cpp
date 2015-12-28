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

// EzSpiceDoc.cpp : CEzSpiceDoc Ŭ������ ����
//

#include "stdafx.h"
#include "EzSpice.h"
#include "MainFrm.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include "BaseElement.h"
#include "DrawTool.h"
#include <algorithm>
#include <math.h>
#include <fstream>
#include "ElementFactory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEzSpiceDoc

IMPLEMENT_DYNCREATE(CEzSpiceDoc, CDocument)

BEGIN_MESSAGE_MAP(CEzSpiceDoc, CDocument)
END_MESSAGE_MAP()


// CEzSpiceDoc ����/�Ҹ�

CEzSpiceDoc::CEzSpiceDoc()
{
	m_ToolList.push_back(new CSelectTool);
	m_ToolList.push_back(new CElementTool);
	m_ToolList.push_back(new CLineTool);

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateClassView(this);

	ReUndo NewBackup;
	m_ReUndoList.push_back(NewBackup);
	m_ReUndoNumber = 1;

}

CEzSpiceDoc::~CEzSpiceDoc()
{
	RemoveAllCircultList(m_CircuitList);

	vector<CDrawTool*>::iterator pos = m_ToolList.begin();
	while (pos != m_ToolList.end()) {
		delete (*pos);
		++pos;
	}
	m_ToolList.clear();

	vector<ReUndo>::iterator Backpos = m_ReUndoList.begin();
	while (Backpos != m_ReUndoList.end()) {
		RemoveAllCircultList(Backpos->m_CircuitList);
		++Backpos;
	}

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateClassView(this, TRUE);
	m_ReUndoList.clear();
	
}

void CEzSpiceDoc::RemoveAllCircultList(list<CBaseElement*> &CircultList)
{
	list<CBaseElement*>::iterator pos = CircultList.begin();
	while (pos != CircultList.end()) {
		delete (*pos);
		++pos;
	}
	CircultList.clear();
}

BOOL CEzSpiceDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}


// CEzSpiceDoc serialization
void CEzSpiceDoc::Serialize(CArchive& ar)
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CElementFactory *pFactory = pFrame->CreateElementFactory();

	if (ar.IsStoring())
	{
		if (!m_CircuitList.empty()) {

			ar << (int)m_CircuitList.size();
			list<CBaseElement*>::iterator pos = m_CircuitList.begin();
			while (pos != m_CircuitList.end()) {

				ElementInformation NewInformation = pFactory->GetElementToInformation((*pos));
				NewInformation.Serialize(ar);
				++pos;
			}
		}
	}
	else
	{
		int CircuitSize = 0;
		ar >> CircuitSize; 

		for (int iCount = 0; iCount < CircuitSize; iCount++) {
			
			ElementInformation NewInformation;
			NewInformation.Serialize(ar);
			CBaseElement *NewElement = pFactory->GetInformationToElement(NewInformation);
			m_CircuitList.push_back(NewElement);
		}
		LoadDataElementConnect();
	}
}

void CEzSpiceDoc::LoadDorgFile( CString CurrentFileAdress )
{
	CFile CurrentFile;
	BOOL Result = CurrentFile.Open(CurrentFileAdress, CFile::modeRead);
	if (Result == FALSE) {
		AfxMessageBox(_T("������ ���� �� �����ϴ�."));
		return;
	}
	CArchive ar(&CurrentFile, CArchive::load);

	int CircuitSize = 0;
	ar >> CircuitSize; 
	if (CircuitSize > 0 && CircuitSize < 100) {

		m_CircuitList.clear();
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		CElementFactory *pFactory = pFrame->CreateElementFactory();

		for (int iCount = 0; iCount < CircuitSize; iCount++) {

			ElementInformation NewInformation;
			NewInformation.Serialize(ar);
			CBaseElement *NewElement = pFactory->GetInformationToElement(NewInformation);
			m_CircuitList.push_back(NewElement);
		}
		LoadDataElementConnect();

		CString Token, FinalString;
		int TokenCount = 0;
		while (AfxExtractSubString(Token, CurrentFileAdress, TokenCount++, '\\')) {
			FinalString = Token;
		}

		this->SetTitle(FinalString);
		pFrame->UpdateClassView(this, TRUE);
		UpdateAllViews(NULL);
	}
	ar.Close();
	CurrentFile.Close();

}



void CEzSpiceDoc::LoadDataElementConnect()
{
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {

		list<CBaseElement*>::iterator connectpos = m_CircuitList.begin();
		while (connectpos != m_CircuitList.end()) {
	
			if ((*connectpos) != (*pos) && IsElementsConnecting((*pos), (*connectpos))) {
				(*pos)->m_ConnectElement.push_back((*connectpos));
			}
			++connectpos;
		}
		++pos;
	}
}

BOOL CEzSpiceDoc::IsElementsConnecting(CBaseElement* firstelement, CBaseElement* secondelement)
{
	if (firstelement->m_Flag == lines && secondelement->m_Flag == lines) {
	
		if (firstelement->m_LineTo == secondelement->m_LineTo || firstelement->m_LineTo == secondelement->m_MoveTo
			|| firstelement->m_MoveTo == secondelement->m_LineTo || firstelement->m_MoveTo == secondelement->m_MoveTo) {
				return TRUE;
		}
	}
	else if (firstelement->m_Flag == lines && secondelement->m_Flag != lines) {
		
		list<CPoint>::iterator pointpos = secondelement->m_ConnectPoint.begin();
		while (pointpos != secondelement->m_ConnectPoint.end()) {
				
			if (secondelement->m_Position + (*pointpos) == firstelement->m_LineTo 
				|| secondelement->m_Position + (*pointpos) == firstelement->m_MoveTo) {
					return TRUE;
			}
			++pointpos;
		}
	}
	else if (firstelement->m_Flag != lines && secondelement->m_Flag == lines) {
		list<CPoint>::iterator pointpos = firstelement->m_ConnectPoint.begin();
		while (pointpos != firstelement->m_ConnectPoint.end()) {

			if (firstelement->m_Position + (*pointpos) == secondelement->m_LineTo 
				|| firstelement->m_Position + (*pointpos) == secondelement->m_MoveTo) {
					return TRUE;
			}
			++pointpos;
		}
	}

	return FALSE;
}

// CEzSpiceDoc ����

CBaseElement* CEzSpiceDoc::IsObjected(const CPoint& point, list<CBaseElement*> *ExceptionList)
{
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {

		if (ExceptionList != NULL) {
			list<CBaseElement*>::iterator findpos = find(ExceptionList->begin(), ExceptionList->end(), (*pos));
			if (findpos == ExceptionList->end()) {
				if ((*pos)->ObjectInPoint(point)) {
					return (*pos);
				}
			}
		}
		else {
			if ((*pos)->ObjectInPoint(point)) {
				return (*pos);
			}
		}
		++pos;
	}
	return NULL;
}

int	CEzSpiceDoc::GetElementCount(int ElementFlag)
{
	int iCount = 0;
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		if ((*pos)->m_Flag == ElementFlag) {
			iCount++;
		}
		++pos;
	}
	return iCount;
}

double CEzSpiceDoc::Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y) 
{
	double segment_mag = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
	double Distance;

	if(segment_mag != 0){		
		double u = ((x-x1)*(x2-x1)+(y-y1)*(y2-y1))/segment_mag ;
		double xp = x1+ u * (x2-x1);
		double yp = y1+ u * (y2-y1);
		Distance = sqrt((xp-x)*(xp-x)+(yp-y)*(yp-y));
	}
	else{
		Distance = sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1));
	}

	return Distance;
}


void CEzSpiceDoc::Draw( Gdiplus::Graphics *pDC, CEzSpiceView* pView )
{
	BOOLCollection Collection;
	Collection.LineFlag = CBaseElement::m_DrawState == Connectline ? TRUE : FALSE;
	Collection.GroundAreaFlag = pView->m_GroundFlag;
	Collection.DecideFlag = pView->m_DecideFlag;
	Collection.LineStringFlag = pView->m_LineStringFlag;
		
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		if (pView->IsSelected((*pos))) {
			Collection.Selectflag = TRUE;
			(*pos)->Draw(pDC, Collection);	//Ȱ��ȭ �׸���
		}
		else {
			Collection.Selectflag = FALSE;
			(*pos)->Draw(pDC, Collection);		//��Ȱ��ȭ �׸���
		}
		++pos;
	}

	if (pView->m_SelectModityElement.size() > 0) {
		list<LineModity>::iterator Moditypos = pView->m_SelectModityElement.begin();

		Collection.Selectflag = FALSE;
		list<ModityInfo>::iterator pos = (*Moditypos).ConnectList.begin();
		while (pos != (*Moditypos).ConnectList.end()) {
			(*pos).ConnectElement->Draw(pDC, Collection, (*Moditypos).ConnectPoint);	//���� ���� Ȱ��ȭ �׸���
			++pos;
		}
	}
}

void CEzSpiceDoc::DeleteElement( CBaseElement *pElement )
{
	list<CBaseElement*>::iterator pos = find(m_CircuitList.begin(), m_CircuitList.end(), pElement);
	if (pos != m_CircuitList.end()) {

		list<CBaseElement*>::iterator lowpos = (*pos)->m_ConnectElement.begin();
		while (lowpos != (*pos)->m_ConnectElement.end()) {
			(*lowpos)->DeleteElement((*pos));					
			++lowpos;
		}
		delete (*pos);
		m_CircuitList.erase(pos);
	}
}

void CEzSpiceDoc::InsertReUndo()
{
	for (int iCount = m_ReUndoList.size() -1; iCount > m_ReUndoNumber -1 ; --iCount) {
		ReUndo *pReUn = &m_ReUndoList[iCount];
		RemoveAllCircultList(pReUn->m_CircuitList);
		m_ReUndoList.pop_back();
	}

	//��ü ����
	ReUndo NewBackup;
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		CBaseElement* CloneElement = (*pos)->Clone();
		if (CloneElement != NULL) {
			NewBackup.m_CircuitList.push_back(CloneElement);
		}
		++pos;
	}

	CopyConnectToLink(m_CircuitList, NewBackup.m_CircuitList);
	m_ReUndoList.push_back(NewBackup);
	m_ReUndoNumber++;
}

BOOL CEzSpiceDoc::Redo()
{
	if (m_ReUndoNumber +1 <= (int)m_ReUndoList.size()) {

		CEzSpiceView *pView = (CEzSpiceView*)m_viewList.GetHead();
		pView->m_SelectModityElement.clear();

		m_ReUndoNumber++;
		RemoveAllCircultList(m_CircuitList);

		ReUndo *pReUn = &m_ReUndoList[m_ReUndoNumber-1];
		ExChangeCircult(pReUn->m_CircuitList);

		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->UpdateClassView(this, TRUE);
		return TRUE;   
	}
	return FALSE;
}

BOOL CEzSpiceDoc::Undo()
{
	if (m_ReUndoNumber -1 > 0) {
		
		CEzSpiceView *pView = (CEzSpiceView*)m_viewList.GetHead();
		pView->m_SelectModityElement.clear();

		m_ReUndoNumber--;
		RemoveAllCircultList(m_CircuitList);
		
		ReUndo *pReUn = &m_ReUndoList[m_ReUndoNumber-1];
		ExChangeCircult(pReUn->m_CircuitList);
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->UpdateClassView(this, TRUE);
		return TRUE;
	}
	return FALSE;
}

void CEzSpiceDoc::ExChangeCircult(list<CBaseElement*> &Circult) 
{
	m_CircuitList.clear();
	list<CBaseElement*>::iterator pos = Circult.begin();
	while (pos != Circult.end()) {
		m_CircuitList.push_back((*pos)->Clone());
		++pos;
	}
	CopyConnectToLink(Circult, m_CircuitList);
}

void CEzSpiceDoc::CopyConnectToLink(list<CBaseElement*> &ParentList, list<CBaseElement*> &ChildList)
{
	//��ũ ����
	list<CBaseElement*>::iterator pos = ParentList.begin();
	while (pos != ParentList.end()) {

		CompareBuffer Buffer((*pos)->m_ElementNumber, (*pos)->m_Flag);
		list<CBaseElement*>::iterator Copypos = find_if(ChildList.begin(), ChildList.end(), Buffer);

		if (Copypos != ChildList.end()) {

			list<CBaseElement*>::iterator lowpos = (*pos)->m_ConnectElement.begin();
			while (lowpos != (*pos)->m_ConnectElement.end()) {
				Buffer.SetElementNumber((*lowpos)->m_ElementNumber, (*lowpos)->m_Flag);
				list<CBaseElement*>::iterator Copylowpos = find_if(ChildList.begin(), ChildList.end(), Buffer);

				if (Copylowpos != ChildList.end()) {
					//�ߺ��˻� �ʿ�
					if (DuplicateElement((*Copypos), (*Copylowpos)) == FALSE) {
						(*Copypos)->m_ConnectElement.push_back((*Copylowpos));
					}

					if (DuplicateElement((*Copylowpos), (*Copypos)) == FALSE) {
						(*Copylowpos)->m_ConnectElement.push_back((*Copypos));
					}
				}
				++lowpos;
			}
		}
		++pos;
	}
}

BOOL CEzSpiceDoc::DuplicateElement(CBaseElement *pParent, CBaseElement *pChild)
{
	list<CBaseElement*>::iterator Findpos = find(pParent->m_ConnectElement.begin(), pParent->m_ConnectElement.end(), pChild);
	if (Findpos != pParent->m_ConnectElement.end()) {
		return TRUE;
	}
	return FALSE;
}

void CEzSpiceDoc::MergerLinkConnect(CBaseElement *CurrentElement)
{
	CPoint ComparePoint = CurrentElement->m_LineTo;
	CurrentElement->m_LineTo = CurrentElement->m_MoveTo = CPoint(0,0);

	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {

		list<CBaseElement*>::iterator lowpos = CurrentElement->m_ConnectElement.begin();
		while (lowpos != CurrentElement->m_ConnectElement.end()) {
			if ((*pos) != (*lowpos)) {

				list<CBaseElement*>::iterator Duplicatepos = find((*lowpos)->m_ConnectElement.begin(), (*lowpos)->m_ConnectElement.end(), (*pos));
				if (Duplicatepos == (*lowpos)->m_ConnectElement.end()) {
					(*pos)->m_ConnectElement.push_back((*lowpos));
					(*lowpos)->m_ConnectElement.push_back((*pos));
				}
			}
			++lowpos;
		}
		++pos;
	}
}

void CEzSpiceDoc::MoveMergerConnectLine()
{
	list<CBaseElement*> DeleteList;
	//�Ѱ�ü�� m_LineTo�� m_MoveTo�� ��ġ�ϴ� ��� �ش� ��ü ����
	list<CBaseElement*>::iterator Exceptionpos = m_CircuitList.begin();
	while (Exceptionpos != m_CircuitList.end()) {
		if ((*Exceptionpos)->m_Flag == lines) {
			if ((*Exceptionpos)->m_LineTo == (*Exceptionpos)->m_MoveTo) {
				MergerLinkConnect((*Exceptionpos));
				DeleteList.push_back((*Exceptionpos));
			}
		}
		++Exceptionpos;
	}

	list<CBaseElement*>::iterator deletepos = DeleteList.begin();
	while (deletepos != DeleteList.end()) {
		DeleteElement((*deletepos));
		++deletepos;
	}

	//�ι� �ϸ� �� ��Ȯ����
	MergerConnectLine();
	MergerConnectLine();
}

void CEzSpiceDoc::MergerConnectLine()
{
	list<CBaseElement*> DeleteList;

	//�Ϲ� ���� �˰��� (���� ���϶� �ΰ��� ��ü ����)
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {

		list<CBaseElement*>::iterator Duplicatepos = find(DeleteList.begin(), DeleteList.end(), (*pos));
		if (Duplicatepos == DeleteList.end() && (*pos)->m_Flag == lines) {
			int BaseElementAxis = this->GetAxis((*pos));

			vector<CBaseElement*> Linelist;
			BOOL LineFlag = FALSE;
			if (GetPositionCount((*pos), (*pos)->m_LineTo, Linelist) == 1) {
				LineFlag = TRUE;
			}

			vector<CBaseElement*> MoveList;
			BOOL MoveFlag = FALSE;
			if (GetPositionCount((*pos), (*pos)->m_MoveTo, MoveList) == 1) {
				MoveFlag = TRUE;
			}

			if (LineFlag && MoveFlag == FALSE && BaseElementAxis == GetAxis(Linelist[0])) {	//Move�ʿ� ���ڰ� �ɸ��ų� 3�� �̻�(��Ÿ� ����)�϶�
				if (Linelist[0]->GetPositonDirection((*pos)->m_LineTo) == LineToConnect) {
					(*pos)->m_LineTo = Linelist[0]->m_MoveTo;
				}
				else {
					(*pos)->m_LineTo = Linelist[0]->m_LineTo;
				}
				LinkTransfer((*pos), Linelist[0]);
				DeleteList.push_back(Linelist[0]);

				if (Linelist[0] == CLineTool::m_pNewFirstLine) CLineTool::m_pNewFirstLine = NULL;
				if (Linelist[0] == CLineTool::m_pNewSecondLine) CLineTool::m_pNewSecondLine = NULL;
			}
			else if (LineFlag == FALSE && MoveFlag && BaseElementAxis == GetAxis(MoveList[0])) { //Line�ʿ� ���ڰ� �ɸ��ų� 3�� �̻�(��Ÿ� ����)�϶�
				if (MoveList[0]->GetPositonDirection((*pos)->m_MoveTo) == LineToConnect) {
					(*pos)->m_MoveTo = MoveList[0]->m_MoveTo;
				}
				else {
					(*pos)->m_MoveTo = MoveList[0]->m_LineTo;
				}
				LinkTransfer((*pos), MoveList[0]);
				DeleteList.push_back(MoveList[0]);

				if (MoveList[0] == CLineTool::m_pNewFirstLine) CLineTool::m_pNewFirstLine = NULL;
				if (MoveList[0] == CLineTool::m_pNewSecondLine) CLineTool::m_pNewSecondLine = NULL;
			}
			else if(LineFlag && MoveFlag) {
				//������ ��� TRUE�Ͻ� ���������� ����
				if (BaseElementAxis == GetAxis(Linelist[0])) {
					if (Linelist[0]->GetPositonDirection((*pos)->m_LineTo) == LineToConnect) {
						(*pos)->m_LineTo = Linelist[0]->m_MoveTo;
					}
					else {
						(*pos)->m_LineTo = Linelist[0]->m_LineTo;
					}
					LinkTransfer((*pos), Linelist[0]);
					DeleteList.push_back(Linelist[0]);

					if (Linelist[0] == CLineTool::m_pNewFirstLine) CLineTool::m_pNewFirstLine = NULL;
					if (Linelist[0] == CLineTool::m_pNewSecondLine) CLineTool::m_pNewSecondLine = NULL;
				}

				if (BaseElementAxis == GetAxis(MoveList[0])) {
					if (MoveList[0]->GetPositonDirection((*pos)->m_MoveTo) == LineToConnect) {
						(*pos)->m_MoveTo = MoveList[0]->m_MoveTo;
					}
					else {
						(*pos)->m_MoveTo = MoveList[0]->m_LineTo;
					}
					LinkTransfer((*pos), MoveList[0]);
					DeleteList.push_back(MoveList[0]);

					if (MoveList[0] == CLineTool::m_pNewFirstLine) CLineTool::m_pNewFirstLine = NULL;
					if (MoveList[0] == CLineTool::m_pNewSecondLine) CLineTool::m_pNewSecondLine = NULL;
				}
			}
		}
		++pos;
	}

	list<CBaseElement*>::iterator deletepos = DeleteList.begin();
	while (deletepos != DeleteList.end()) {
		DeleteElement((*deletepos));
		++deletepos;
	}
}

void CEzSpiceDoc::LinkTransfer(CBaseElement *ReceiveElement, CBaseElement *SendElement )
{

	list<CBaseElement*>::iterator pos = SendElement->m_ConnectElement.begin();
	while (pos != SendElement->m_ConnectElement.end()) {
		if ((*pos) != ReceiveElement) {
			ReceiveElement->m_ConnectElement.push_back((*pos));
			(*pos)->m_ConnectElement.push_back(ReceiveElement);
		}
		++pos;
	}
	SendElement->m_LineTo = SendElement->m_MoveTo = CPoint(0,0);
}

int CEzSpiceDoc::GetPositionCount(CBaseElement* CurrentElement, CPoint &point, vector<CBaseElement*> &Copylist)
{
	int TotalCount = 0;
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {
		if ((*pos)->m_Flag == lines) {
			if ((*pos)->m_LineTo == point || (*pos)->m_MoveTo == point) {
				Copylist.push_back((*pos));
				TotalCount++;
			}
		}
		++pos;
	}
	return TotalCount;
}

int CEzSpiceDoc::GetAxis( CBaseElement *pElement )
{
	float Angleaxis = 0;
	Angleaxis = abs((float)(pElement->m_LineTo.y - pElement->m_MoveTo.y) / abs((float)(pElement->m_LineTo.x - pElement->m_MoveTo.x)));

	int Axis;
	if (Angleaxis == 0) {
		if (pElement->m_MoveTo.x == pElement->m_LineTo.x && pElement->m_MoveTo.y != pElement->m_LineTo.y) {
			Axis = Yaxis;
		}
		else {
			Axis = Xaxis;
		}
	}
	else {
		if (Angleaxis < 0.45 && Angleaxis > -0.45) {
			Axis = Xaxis;
		}
		else {
			Axis = Yaxis;
		}
	}
	return Axis;
}

CPoint CEzSpiceDoc::GetCrossPosition( CPoint &point, vector<CBaseElement*> &Junctionlist)
{
 	int ResultCount = 0;
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		
		if ((*pos)->m_Flag == lines && (*pos)->ObjectInPoint(point)) {
			Junctionlist.push_back((*pos));
		}
		++pos;
	}

	CPoint CrossPoint;
	if (Junctionlist.size() == 2 && GetIntersectPoint(Junctionlist[0], Junctionlist[1], CrossPoint)) {
		return CrossPoint;
	}

	return CPoint(-1, -1);
}

BOOL CEzSpiceDoc::GetIntersectPoint(CBaseElement* pFirstElement, CBaseElement* pSecodeElement, CPoint &ReturnPoint) 
{
	CPoint AP1 = pFirstElement->m_LineTo;
	CPoint AP2 = pFirstElement->m_MoveTo;
	CPoint BP1 = pSecodeElement->m_LineTo;
	CPoint BP2 = pSecodeElement->m_MoveTo;

	double t;
	double s; 
	double under = (BP2.y-BP1.y)*(AP2.x-AP1.x)-(BP2.x-BP1.x)*(AP2.y-AP1.y);
	if(under==0) return false;

	double _t = (BP2.x-BP1.x)*(AP1.y-BP1.y) - (BP2.y-BP1.y)*(AP1.x-BP1.x);
	double _s = (AP2.x-AP1.x)*(AP1.y-BP1.y) - (AP2.y-AP1.y)*(AP1.x-BP1.x); 

	t = _t/under;
	s = _s/under; 

	if(t<0.0 || t>1.0 || s<0.0 || s>1.0) return false;
	if(_t==0 && _s==0) return false; 

	ReturnPoint.x = (int)(AP1.x + t * (double)(AP2.x-AP1.x));
	ReturnPoint.y = (int)(AP1.y + t * (double)(AP2.y-AP1.y));

	return TRUE;
}


#ifdef _DEBUG
void CEzSpiceDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEzSpiceDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}



#endif //_DEBUG


// CEzSpiceDoc ���



//void CEzSpiceDoc::OnFileSave()
//{
//
//	//m_CircuitList
//
// 	CFileDialog dlg(FALSE, _T("dat"), _T("*.dat"), OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT, _T("dat Files(*.dat)||"));
// 	dlg.m_ofn.lpstrTitle = _T("Saving file...");
// 	if( dlg.DoModal()== IDOK) 
// 	{
// 		CString filename = dlg.GetPathName();
// 	}
//}

//void CEzSpiceDoc::OnFileOpen()
//{
// 	CFileDialog dlg(TRUE, _T("dat"), _T("*.dat"), OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT, _T("dat Files(*.dat)||"));
// 	dlg.m_ofn.lpstrTitle = _T("Loading file...");
// 	if( dlg.DoModal()== IDOK) 
// 	{
// 		CString filename = dlg.GetPathName();
// 
// 	}
//}
