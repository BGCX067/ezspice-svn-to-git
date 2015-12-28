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

	if (pView->m_JunctionFlag) {	//교차점 추가 (객체 분할)
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
	
	CBaseElement* pSelectElement = pDoc->IsObjected(ConvertPos); //현재 위치에 객체가 있나?
	if (pSelectElement != NULL) {	
		
		pView->DecisionAxis();
		CPoint LineConnectPosition(0,0);
		if (pSelectElement->m_Flag == lines) {
			LineConnectPosition = pSelectElement->ComparePosition(ConvertPos);
		}

		if (pView->IsSelected(pSelectElement) == FALSE) {
			if ((nFlags & MK_CONTROL) == 0) {				//Ctrl 안눌렀으면 모두 비활성화
				pView->m_SelectElement.clear();
				pView->m_SelectModityElement.clear();
				pView->Invalidate(FALSE);
			}
			if (LineConnectPosition.x != 0 && LineConnectPosition.y != 0) {	//선택좌표가 객체 라인의 끝점일때
				pView->m_SelectElement.clear();	//객체 수정은 한개만 가능하도록 진행
				pView->m_SelectModityElement.clear();

				pView->SetActivityModityElement(LineConnectPosition);	//m_SelelctModityElement에 삽입후 해당 객체 활성화
			}
			else {
				pView->SetActivityElement(pSelectElement);			//m_SelelctElement에 삽입후 해당 객체 활성화
			}
		}
		else {				//이미 활성화 상태인거 다시 눌름 
			if ((nFlags & MK_CONTROL) != 0) {	//Ctrl 눌르고 눌르면 다시 비활성화
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
	//경로 쪼개기
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

	if (pView->GetCapture() != pView) {		//GetCapture()함수는 현재 뷰에 활성화 되어있는지 체크한다. 및 영역 벗어남 체크
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
			pView->MoveActivityElement(MovePoint);				//활성화 객체 모두 이동 및 다시 그리기
			pView->MoveActivityModityElement(MovePoint);		//활성화 객체 모두 이동 및 다시 그리기
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
			pView->SetActivityInRect(CPoint(CompareRect.left, CompareRect.top), CPoint(CompareRect.right, CompareRect.bottom));			//사각 영역 안에 있는 객체 모두 활성화
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






/////////////////////////////////////  CElementTool  소자 객체 생성을 요청하여 삽입




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
			AfxMessageBox(_T("이미 소자가 존재합니다."));
			return;
		}
		else {
			ElementFlag = m_pNewElement->m_Flag;
		}

		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->UpdateClassView(pView->GetDocument());
 		pView->GetDocument()->InsertReUndo();
		ProvProcessing(pView);

		if (m_pNewElement->m_Flag == ground) { //그라운드 예외처리 (그라운드 삽입시 바로 연결라인 생성)
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

	if (ElementFlag != -1) {		//객체 삽입후 객체 다중 추가 실행
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
		m_pNewElement = pFactory->CreateElement(m_InsertKind);				//객체 생성해서 전역변수에만 저장
		CEzSpiceDoc *pDoc = pView->GetDocument();							//Doc에 m_CircultList에 저장
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
		pView->MoveActivityElement(MovePoint);								//활성화 객체 모두 이동 및 다시 그리기
		pView->MoveActivityModityElement(MovePoint);						//활성화 객체 모두 이동 및 다시 그리기
	}
	pView->Invalidate(FALSE);
	m_MovePoint = point;
}

void CElementTool::OnEditProperties( CEzSpiceView* pView )
{

}




//////////////////////////////////// CLineTool 연결선 그리기 

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
		if (pSelectElement != NULL) {	//해당 위치에 소자있는지 검사
			m_pNewElement = pSelectElement;

			CPoint ConnectPostion;
			if (pSelectElement->m_Flag != lines) {	//소자일경우 연결 위치 획득

				if (pSelectElement->IsDuplicateConnectPoint(ConvertPos)) return;	//만약 다중 연결이면 해제				
				ConnectPostion = pSelectElement->ConnectRectDirection(ConvertPos);
				if (ConnectPostion.x == 0 && ConnectPostion.y ==0 ) return;
			}

			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			m_pNewFirstLine = pFactory->CreateElement(lines);

			if (pSelectElement->m_Flag == lines) {		//라인에 연결시
				
				m_DrawLineType = pView->GetAxis(pSelectElement);	//현재 축을 획득하여 반대로 변경
				CPoint LineConnectPosition = pSelectElement->ComparePosition(ConvertPos);
				if (LineConnectPosition.x != 0 && LineConnectPosition.y != 0) { //라인끝인지 검사
					
					m_pNewFirstLine->m_LineTo = LineConnectPosition;
					m_pNewFirstLine->m_MoveTo = LineConnectPosition;
					
					LinkConnect(pView, m_pNewFirstLine, LineConnectPosition);		
					m_DownPoint = LineConnectPosition;
				}	
				else {	//라인 중간
					m_pFirstDivideElement = pSelectElement;

					//선택한 라인을 병렬로 분산
					CElementFactory *pFactory = CMainFrame::CreateElementFactory();					//분할되는 새로운 라인소자 생성	
					CBaseElement *pElement = pFactory->CreateElement(lines);	
					m_pSecondDivideElement = pElement;

					CPoint NewPosition = Near_LineToPoint(m_pFirstDivideElement->m_LineTo.x, m_pFirstDivideElement->m_LineTo.y,
						m_pFirstDivideElement->m_MoveTo.x, m_pFirstDivideElement->m_MoveTo.y, ConvertPos.x, ConvertPos.y);		

					m_pSecondDivideElement->m_MoveTo = m_pFirstDivideElement->m_MoveTo;
					m_pSecondDivideElement->m_LineTo = NewPosition;
					m_pFirstDivideElement->m_MoveTo = NewPosition;
					m_pNewFirstLine->m_LineTo = NewPosition;
					m_pNewFirstLine->m_MoveTo = NewPosition;

					DivideLinkConnect();	//링크 분할

					pDoc->m_CircuitList.push_back(m_pSecondDivideElement);
					LinkConnect(pView, m_pNewFirstLine, NewPosition);					//새로운 링크 연결
					m_pFirstDivideElement->m_ConnectElement.push_back(m_pSecondDivideElement);
					m_pSecondDivideElement->m_ConnectElement.push_back(m_pFirstDivideElement);
					m_DownPoint = NewPosition;
				}
			}
			else {	//소자에 연결시
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

		if (DownPos.x != MovePos.x && DownPos.y != MovePos.y) {						//x,y축이 모두 변화 있을때	
			if (m_pNewFirstLine != NULL && m_pNewSecondLine == NULL) {				//라인 정보가 하나인가?

				if (m_DrawLineType == Xaxis) 
					m_pNewFirstLine->m_MoveTo = CPoint(MovePos.x, DownPos.y);
				else 
					m_pNewFirstLine->m_MoveTo = CPoint(DownPos.x, MovePos.y);

				CElementFactory *pFactory = CMainFrame::CreateElementFactory();		//두번째 라인정보 생성
				m_pNewSecondLine = pFactory->CreateElement(lines);	
				
				m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
				m_pNewSecondLine->m_MoveTo = MovePos;

				m_pNewFirstLine->m_ConnectElement.push_back(m_pNewSecondLine);
				m_pNewSecondLine->m_ConnectElement.push_back(m_pNewFirstLine);
				pDoc->m_CircuitList.push_back(m_pNewSecondLine);
				OnTabDown(pView);
			}
			else {	//라인정보가 2개일때 값 갱신
				m_pNewSecondLine->m_MoveTo = MovePos;
				if (m_DrawLineType == Xaxis) {	
					m_pNewFirstLine->m_MoveTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
					m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
				}
				else {																			//y축이냐
					m_pNewFirstLine->m_MoveTo = CPoint(m_pNewFirstLine->m_LineTo.x, m_pNewSecondLine->m_MoveTo.y);
					m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
				}
			}
		}
		else { //한축으로 이동시
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
		if (pSelectElement != NULL && pSelectElement != m_pNewElement) {	//현재 위치에 자신과 다른 소자가 있나?
				
			if (pSelectElement->m_Flag != lines) {	//소자일때
				if (pSelectElement->IsDuplicateConnectPoint(ConvertPos)) return;	//만약 다중 연결이면 해제		
				CPoint ConnectPostion = pSelectElement->ConnectRectDirection(ConvertPos);
				if (ConnectPostion.x == 0 && ConnectPostion.y ==0 ) return;
				
				//회로 연결
				if (m_pNewSecondLine != NULL) {	//연결선이 두개일때
					m_pNewSecondLine->m_MoveTo = ConnectPostion; //값 보정
					if (m_DrawLineType == Xaxis) {	
						m_pNewFirstLine->m_MoveTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
						m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
					}
					else {																			//y축이냐
						m_pNewFirstLine->m_MoveTo = CPoint(m_pNewFirstLine->m_LineTo.x, m_pNewSecondLine->m_MoveTo.y);
						m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
					}

					//만약 첫번째 라인과 두번째 라인의 축이 동일하면 (위치 동일) 하나로 합치기..
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
			else {	//라인일때

				CPoint LineConnectPosition = pSelectElement->ComparePosition(ConvertPos);
				if (LineConnectPosition.x != 0 && LineConnectPosition.y != 0) { //라인끝인지 검사
					if (m_pNewSecondLine != NULL) {
						m_pNewSecondLine->m_MoveTo = LineConnectPosition;
						
						//값 보정
						if (m_DrawLineType == Xaxis) {	
							m_pNewFirstLine->m_MoveTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
							m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
						}
						else {																			//y축이냐
							m_pNewFirstLine->m_MoveTo = CPoint(m_pNewFirstLine->m_LineTo.x, m_pNewSecondLine->m_MoveTo.y);
							m_pNewSecondLine->m_LineTo = m_pNewFirstLine->m_MoveTo;
						}

						//첫번째 객체와 두번째 객체가 같으면 합치기 
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
				else {	//라인 중간
					//객체 분할

					int SelectElementAxis = pView->GetAxis(pSelectElement);	//같은 축이면 분할 안시킴
					if (m_pNewSecondLine != NULL) {
						if (SelectElementAxis == pView->GetAxis(m_pNewSecondLine)) return;
					}
					else {
						if (SelectElementAxis == pView->GetAxis(m_pNewFirstLine)) return;
					}

					m_pFirstDivideElement = pSelectElement;

					CElementFactory *pFactory = CMainFrame::CreateElementFactory();					//분할되는 새로운 라인소자 생성	
					CBaseElement *pElement = pFactory->CreateElement(lines);	
					m_pSecondDivideElement = pElement;

					CPoint NewPosition = Near_LineToPoint(m_pFirstDivideElement->m_LineTo.x, m_pFirstDivideElement->m_LineTo.y,
						m_pFirstDivideElement->m_MoveTo.x, m_pFirstDivideElement->m_MoveTo.y, ConvertPos.x, ConvertPos.y);	

					m_pSecondDivideElement->m_MoveTo = m_pFirstDivideElement->m_MoveTo;
					m_pSecondDivideElement->m_LineTo = NewPosition;
					m_pFirstDivideElement->m_MoveTo = NewPosition;
					DivideLinkConnect();	//링크 분할

					if (m_pNewSecondLine != NULL) {
						pDoc->m_CircuitList.push_back(m_pSecondDivideElement);

						//만약 첫번째 라인과 두번째 라인의 축이 동일하면 (위치 동일) 하나로 합치기..
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
			//즉 첫번째 라인정보는 라인소자에 현재 값으로 저장되고 두번째 라인정보가 첫번째로 이동됨
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
		if (m_DrawLineType == Xaxis) {	//x축이냐?
			m_pNewFirstLine->m_MoveTo = CPoint( m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
			m_pNewSecondLine->m_LineTo = CPoint(m_pNewSecondLine->m_MoveTo.x, m_pNewFirstLine->m_LineTo.y);
		}
		else {																			//y축이냐
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



void CLineTool::LinkConnect(CEzSpiceView *pView, CBaseElement* NewElement, CPoint &ComparePoint)	// ㄱ자 회로가 있을시 하나 더 연결하면 3개가 서로를 가리켜야 한다.
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
	//경로 쪼개기
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

	CBaseElement* pSelectElement = pDoc->IsObjected(point);	//해당 위치에 연결할수 있다는 표시를 하기 위한 객체 검색
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

	CBaseElement* pSelectElement = pDoc->IsObjected(point);	//해당 위치에 연결할수 있다는 표시를 하기 위한 객체 검색
	if (pSelectElement != NULL && pSelectElement->m_Flag == lines) {

		CPoint ConnerPoint = pSelectElement->ComparePosition(point);
		ConnerPoint.x != 0 && ConnerPoint.y != 0 ? pView->m_ConnerFlag = TRUE : pView->m_ConnerFlag = FALSE;
		pView->m_Connectdescription.push_back(point);
	}
}
