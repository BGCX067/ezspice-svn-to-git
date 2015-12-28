#include "StdAfx.h"
#include "DrawTool.h"

#include "BaseElement.h"
#include "ElementFactory.h"
#include "EzSpice.h"
#include "MainFrm.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include <math.h>

CPoint CDrawTool::m_DownPoint;
CPoint CDrawTool::m_MovePoint;
BOOL CDrawTool::m_LButtonDownFlag;
UINT CDrawTool::m_DownFlag;
UINT CDrawTool::m_InsertKind;

CBaseElement* CDrawTool::m_pNewElement = NULL;
CBaseElement* CDrawTool::m_pNewFirstLine = NULL;
CBaseElement* CDrawTool::m_pNewSecondLine = NULL;

CDrawTool::CDrawTool(int State)
{
	m_DrawState = State;
}

CDrawTool::~CDrawTool(void)
{
}

CDrawTool* CDrawTool::FindTool(CEzSpiceView* pView, int State)
{
	CEzSpiceDoc *pDoc = pView->GetDocument();
	vector<CDrawTool*>::iterator ToolPos =  pDoc->m_ToolList.begin();
	while (ToolPos != pDoc->m_ToolList.end()) {
		if ((*ToolPos)->m_DrawState == State) {
			return (*ToolPos);
		}
		++ToolPos;
	}
	return NULL;
}


void CDrawTool::OnLButtonDown( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{	
	pView->SetCapture();
	m_DownPoint = point;
	m_MovePoint = point;
	m_DownFlag = nFlags;
	m_LButtonDownFlag = TRUE;

}

void CDrawTool::OnLButtonUp( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	ReleaseCapture();
	m_LButtonDownFlag = FALSE;
}

void CDrawTool::OnMouseMove( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	m_MovePoint = point;
}


void CDrawTool::OnLButtonDblck( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{

}


void CDrawTool::OnEditProperties( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{

}

void CDrawTool::ProvProcessing(CEzSpiceView *pView)
{
	list<CBaseElement*>::iterator pos = pView->GetDocument()->m_CircuitList.begin();
	while (pos != pView->GetDocument()->m_CircuitList.end()) {
		if ((*pos)->m_Flag == prov) {

			list<CBaseElement*> ExceptionList;
			ExceptionList.push_back((*pos));

			CBaseElement* pSelectElement = pView->GetDocument()->IsObjected((*pos)->m_Position, &ExceptionList);
			if (pSelectElement != NULL) {
				(*pos)->m_InputElement.clear();
				(*pos)->m_InputElement.push_back(pSelectElement);
				(*pos)->m_ProvActivityFlag = TRUE;
			}
			else {
				(*pos)->m_InputElement.clear();
				(*pos)->m_ProvActivityFlag = FALSE;
			}
		}
		++pos;
	}

	pView->Invalidate(FALSE);
}

void CDrawTool::OnTabDown( CEzSpiceView* pView )
{

}

void CDrawTool::GridPostion( CPoint &position )
{
	int GridValue = CEzSpiceView::m_GridExtent;

	int Xvalue = position.x / CEzSpiceView::m_GridExtent;
	Xvalue *= CEzSpiceView::m_GridExtent;
	if (position.x % CEzSpiceView::m_GridExtent > CEzSpiceView::m_GridExtent/2) {
		Xvalue += CEzSpiceView::m_GridExtent;
	}

	int Yvalue = position.y / CEzSpiceView::m_GridExtent;
	Yvalue *= CEzSpiceView::m_GridExtent;
	if (position.y % CEzSpiceView::m_GridExtent > CEzSpiceView::m_GridExtent/2) {
		Yvalue += CEzSpiceView::m_GridExtent;
	}

	position.SetPoint(Xvalue, Yvalue);
}


/////////////////////////////////////  CChoiceTool

enum SelectMode
{
	none, move
};

SelectMode selectMode = none;

CSelectTool::CSelectTool()
: CDrawTool(selection)
{

}

CSelectTool::~CSelectTool()
{

}

void CSelectTool::OnLButtonDown( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	m_MoveCount = 0;
	selectMode = none;
	CEzSpiceDoc *pDoc = pView->GetDocument();

	CPoint ConvertPos(point);
	CDrawTool::GridPostion(ConvertPos);

	CClientDC dc(pView);
	pView->OnPrepareDC(&dc);
	dc.DPtoLP(&ConvertPos);

	if (pView->m_JunctionFlag) {	//������ �߰� (��ü ����)
		vector<CBaseElement*> JunctionList;
		CPoint CrossPoint = pDoc->GetCrossPosition(ConvertPos, JunctionList);
		
		if (CrossPoint.x != -1, CrossPoint.y != -1) {
			
			list<CBaseElement*> ConnectList;
			vector<CBaseElement*>::iterator pos = JunctionList.begin();
			while (pos != JunctionList.end()) {

				CBaseElement *pFirstDivideElement = (*pos);
				CElementFactory *pFactory = CMainFrame::CreateElementFactory();				
				CBaseElement *pSecondDivideElement = pFactory->CreateElement(lines);	

				pSecondDivideElement->m_MoveTo = pFirstDivideElement->m_MoveTo;
				pSecondDivideElement->m_LineTo = CrossPoint;
				pFirstDivideElement->m_MoveTo = CrossPoint;

				DivideLinkConnect(pFirstDivideElement, pSecondDivideElement);
				pDoc->m_CircuitList.push_back(pSecondDivideElement);

				ConnectList.push_back(pFirstDivideElement);
				ConnectList.push_back(pSecondDivideElement);
				++pos;
			}

			list<CBaseElement*>::iterator connectpos = ConnectList.begin();
			while (connectpos != ConnectList.end()) {
				
				list<CBaseElement*>::iterator lowpos = ConnectList.begin();
				while (lowpos != ConnectList.end()) {
					
					if ((*lowpos) != (*connectpos)) {
						(*lowpos)->m_ConnectElement.push_back((*connectpos));
					}
					++lowpos;
				}
				++connectpos;
			}
		}
		pView->m_JunctionFlag = FALSE;
		pView->m_Connectdescription.clear();
		return;
	}
	
	CBaseElement* pSelectElement = pDoc->IsObjected(ConvertPos); //���� ��ġ�� ��ü�� �ֳ�?
	if (pSelectElement != NULL) {	
		
		pView->DecisionAxis();
		CPoint LineConnectPosition(0,0);
		if (pSelectElement->m_Flag == lines) {
			LineConnectPosition = pSelectElement->ComparePosition(ConvertPos);
		}

		if (pView->IsSelected(pSelectElement) == FALSE) {
			if ((nFlags & MK_CONTROL) == 0) {				//Ctrl �ȴ������� ��� ��Ȱ��ȭ
				pView->m_SelectElement.clear();
				pView->m_SelectModityElement.clear();
				pView->Invalidate(FALSE);
			}
			if (LineConnectPosition.x != 0 && LineConnectPosition.y != 0) {	//������ǥ�� ��ü ������ �����϶�
				pView->m_SelectElement.clear();	//��ü ������ �Ѱ��� �����ϵ��� ����
				pView->m_SelectModityElement.clear();

				pView->SetActivityModityElement(LineConnectPosition);	//m_SelelctModityElement�� ������ �ش� ��ü Ȱ��ȭ
			}
			else {
				pView->SetActivityElement(pSelectElement);			//m_SelelctElement�� ������ �ش� ��ü Ȱ��ȭ
			}
		}
		else {				//�̹� Ȱ��ȭ �����ΰ� �ٽ� ���� 
			if ((nFlags & MK_CONTROL) != 0) {	//Ctrl ������ ������ �ٽ� ��Ȱ��ȭ
				pView->SetNonActivityElement(pSelectElement);
			}
		}
		selectMode = move;
	}

	if (selectMode == none) {
		if (pView->m_SelectElement.size() > 0 || pView->m_SelectModityElement.size() > 0) {
			pView->m_SelectElement.clear();
			pView->m_SelectModityElement.clear();
			pView->Invalidate(FALSE);
		}

		CClientDC dc(pView);
		CRect rect(point.x, point.y, point.x, point.y);
		rect.NormalizeRect();
		dc.DrawFocusRect(rect);
	}

	if (selectMode == move) {
		pView->GetDocument()->MoveMergerConnectLine();
	}

	CDrawTool::OnLButtonDown(pView, nFlags, point);
}

void CSelectTool::DivideLinkConnect(CBaseElement *pFirstDivideElement, CBaseElement *pSecondDivideElement)
{
	list<CBaseElement*> FirstDeleteList;
	//��� �ɰ���
	list<CBaseElement*>::iterator pos = pFirstDivideElement->m_ConnectElement.begin();
	while (pos != pFirstDivideElement->m_ConnectElement.end()) {

		if ((*pos)->m_Flag != lines) {
			BOOL nFlag = FALSE;
			list<CPoint>::iterator Connectpos = (*pos)->m_ConnectPoint.begin();
			while (Connectpos != (*pos)->m_ConnectPoint.end()) {
				if ((*pos)->m_Position + (*Connectpos) == pFirstDivideElement->m_MoveTo
					|| (*pos)->m_Position + (*Connectpos) == pFirstDivideElement->m_LineTo) {
						nFlag = TRUE;
				}
				++Connectpos;
			}

			if (!nFlag) {
				pSecondDivideElement->m_ConnectElement.push_back((*pos));
				(*pos)->m_ConnectElement.push_back(pSecondDivideElement);
				(*pos)->DeleteElement(pFirstDivideElement);
				FirstDeleteList.push_back((*pos));
			}
		}
		else {
			if ((*pos)->m_MoveTo == pFirstDivideElement->m_MoveTo || (*pos)->m_LineTo == pFirstDivideElement->m_LineTo
				|| (*pos)->m_MoveTo == pFirstDivideElement->m_LineTo || (*pos)->m_LineTo == pFirstDivideElement->m_MoveTo) {
					NULL;
			}
			else {
				pSecondDivideElement->m_ConnectElement.push_back((*pos));
				(*pos)->m_ConnectElement.push_back(pSecondDivideElement);
				(*pos)->DeleteElement(pFirstDivideElement);
				FirstDeleteList.push_back((*pos));
			}
		}
		++pos;
	}

	list<CBaseElement*>::iterator deletepos = FirstDeleteList.begin();
	while (deletepos != FirstDeleteList.end()) {
		pFirstDivideElement->DeleteElement((*deletepos));
		++deletepos;
	}
}

void CSelectTool::OnMouseMove( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	if (pView->m_JunctionFlag) {
		CPoint ConvertPos(point);
		CDrawTool::GridPostion(ConvertPos);

		CClientDC dc(pView);
		pView->OnPrepareDC(&dc);
		dc.DPtoLP(&ConvertPos);

		pView->m_Connectdescription.clear();
		pView->m_Connectdescription.push_back(ConvertPos);
		pView->m_ConnerFlag = FALSE;
		pView->Invalidate(FALSE);
		return;
	}

	if (pView->GetCapture() != pView) {		//GetCapture()�Լ��� ���� �信 Ȱ��ȭ �Ǿ��ִ��� üũ�Ѵ�. �� ���� ��� üũ
		if (m_DrawState == selection) {
			CDrawTool::OnMouseMove(pView, nFlags, point);
		}
		return;
	}

	if (point.x < 0 || point.y < 0) {
		return;
	}	

	if (m_LButtonDownFlag) {
		if (selectMode== none) {
			CClientDC dc(pView);
			CRect rect(m_DownPoint.x, m_DownPoint.y, m_MovePoint.x, m_MovePoint.y);
			rect.NormalizeRect();
			dc.DrawFocusRect(rect);
			rect.SetRect(m_DownPoint.x, m_DownPoint.y, point.x, point.y);
			rect.NormalizeRect();
			dc.DrawFocusRect(rect);
		}
		else if (selectMode == move && (nFlags & MK_CONTROL) ==0) {

			CPoint PastPos(m_MovePoint);
			CDrawTool::GridPostion(PastPos);
			CPoint ConvertPos(point);
			CDrawTool::GridPostion(ConvertPos);

			CClientDC dc(pView);
			pView->OnPrepareDC(&dc);
			dc.DPtoLP(&ConvertPos);
			dc.DPtoLP(&PastPos);

			CPoint MovePoint(ConvertPos - PastPos);
			pView->MoveActivityElement(MovePoint);				//Ȱ��ȭ ��ü ��� �̵� �� �ٽ� �׸���
			pView->MoveActivityModityElement(MovePoint);		//Ȱ��ȭ ��ü ��� �̵� �� �ٽ� �׸���
			m_MoveCount++;
		}
	}

	CDrawTool::OnMouseMove(pView, nFlags, point);
}


void CSelectTool::OnLButtonUp( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	if (pView->GetCapture() == pView) {
		if (selectMode == none) {
			CClientDC dc(pView);
			CRect rect(m_DownPoint.x, m_DownPoint.y, m_MovePoint.x, m_MovePoint.y);
			rect.NormalizeRect();
			dc.DrawFocusRect(rect);

			CRect CompareRect(m_DownPoint, point);
			pView->OnPrepareDC(&dc);
			dc.DPtoLP(&CompareRect);

			CEzSpiceDoc *pDoc = pView->GetDocument();
			pView->SetActivityInRect(CPoint(CompareRect.left, CompareRect.top), CPoint(CompareRect.right, CompareRect.bottom));			//�簢 ���� �ȿ� �ִ� ��ü ��� Ȱ��ȭ
		}
		else if (selectMode == move && m_MoveCount > 5) {
			m_MoveCount = 0;		
			pView->GetDocument()->InsertReUndo();
			ProvProcessing(pView);		
		}
	}

	pView->GetDocument()->MoveMergerConnectLine();
	pView->m_SelectModityElement.clear();
	pView->Connectdescription();
	pView->Invalidate(FALSE);
	CDrawTool::OnLButtonUp(pView, nFlags, point);
}


void CSelectTool::OnLButtonDblClk( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
}

void CSelectTool::OnEditProperties( CEzSpiceView* pView )
{

}






/////////////////////////////////////  CElementTool  ���� ��ü ������ ��û�Ͽ� ����




CElementTool::CElementTool()
: CDrawTool(insert)
{
	m_pNewElement = NULL;
}

CElementTool::~CElementTool()
{

}


void CElementTool::OnLButtonDown( CEzSpiceView* pView, UINT nFlags, const CPoint& point )	
{
	CDrawTool::OnLButtonDown(pView, nFlags, point);
}

void CElementTool::OnLButtonDblClk( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{

}

void CElementTool::OnLButtonUp( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	int ElementFlag = -1;
	if (m_pNewElement != NULL) {

		CEzSpiceDoc *pDoc = pView->GetDocument();
		CPoint ConvertPos(point);
		CDrawTool::GridPostion(ConvertPos);

		CClientDC dc(pView);
		pView->OnPrepareDC(&dc);
		dc.DPtoLP(&ConvertPos);

		CBaseElement* pDuplicateElement = pDoc->IsObjected(ConvertPos);
		if (pDuplicateElement != m_pNewElement && m_pNewElement->m_Flag != prov) {
			AfxMessageBox(_T("�̹� ���ڰ� �����մϴ�."));
			return;
		}
		else {
			ElementFlag = m_pNewElement->m_Flag;
		}

		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->UpdateClassView(pView->GetDocument());
 		pView->GetDocument()->InsertReUndo();
		ProvProcessing(pView);

		if (m_pNewElement->m_Flag == ground) { //�׶��� ����ó�� (�׶��� ���Խ� �ٷ� ������� ����)
			ElementFlag = -1;

			pView->SetSituation(Connectline);
			CDrawTool *pTool = CDrawTool::FindTool(pView, pView->m_DrawState);
			if (pTool != NULL) {
				pTool->OnLButtonDown(pView, nFlags, point);
			}
		}
		m_pNewElement = NULL;
	}	

	pView->Connectdescription();
	CDrawTool::OnLButtonUp(pView, nFlags, point);

	if (ElementFlag != -1) {		//��ü ������ ��ü ���� �߰� ����
		if (ElementFlag == prov && pView->GetProvCount()+1 > 3) {
			pView->SetSituation(selection);			
		}
		else if (ElementFlag == acpower) {
			int ElementCount = pView->GetDocument()->GetElementCount(ElementFlag);
			if (ElementCount == 1) {
				pView->SetSituation(selection);			
			}
		}
		else {
			pView->InsertElement(ElementFlag);
		}
	}
}

void CElementTool::OnMouseMove( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	CPoint ConvertPos(point);
	CDrawTool::GridPostion(ConvertPos);
	CPoint PastPos(m_MovePoint);
	CDrawTool::GridPostion(PastPos);

	CClientDC dc(pView);
	pView->OnPrepareDC(&dc);
	dc.DPtoLP(&ConvertPos);
	dc.DPtoLP(&PastPos);

	if (m_pNewElement == NULL) {
		CElementFactory *pFactory = CMainFrame::CreateElementFactory();
		m_pNewElement = pFactory->CreateElement(m_InsertKind);				//��ü �����ؼ� ������������ ����
		CEzSpiceDoc *pDoc = pView->GetDocument();							//Doc�� m_CircultList�� ����
		pDoc->m_CircuitList.push_back(m_pNewElement);
		m_pNewElement->m_Position = ConvertPos;
		pView->SetActivityElement(m_pNewElement);

		if (m_pNewElement->m_Flag == prov) {
			
			ElementData NewData;
			NewData.Text = CString(_T("ProvColor"));
			NewData.Value = pView->NextProvColor();

			list<ElementData> SendList;
			SendList.push_back(NewData);
			m_pNewElement->OnEditProperties(&SendList);
		}
	}
	else {
		CPoint MovePoint(ConvertPos - PastPos);
		pView->MoveActivityElement(MovePoint);								//Ȱ��ȭ ��ü ��� �̵� �� �ٽ� �׸���
		pView->MoveActivityModityElement(MovePoint);						//Ȱ��ȭ ��ü ��� �̵� �� �ٽ� �׸���
	}
	pView->Invalidate(FALSE);
	m_MovePoint = point;
}

void CElementTool::OnEditProperties( CEzSpiceView* pView )
{

}




//////////////////////////////////// CLineTool ���ἱ �׸��� 

BOOL	CLineTool::m_MergerFlag = FALSE;
CBaseElement*	CLineTool::m_pLastElement = NULL;
CBaseElement*	CLineTool::m_pFirstDivideElement = NULL;
CBaseElement*	CLineTool::m_pSecondDivideElement = NULL;


CLineTool::CLineTool()
: CDrawTool(Connectline), m_DrawLineType(Yaxis)
{

}

CLineTool::~CLineTool()
{


}
void CLineTool::OnLButtonDown( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	CDrawTool::OnLButtonDown(pView, nFlags, point);

	if (m_pNewFirstLine == NULL) {
		CEzSpiceDoc *pDoc = pView->GetDocument();

		CPoint ConvertPos(point);
		CDrawTool::GridPostion(ConvertPos);

		CClientDC dc(pView);
		pView->OnPrepareDC(&dc);
		dc.DPtoLP(&ConvertPos);

		CBaseElement* pSelectElement = pDoc->IsObjected(ConvertPos);
		if (pSelectElement != NULL) {	//�ش� ��ġ�� �����ִ��� �˻�
			m_pNewElement = pSelectElement;

			CPoint ConnectPostion;
			if (pSelectElement->m_Flag != lines) {	//�����ϰ�� ���� ��ġ ȹ��

				if (pSelectElement->IsDuplicateConnectPoint(ConvertPos)) return;	//���� ���� �����̸� ����				
				ConnectPostion = pSelectElement->ConnectRectDirection(ConvertPos);
				if (ConnectPostion.x == 0 && ConnectPostion.y ==0 ) return;
			}

			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			m_pNewFirstLine = pFactory->CreateElement(lines);

			if (pSelectElement->m_Flag == lines) {		//���ο� �����
				
				m_DrawLineType = pView->GetAxis(pSelectElement);	//���� ���� ȹ���Ͽ� �ݴ�� ����
				CPoint LineConnectPosition = pSelectElement->ComparePosition(ConvertPos);
				if (LineConnectPosition.x != 0 && LineConnectPosition.y != 0) { //���γ����� �˻�
					
					m_pNewFirstLine->m_LineTo = LineConnectPosition;
					m_pNewFirstLine->m_MoveTo = LineConnectPosition;
					
					LinkConnect(pView, m_pNewFirstLine, LineConnectPosition);		
					m_DownPoint = LineConnectPosition;
				}	
				else {	//���� �߰�
					m_pFirstDivideElement = pSelectElement;

					//������ ������ ���ķ� �л�
					CElementFactory *pFactory = CMainFrame::CreateElementFactory();					//���ҵǴ� ���ο� ���μ��� ����	
					CBaseElement *pElement = pFactory->CreateElement(lines);	
					m_pSecondDivideElement = pElement;

					CPoint NewPosition = Near_LineToPoint(m_pFirstDivideElement->m_LineTo.x, m_pFirstDivideElement->m_LineTo.y,
						m_pFirstDivideElement->m_MoveTo.x, m_pFirstDivideElement->m_MoveTo.y, ConvertPos.x, ConvertPos.y);		

					m_pSecondDivideElement->m_MoveTo = m_pFirstDivideElement->m_MoveTo;
					m_pSecondDivideElement->m_LineTo = NewPosition;
					m_pFirstDivideElement->m_MoveTo = NewPosition;
					m_pNewFirstLine->m_LineTo = NewPosition;
					m_pNewFirstLine->m_MoveTo = NewPosition;

					DivideLinkConnect();	//��ũ ����

					pDoc->m_CircuitList.push_back(m_pSecondDivideElement);
					LinkConnect(pView, m_pNewFirstLine, NewPosition);					//���ο� ��ũ ����
					m_pFirstDivideElement->m_ConnectElement.push_back(m_pSecondDivideElement);
					m_pSecondDivideElement->m_ConnectElement.push_back(m_pFirstDivideElement);
					m_DownPoint = NewPosition;
				}
			}
			else {	//���ڿ� �����
				m_pNewFirstLine->m_LineTo = ConnectPostion;
				//LPtoDP; -> ConnectPosition;
				m_DownPoint = ConnectPostion;
				pSelectElement->m_ConnectElement.push_back(m_pNewFirstLine);			
				m_pNewFirstLine->m_ConnectElement.push_back(pSelectElement);
			}
			pDoc->m_CircuitList.push_back(m_pNewFirstLine);
		}
	}
}

void CLineTool::OnMouseMove( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	CPoint MovePos(point);
	CDrawTool::GridPostion(MovePos);

	CClientDC dc(pView);
	pView->OnPrepareDC(&dc);
	dc.DPtoLP(&MovePos);

	if (m_pNewFirstLine != NULL) {
		SensingConnect(pView, MovePos);

		CPoint DownPos(m_DownPoint);
		CDrawTool::GridPostion(DownPos);
		dc.DPtoLP(&DownPos);
		CEzSpiceDoc *pDoc =  pView->GetDocument();

		if (DownPos.x != MovePos.x && DownPos.y != MovePos.y) {						//x,y���� ��� ��ȭ ������	
			if (m_pNewFirstLine != NULL && m_pNewSecondLine == NULL) {				//���� ������ �ϳ��ΰ�?

				if (m_DrawLineType == Xaxis) 
					m_pNewFirstLine->m_MoveTo = CPoint(MovePos.x, DownPos.y);
				else 
					m_pNewFirstLine->m_MoveTo = CPoint(DownPos.x, MovePos.y);

				CElementFactory *pFactory = CMainFrame::CreateElementFactory();		//�ι�° �������� ����
				m_pNewSecondLine = pFactory->CreateElement(lines);	
				
				m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
				m_pNewSecondLine->m_MoveTo = MovePos;

				m_pNewFirstLine->m_ConnectElement.push_back(m_pNewSecondLine);
				m_pNewSecondLine->m_ConnectElement.push_back(m_pNewFirstLine);
				pDoc->m_CircuitList.push_back(m_pNewSecondLine);
				OnTabDown(pView);
			}
			else {	//���������� 2���϶� �� ����
				m_pNewSecondLine->m_MoveTo = MovePos;
				if (m_DrawLineType == Xaxis) {	
					m_pNewFirstLine->m_MoveTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
					m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
				}
				else {																			//y���̳�
					m_pNewFirstLine->m_MoveTo = CPoint(m_pNewFirstLine->m_LineTo.x, m_pNewSecondLine->m_MoveTo.y);
					m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
				}
			}
		}
		else { //�������� �̵���
			if (m_pNewSecondLine == NULL) {
				m_pNewFirstLine->m_MoveTo = MovePos;
			}
			else {
				m_pNewSecondLine->m_MoveTo = MovePos;
				if (m_pNewSecondLine->m_LineTo == m_pNewSecondLine->m_MoveTo) {
					pView->m_Connectdescription.clear();
					pDoc->DeleteElement(m_pNewSecondLine);
					m_pNewSecondLine = NULL;
				}
			}
		}
	}
	else {
		NonClickSensingConnect(pView, MovePos);
	}

	pView->Invalidate(FALSE);
	CDrawTool::OnMouseMove(pView, nFlags, point);
}


void CLineTool::OnLButtonUp( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{
	if (m_pNewFirstLine != NULL) {

		CEzSpiceDoc *pDoc =  pView->GetDocument();

		CPoint ConvertPos(point);
		CDrawTool::GridPostion(ConvertPos);

		CClientDC dc(pView);
		pView->OnPrepareDC(&dc);
		dc.DPtoLP(&ConvertPos);

		list<CBaseElement*> ExceptionList;
		ExceptionList.push_back(m_pNewFirstLine);
		if (m_pNewSecondLine != NULL) ExceptionList.push_back(m_pNewSecondLine);

		CBaseElement* pSelectElement = pDoc->IsObjected(ConvertPos, &ExceptionList);
		if (pSelectElement != NULL && pSelectElement != m_pNewElement) {	//���� ��ġ�� �ڽŰ� �ٸ� ���ڰ� �ֳ�?
				
			if (pSelectElement->m_Flag != lines) {	//�����϶�
				if (pSelectElement->IsDuplicateConnectPoint(ConvertPos)) return;	//���� ���� �����̸� ����		
				CPoint ConnectPostion = pSelectElement->ConnectRectDirection(ConvertPos);
				if (ConnectPostion.x == 0 && ConnectPostion.y ==0 ) return;
				
				//ȸ�� ����
				if (m_pNewSecondLine != NULL) {	//���ἱ�� �ΰ��϶�
					m_pNewSecondLine->m_MoveTo = ConnectPostion; //�� ����
					if (m_DrawLineType == Xaxis) {	
						m_pNewFirstLine->m_MoveTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
						m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
					}
					else {																			//y���̳�
						m_pNewFirstLine->m_MoveTo = CPoint(m_pNewFirstLine->m_LineTo.x, m_pNewSecondLine->m_MoveTo.y);
						m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
					}

					//���� ù��° ���ΰ� �ι�° ������ ���� �����ϸ� (��ġ ����) �ϳ��� ��ġ��..
					if (m_pNewFirstLine->m_LineTo.x == m_pNewSecondLine->m_MoveTo.x  && m_pNewFirstLine->m_LineTo.y != m_pNewSecondLine->m_MoveTo.y
						|| m_pNewFirstLine->m_LineTo.y == m_pNewSecondLine->m_MoveTo.y && m_pNewFirstLine->m_LineTo.x != m_pNewSecondLine->m_MoveTo.x ) {
						
							m_pNewFirstLine->m_MoveTo = ConnectPostion;
							pSelectElement->m_ConnectElement.push_back(m_pNewFirstLine);
							m_pNewFirstLine->m_ConnectElement.push_back(pSelectElement);

							pDoc->DeleteElement(m_pNewSecondLine);
					}
					else {
						pSelectElement->m_ConnectElement.push_back(m_pNewSecondLine);
						m_pNewSecondLine->m_ConnectElement.push_back(pSelectElement);
					}
				}
				else {
					m_pNewFirstLine->m_MoveTo = ConnectPostion;
					pSelectElement->m_ConnectElement.push_back(m_pNewFirstLine);
					m_pNewFirstLine->m_ConnectElement.push_back(pSelectElement);
				}	
			}
			else {	//�����϶�

				CPoint LineConnectPosition = pSelectElement->ComparePosition(ConvertPos);
				if (LineConnectPosition.x != 0 && LineConnectPosition.y != 0) { //���γ����� �˻�
					if (m_pNewSecondLine != NULL) {
						m_pNewSecondLine->m_MoveTo = LineConnectPosition;
						
						//�� ����
						if (m_DrawLineType == Xaxis) {	
							m_pNewFirstLine->m_MoveTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
							m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
						}
						else {																			//y���̳�
							m_pNewFirstLine->m_MoveTo = CPoint(m_pNewFirstLine->m_LineTo.x, m_pNewSecondLine->m_MoveTo.y);
							m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
						}

						//ù��° ��ü�� �ι�° ��ü�� ������ ��ġ�� 
						if (m_pNewFirstLine->m_LineTo.x == m_pNewSecondLine->m_MoveTo.x  && m_pNewFirstLine->m_LineTo.y != m_pNewSecondLine->m_MoveTo.y
							|| m_pNewFirstLine->m_LineTo.y == m_pNewSecondLine->m_MoveTo.y && m_pNewFirstLine->m_LineTo.x != m_pNewSecondLine->m_MoveTo.x ) {

								m_pNewFirstLine->m_MoveTo = LineConnectPosition;
								LinkConnect(pView, m_pNewFirstLine, LineConnectPosition);
								pDoc->DeleteElement(m_pNewSecondLine);
								m_pNewSecondLine = NULL;
						}
						else {
							LinkConnect(pView, m_pNewSecondLine, LineConnectPosition);
						}
						
					}
					else {
						m_pNewFirstLine->m_MoveTo = LineConnectPosition;
						LinkConnect(pView, m_pNewFirstLine, LineConnectPosition);
					}
				}	
				else {	//���� �߰�
					//��ü ����

					int SelectElementAxis = pView->GetAxis(pSelectElement);	//���� ���̸� ���� �Ƚ�Ŵ
					if (m_pNewSecondLine != NULL) {
						if (SelectElementAxis == pView->GetAxis(m_pNewSecondLine)) return;
					}
					else {
						if (SelectElementAxis == pView->GetAxis(m_pNewFirstLine)) return;
					}

					m_pFirstDivideElement = pSelectElement;

					CElementFactory *pFactory = CMainFrame::CreateElementFactory();					//���ҵǴ� ���ο� ���μ��� ����	
					CBaseElement *pElement = pFactory->CreateElement(lines);	
					m_pSecondDivideElement = pElement;

					CPoint NewPosition = Near_LineToPoint(m_pFirstDivideElement->m_LineTo.x, m_pFirstDivideElement->m_LineTo.y,
						m_pFirstDivideElement->m_MoveTo.x, m_pFirstDivideElement->m_MoveTo.y, ConvertPos.x, ConvertPos.y);	

					m_pSecondDivideElement->m_MoveTo = m_pFirstDivideElement->m_MoveTo;
					m_pSecondDivideElement->m_LineTo = NewPosition;
					m_pFirstDivideElement->m_MoveTo = NewPosition;
					DivideLinkConnect();	//��ũ ����

					if (m_pNewSecondLine != NULL) {
						pDoc->m_CircuitList.push_back(m_pSecondDivideElement);

						//���� ù��° ���ΰ� �ι�° ������ ���� �����ϸ� (��ġ ����) �ϳ��� ��ġ��..
						if (m_pNewFirstLine->m_LineTo.x == m_pNewSecondLine->m_MoveTo.x  && m_pNewFirstLine->m_LineTo.y != m_pNewSecondLine->m_MoveTo.y
							|| m_pNewFirstLine->m_LineTo.y == m_pNewSecondLine->m_MoveTo.y && m_pNewFirstLine->m_LineTo.x != m_pNewSecondLine->m_MoveTo.x ) {

								m_pNewFirstLine->m_MoveTo = NewPosition;
								LinkConnect(pView, m_pNewFirstLine, NewPosition);
								pDoc->DeleteElement(m_pNewSecondLine);
						}
						else {
							LinkConnect(pView, m_pNewSecondLine, NewPosition);
						}
					}
					else {
						m_pNewFirstLine->m_MoveTo = NewPosition;
						pDoc->m_CircuitList.push_back(m_pSecondDivideElement);
						LinkConnect(pView, m_pNewFirstLine, NewPosition);
					}
					m_pFirstDivideElement->m_ConnectElement.push_back(m_pSecondDivideElement);
					m_pSecondDivideElement->m_ConnectElement.push_back(m_pFirstDivideElement);
				}
			}	

			m_pNewFirstLine = NULL;
			m_pNewSecondLine = NULL;
			m_pNewElement = NULL;
			m_pLastElement = NULL;
			m_pFirstDivideElement = NULL;
			m_pSecondDivideElement = NULL;
			pView->GetDocument()->InsertReUndo();
		}
		else {
			//�� ù��° ���������� ���μ��ڿ� ���� ������ ����ǰ� �ι�° ���������� ù��°�� �̵���
			if (m_pNewSecondLine != NULL && pSelectElement != m_pNewElement) {
				m_pLastElement = m_pNewFirstLine;
				m_pNewFirstLine = m_pNewSecondLine;	

				m_pNewSecondLine = NULL;
				m_pFirstDivideElement = NULL;
				m_pSecondDivideElement = NULL;

				CPoint ConvertPos(m_pNewFirstLine->m_LineTo);
				dc.LPtoDP(&ConvertPos);
				m_DownPoint = ConvertPos;
			}
		}
	}
	pView->m_Connectdescription.clear();
	if (m_MergerFlag) {
		pView->GetDocument()->MergerConnectLine();
	}
	else m_MergerFlag = TRUE;

	pView->Invalidate(FALSE);
	CDrawTool::OnLButtonUp(pView, nFlags, point);
}


 void CLineTool::OnEditProperties( CEzSpiceView* pView )
{

}

void CLineTool::OnLButtonDblClk( CEzSpiceView* pView, UINT nFlags, const CPoint& point )
{

}

void CLineTool::OnTabDown( CEzSpiceView* pView )
{
	m_DrawLineType == Xaxis ? m_DrawLineType = Yaxis : m_DrawLineType = Xaxis;

	if (m_pNewFirstLine != NULL && m_pNewSecondLine != NULL) {
		if (m_DrawLineType == Xaxis) {	//x���̳�?
			m_pNewFirstLine->m_MoveTo = CPoint( m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
			m_pNewSecondLine->m_LineTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
		}
		else {																			//y���̳�
			m_pNewFirstLine->m_MoveTo = CPoint( m_pNewFirstLine->m_LineTo.x, m_pNewSecondLine->m_MoveTo.y);
			m_pNewSecondLine->m_LineTo = CPoint(m_pNewFirstLine->m_MoveTo.x, m_pNewSecondLine->m_MoveTo.y);
		}
	}
	pView->Invalidate(FALSE);	
}

CPoint CLineTool::Near_LineToPoint(double x1, double y1, double x2, double y2, double x,double y) 
{
	CPoint LinePos(0, 0);
	double segment_mag = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
	if(segment_mag != 0){		
		double u = ((x-x1)*(x2-x1)+(y-y1)*(y2-y1))/segment_mag ;
		double xp = x1+ u * (x2-x1);
		double yp = y1+ u * (y2-y1);
		LinePos.SetPoint((int)xp, (int)yp);
	}
	return LinePos;
}



void CLineTool::LinkConnect(CEzSpiceView *pView, CBaseElement* NewElement, CPoint &ComparePoint)	// ���� ȸ�ΰ� ������ �ϳ� �� �����ϸ� 3���� ���θ� �����Ѿ� �Ѵ�.
{
	CEzSpiceDoc *pDoc = pView->GetDocument();

	list<CBaseElement*>::iterator pos = pDoc->m_CircuitList.begin();
	while (pos != pDoc->m_CircuitList.end()) {
		if ((*pos) != NewElement) {
			if ((*pos)->m_LineTo == ComparePoint) {
				NewElement->m_ConnectElement.push_back((*pos));	
				(*pos)->m_ConnectElement.push_back(NewElement);
			
			}
			else if ((*pos)->m_MoveTo == ComparePoint) {
				NewElement->m_ConnectElement.push_back((*pos));
				(*pos)->m_ConnectElement.push_back(NewElement);
		
			}
		}
		++pos;
	}
}

void CLineTool::DivideLinkConnect()
{
	list<CBaseElement*> FirstDeleteList;
	//��� �ɰ���
	list<CBaseElement*>::iterator pos = m_pFirstDivideElement->m_ConnectElement.begin();
	while (pos != m_pFirstDivideElement->m_ConnectElement.end()) {

		if ((*pos)->m_Flag != lines) {
			BOOL nFlag = FALSE;
			list<CPoint>::iterator Connectpos = (*pos)->m_ConnectPoint.begin();
			while (Connectpos != (*pos)->m_ConnectPoint.end()) {
				if ((*pos)->m_Position + (*Connectpos) == m_pFirstDivideElement->m_MoveTo
					|| (*pos)->m_Position + (*Connectpos) == m_pFirstDivideElement->m_LineTo) {
						nFlag = TRUE;
				}
				++Connectpos;
			}

			if (!nFlag) {
				m_pSecondDivideElement->m_ConnectElement.push_back((*pos));
				(*pos)->m_ConnectElement.push_back(m_pSecondDivideElement);
				(*pos)->DeleteElement(m_pFirstDivideElement);
				FirstDeleteList.push_back((*pos));
			}
		}
		else {
			if ((*pos)->m_MoveTo == m_pFirstDivideElement->m_MoveTo || (*pos)->m_LineTo == m_pFirstDivideElement->m_LineTo
				|| (*pos)->m_MoveTo == m_pFirstDivideElement->m_LineTo || (*pos)->m_LineTo == m_pFirstDivideElement->m_MoveTo) {
					NULL;
			}
			else {
				m_pSecondDivideElement->m_ConnectElement.push_back((*pos));
				(*pos)->m_ConnectElement.push_back(m_pSecondDivideElement);
				(*pos)->DeleteElement(m_pFirstDivideElement);
				FirstDeleteList.push_back((*pos));
			}
		}
		++pos;
	}

	list<CBaseElement*>::iterator deletepos = FirstDeleteList.begin();
	while (deletepos != FirstDeleteList.end()) {
		m_pFirstDivideElement->DeleteElement((*deletepos));
		++deletepos;
	}
}

void CLineTool::SensingConnect(CEzSpiceView *pView, CPoint &point)
{
	CEzSpiceDoc *pDoc =  pView->GetDocument();
	pView->m_Connectdescription.clear();

	CBaseElement* pSelectElement = pDoc->IsObjected(point);	//�ش� ��ġ�� �����Ҽ� �ִٴ� ǥ�ø� �ϱ� ���� ��ü �˻�
	if (pSelectElement != NULL && pSelectElement->m_Flag == lines) {

		CPoint ConnerPoint = pSelectElement->ComparePosition(point);
		ConnerPoint.x != 0 && ConnerPoint.y != 0 ? pView->m_ConnerFlag = TRUE : pView->m_ConnerFlag = FALSE;

		if (m_pNewSecondLine != NULL) {	
			if (m_pNewSecondLine != pSelectElement) {
				pView->m_Connectdescription.push_back(point);
			}
		}
		else {
			if (m_pNewFirstLine != NULL && m_pNewFirstLine != pSelectElement) {
				pView->m_Connectdescription.push_back(point);	
			}
		}
	}
}

void CLineTool::NonClickSensingConnect(CEzSpiceView *pView, CPoint &point)
{
	CEzSpiceDoc *pDoc =  pView->GetDocument();
	pView->m_Connectdescription.clear();

	CBaseElement* pSelectElement = pDoc->IsObjected(point);	//�ش� ��ġ�� �����Ҽ� �ִٴ� ǥ�ø� �ϱ� ���� ��ü �˻�
	if (pSelectElement != NULL && pSelectElement->m_Flag == lines) {

		CPoint ConnerPoint = pSelectElement->ComparePosition(point);
		ConnerPoint.x != 0 && ConnerPoint.y != 0 ? pView->m_ConnerFlag = TRUE : pView->m_ConnerFlag = FALSE;
		pView->m_Connectdescription.push_back(point);
	}
}
