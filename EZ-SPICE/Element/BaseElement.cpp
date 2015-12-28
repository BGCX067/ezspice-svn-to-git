#include "StdAfx.h"
#include "BaseElement.h"
#include "EditPropertyDlg.h"

enum DrawStates {
	selection, Connectline, insert
};

enum ProvColor
{
	darkred, darkgreen, darkyellow, darkblue,
};

enum Direction
{
	up, down, left, right
};

int	CBaseElement::m_DrawState = selection;
#define PIE 3.1415929

CBaseElement::CBaseElement(int Flag)
{
	m_ConnectPointArea = 4;
	m_Flag = Flag;
	m_RotationFlag = FALSE;
	m_GroundFlag = FALSE;
	m_ProvActivityFlag = FALSE;
	m_AntidromicFlag = FALSE;
}


CBaseElement::~CBaseElement( void )
{

}

void CBaseElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
}

BOOL CBaseElement::ObjectInPoint( const CPoint& point )
{
	if (m_Flag != lines) {
		if (point.x > m_Position.x - m_PositionArea.left && point.x < m_Position.x + m_PositionArea.right
			&& point.y > m_Position.y - m_PositionArea.top && point.y < m_Position.y + m_PositionArea.bottom) {
				return TRUE;
		}
	}
	else {	//라인일경우
		CPoint ConvertPos(point);
		CPoint LeftTop(m_LineTo.x < m_MoveTo.x ? m_LineTo.x - 2*m_ConnectPointArea : m_MoveTo.x - 2*m_ConnectPointArea
			, m_LineTo.y < m_MoveTo.y ? m_LineTo.y - 2*m_ConnectPointArea : m_MoveTo.y - 2*m_ConnectPointArea);
		CPoint RightBottom(m_LineTo.x > m_MoveTo.x ? m_LineTo.x + 2*m_ConnectPointArea : m_MoveTo.x + 2*m_ConnectPointArea
			, m_LineTo.y > m_MoveTo.y ? m_LineTo.y + 2*m_ConnectPointArea : m_MoveTo.y + 2*m_ConnectPointArea);

		if (ConvertPos.x > LeftTop.x && ConvertPos.x < RightBottom.x && ConvertPos.y > LeftTop.y && ConvertPos.y < RightBottom.y) {
			double Distance = Distance_LineToPoint(m_LineTo.x, m_LineTo.y, m_MoveTo.x, m_MoveTo.y, ConvertPos.x, ConvertPos.y);
			if (Distance < 7) {
				return TRUE;
			}
		}
	}
	return FALSE;
}


double CBaseElement::Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y) 
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

BOOL CBaseElement::IsDuplicateConnectPoint(CPoint &point)
{
	//현재 위치에 라인이 이미 연결되어있는지 확인
	list<CBaseElement*>::iterator linepos = m_ConnectElement.begin();
	while (linepos != m_ConnectElement.end()) {

		if ((*linepos)->m_Flag == lines) {
			if ((*linepos)->m_LineTo == point + m_Position || (*linepos)->m_MoveTo == point + m_Position) {
				return TRUE;				
			}
		}
		++linepos;
	}
	return FALSE;
}

CPoint CBaseElement::ConnectRectDirection(const CPoint& point)
{
	list<CPoint>::iterator pos = m_ConnectPoint.begin();	//연결 구간
	while (pos != m_ConnectPoint.end()) {

		if (point.x > m_Position.x + (*pos).x - 2*m_ConnectPointArea && point.x < m_Position.x + (*pos).x + 2*m_ConnectPointArea 
			&& point.y > m_Position.y + (*pos).y - 2*m_ConnectPointArea &&  point.y < m_Position.y + (*pos).y + 2*m_ConnectPointArea) {
				return CPoint(m_Position.x + (*pos).x, m_Position.y + (*pos).y);
		}
		++pos;
	}	
	return CPoint(0,0);
}

CPoint CBaseElement::ComparePosition( CPoint &pos )
{
	if (pos.x > m_MoveTo.x - 2*m_ConnectPointArea && pos.x < m_MoveTo.x + 2*m_ConnectPointArea
		&& pos.y > m_MoveTo.y - 2*m_ConnectPointArea && pos.y < m_MoveTo.y + 2*m_ConnectPointArea) {
			return m_MoveTo;	
	}

	if (pos.x > m_LineTo.x - 2*m_ConnectPointArea && pos.x < m_LineTo.x + 2*m_ConnectPointArea
		&& pos.y > m_LineTo.y - 2*m_ConnectPointArea && pos.y < m_LineTo.y + 2*m_ConnectPointArea) {
			return m_LineTo;
	}
	return CPoint(0,0);
}


void CBaseElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/)
{
	Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
	Pen RedPen(Color(255, 255, 0 , 0), 1.0f);
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
	}


	list<CPoint>::iterator pos = m_ConnectPoint.begin();	//연결 구간
	while (pos != m_ConnectPoint.end()) {

		BOOL nFlag = FALSE;
		list<CBaseElement*>::iterator connectpos = m_ConnectElement.begin();
		while (connectpos != m_ConnectElement.end()) {
			if ((*connectpos)->m_Flag == lines && 
				((*connectpos)->m_LineTo == m_Position + (*pos) || (*connectpos)->m_MoveTo == m_Position + (*pos))) {
					nFlag = TRUE;
					break;
			}
			++connectpos;
		}

		if (!nFlag) { 
			if (Collection.LineFlag) {
				SolidBrush NewBrush(Color(255, 255, 0 , 0));
				pDC->FillRectangle(&NewBrush, m_Position.x + (*pos).x -m_ConnectPointArea, 
					m_Position.y + (*pos).y -m_ConnectPointArea, 2*m_ConnectPointArea, 2*m_ConnectPointArea);	
			}
			else {
				pDC->DrawRectangle(&NewPen, m_Position.x + (*pos).x -m_ConnectPointArea/2, m_Position.y + (*pos).y-m_ConnectPointArea/2
					, m_ConnectPointArea, m_ConnectPointArea);	
			}
		}
		else {
			if ((*pos).x != 0) {
				pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -(*pos).x/10, m_Position.y + (*pos).y -2, m_Position.x + (*pos).x -(*pos).x/10, m_Position.y + (*pos).y +2);
				pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -((*pos).x/10)*2, m_Position.y + (*pos).y -2, m_Position.x + (*pos).x -((*pos).x/10)*2, m_Position.y + (*pos).y +2);
			}
			else {
				pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -2, m_Position.y + (*pos).y -(*pos).y/10, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y -(*pos).y/10);
				pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -2, m_Position.y + (*pos).y -((*pos).y/10)*2, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y -((*pos).y/10)*2);
			}
		}
		++pos;
	}
}

int CBaseElement::GetPositonDirection( CPoint point )
{
	if (m_Flag == lines) {
		if (m_MoveTo == point) {
			return MoveToConnect;
		}

		if (m_LineTo == point) {
			return LineToConnect;
		}
	}
	return -1;
}


void CBaseElement::DeleteElement( CBaseElement *pElement )
{
	list<CBaseElement*>::iterator pos = find(m_ConnectElement.begin(), m_ConnectElement.end(), pElement);	
	m_ConnectElement.erase(pos);

	list<CBaseElement*>::iterator inputpos = find(m_InputElement.begin(), m_InputElement.end(), pElement);	
	if (inputpos != m_InputElement.end()) {
		m_InputElement.erase(inputpos);
	}

	list<CBaseElement*>::iterator outputpos = find(m_OutputElement.begin(), m_OutputElement.end(), pElement);	
	if (outputpos != m_OutputElement.end()) {
		m_OutputElement.erase(outputpos);
	}
}

CBaseElement* CBaseElement::Clone()
{
	return NULL;
}

void CBaseElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{

}

int CBaseElement::ConnectPositionDirection( const CPoint& point )
{
	int iCount = 0;
	list<CPoint>::iterator pos = m_ConnectPoint.begin();	//연결 구간
	while (pos != m_ConnectPoint.end()) {

		if (point.x > m_Position.x + (*pos).x - 2*m_ConnectPointArea && point.x < m_Position.x + (*pos).x + 2*m_ConnectPointArea 
			&& point.y > m_Position.y + (*pos).y - 2*m_ConnectPointArea &&  point.y < m_Position.y + (*pos).y + 2*m_ConnectPointArea) {
				break;
		}
		++iCount;
		++pos;
	}	

	if (m_Flag == transistor) {
		return iCount == m_ConnectPoint.size() ? -1 : iCount == 0 ? outputposition :inputposition;
	}

	return iCount == m_ConnectPoint.size() ? -1 : iCount == 0 ? outputposition :inputposition;
}

void CBaseElement::RotationElement()
{
	if (m_ConnectElement.empty()) {

		list<CPoint>::iterator pos = m_ConnectPoint.begin();
		while (pos != m_ConnectPoint.end()) {
			if ((*pos).x != 0 && (*pos).y == 0) {
				(*pos).y = (*pos).x;
				(*pos).x = 0;
			}
			else if ((*pos).y != 0 && (*pos).x == 0) {
				(*pos).x = (*pos).y;
				(*pos).y = 0;
			}
			++pos;
		}

		if (m_Flag == diode) {
			m_RotationFlag +1 > Angle270 ? m_RotationFlag = Angle0 : m_RotationFlag++;

			if (m_RotationFlag == Angle0 || m_RotationFlag == Angle180) {
				reverse(m_ConnectPoint.begin(), m_ConnectPoint.end());	
			}
		}
		else {
			m_RotationFlag =  !m_RotationFlag;
		}
	}
}



///////////////////////////////////////////// CConnectLine

CConnectLine::CConnectLine()
:CBaseElement(lines)
{
	m_Name = CString(_T("Line"));	
}

CConnectLine::~CConnectLine()
{
	//모든 라인 및 해당 라인 연결된 소자들에서의 연결정보도 제거 
}

void CConnectLine::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/)
{
	if (Modifypoint.x == 0 && Modifypoint.y == 0) {
		Pen NewPen(Color(255, 0, 0 , 0), 1.0f);

		if (Collection.Selectflag) {
			NewPen.SetColor(Color(255, 255, 0 , 0));
			SolidBrush NewBrush(Color(255, 255, 0 , 0));
			pDC->FillRectangle(&NewBrush, (float)m_LineTo.x - m_ConnectPointArea, (float)m_LineTo.y - m_ConnectPointArea, (float)2*m_ConnectPointArea, (float)2*m_ConnectPointArea);
			pDC->FillRectangle(&NewBrush, (float)m_MoveTo.x - m_ConnectPointArea, (float)m_MoveTo.y - m_ConnectPointArea, (float)2*m_ConnectPointArea, (float)2*m_ConnectPointArea);
		}

		if (m_GroundFlag && Collection.GroundAreaFlag) {
			NewPen.SetColor(Color(255, 0, 255, 0));
		}
		pDC->DrawLine(&NewPen, m_LineTo.x, m_LineTo.y, m_MoveTo.x, m_MoveTo.y);

	}
	else {	//라인 수정 모드로 그리기
		SolidBrush NewBrush(Color(255, 255, 0 , 0));
		if (Modifypoint == m_LineTo) {
			pDC->FillRectangle(&NewBrush, (float)m_LineTo.x - m_ConnectPointArea, (float)m_LineTo.y - m_ConnectPointArea, (float)2*m_ConnectPointArea, (float)2*m_ConnectPointArea);
			pDC->FillEllipse(&NewBrush, RectF((float)m_MoveTo.x -2, (float)m_MoveTo.y -2, 4.0f, 4.0f));

		}
		else {
			pDC->FillRectangle(&NewBrush, (float)m_MoveTo.x - m_ConnectPointArea, (float)m_MoveTo.y - m_ConnectPointArea, (float)2*m_ConnectPointArea, (float)2*m_ConnectPointArea);
			pDC->FillEllipse(&NewBrush, RectF((float)m_LineTo.x -2, (float)m_LineTo.y -2, 4.0f, 4.0f));
		}
		Pen NewPen(Color(255, 255, 0 , 0), 1.0f);
		pDC->DrawLine(&NewPen, m_LineTo.x, m_LineTo.y, m_MoveTo.x, m_MoveTo.y);
	}

	if (Collection.LineStringFlag) {
		Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
		SolidBrush sbrush(Color::Black);
		StringFormat format;
		format.SetHotkeyPrefix(HotkeyPrefixShow);

		CString TestString;
		TestString.Format(_T("L%d"), m_ElementNumber);

		if (m_LineTo.x == m_MoveTo.x && m_MoveTo.y != m_LineTo.y ) {
			int Distance = abs(m_MoveTo.y - m_LineTo.y)/2;
			if (m_MoveTo.y > m_LineTo.y) {
				pDC->DrawString(TestString, -1, &font, RectF((float)m_LineTo.x -23, (float)m_LineTo.y + Distance -7, (float)m_LineTo.x -20, (float)m_LineTo.y + 7), &format, &sbrush);
			}
			else {
				pDC->DrawString(TestString, -1, &font, RectF((float)m_MoveTo.x -23, (float)m_MoveTo.y + Distance -7, (float)m_MoveTo.x -20, (float)m_MoveTo.y + 7), &format, &sbrush);
			}
		}
		else {
			int Distance = abs(m_LineTo.x - m_MoveTo.x)/2;
			if (m_MoveTo.x > m_LineTo.x) {
				pDC->DrawString(TestString, -1, &font, RectF((float)m_LineTo.x + Distance -7, (float)m_LineTo.y -20, (float)m_LineTo.x +7, (float)m_LineTo.y +20), &format, &sbrush);
			}
			else {
				pDC->DrawString(TestString, -1, &font, RectF((float)m_MoveTo.x + Distance -7, (float)m_MoveTo.y -20, (float)m_MoveTo.x +7, (float)m_MoveTo.y +20), &format, &sbrush);
			}
		}
	}

	if (Collection.DecideFlag) {
		//임시 라인 연결 //현재 위치 중앙에서 output중앙으로
		if (m_OutputElement.size()> 0) {
			Pen NewPen(Color(255, 255, 0 , 0), 2.0f);
			SolidBrush NewBrush(Color(255, 255, 0 , 0));
		
 			CPoint Gap = (m_LineTo - m_MoveTo);
			Gap.x = abs(Gap.x) /2;
			Gap.y = abs(Gap.y) /2;
			CPoint CenterPosition;

			int SmallGap = Gap.x != 0 ? Gap.x/2 : Gap.y/2;
			
			//중심위치 확정
			if (m_LineTo.x == m_MoveTo.x && m_LineTo.y != m_MoveTo.y) {
				m_LineTo.y > m_MoveTo.y ? CenterPosition = m_MoveTo + Gap : CenterPosition = m_LineTo + Gap;
			}
			else {
				m_LineTo.x > m_MoveTo.x ? CenterPosition = m_MoveTo + Gap : CenterPosition = m_LineTo + Gap;
			}

			//방향 확정
			CPoint OutputPos;
			list<CBaseElement*>::iterator linepos = m_OutputElement.begin();
			while (linepos != m_OutputElement.end()) {
				
				if ((*linepos)->m_Flag == lines) {
					if (m_LineTo == (*linepos)->m_LineTo || m_LineTo == (*linepos)->m_MoveTo) {
						OutputPos = m_LineTo;
					}
					else {
						OutputPos = m_MoveTo;
					}
				}
				else {
					list<CPoint>::iterator connectpos = (*linepos)->m_ConnectPoint.begin();
					while (connectpos != (*linepos)->m_ConnectPoint.end()) {
						if (m_LineTo == (*linepos)->m_Position + (*connectpos)) {
							OutputPos = m_LineTo;
						}
						else if (m_MoveTo == (*linepos)->m_Position + (*connectpos)){
							OutputPos = m_MoveTo;
						}
						++connectpos;
					}
				}
				++linepos;
			}

			int OutputDirection;
			if (CenterPosition.x == OutputPos.x && CenterPosition.y != OutputPos.y) {
				CenterPosition.y > OutputPos.y ? OutputDirection = up : OutputDirection = down;
			}
			else {
				CenterPosition.x > OutputPos.x ? OutputDirection = left : OutputDirection = right;
			}

			int DirectGap = 5;
			switch (OutputDirection) {
			case up:
				{
					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y +DirectGap, CenterPosition.x, CenterPosition.y);
					pDC->DrawLine(&NewPen, CenterPosition.x +DirectGap, CenterPosition.y +DirectGap, CenterPosition.x, CenterPosition.y);

					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y, CenterPosition.x, CenterPosition.y -DirectGap);
					pDC->DrawLine(&NewPen, CenterPosition.x +DirectGap, CenterPosition.y, CenterPosition.x, CenterPosition.y -DirectGap);

				}
				break;

			case down:
				{
					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y -DirectGap, CenterPosition.x, CenterPosition.y);
					pDC->DrawLine(&NewPen, CenterPosition.x +DirectGap, CenterPosition.y -DirectGap, CenterPosition.x, CenterPosition.y);

					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y, CenterPosition.x, CenterPosition.y +DirectGap);
					pDC->DrawLine(&NewPen, CenterPosition.x +DirectGap, CenterPosition.y, CenterPosition.x, CenterPosition.y +DirectGap);
				}
				break;

			case left:
				{
					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y, CenterPosition.x, CenterPosition.y -DirectGap);
					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y, CenterPosition.x, CenterPosition.y +DirectGap);

					pDC->DrawLine(&NewPen, CenterPosition.x, CenterPosition.y, CenterPosition.x +DirectGap, CenterPosition.y -DirectGap);
					pDC->DrawLine(&NewPen, CenterPosition.x, CenterPosition.y, CenterPosition.x +DirectGap, CenterPosition.y +DirectGap);
				}
				break;

			case right:
				{
					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y -DirectGap, CenterPosition.x, CenterPosition.y);
					pDC->DrawLine(&NewPen, CenterPosition.x -DirectGap, CenterPosition.y +DirectGap, CenterPosition.x, CenterPosition.y);

					pDC->DrawLine(&NewPen, CenterPosition.x, CenterPosition.y -DirectGap, CenterPosition.x +DirectGap, CenterPosition.y);
					pDC->DrawLine(&NewPen, CenterPosition.x, CenterPosition.y +DirectGap, CenterPosition.x +DirectGap, CenterPosition.y);
				}
				break;
			}
 		}
	}

	if (m_ConnectElement.size() > 2) {	//3개 이상 병렬구간 표시
		SolidBrush NewBrush(Color(255, 255, 0 , 0));
		int LintToCount = 0, MoveToCount = 0;
		list<CBaseElement*>::iterator pos = m_ConnectElement.begin();
		while (pos != m_ConnectElement.end()) {

			if ((*pos)->m_MoveTo == m_MoveTo || (*pos)->m_LineTo == m_MoveTo) {
				MoveToCount++;
			}

			if ((*pos)->m_MoveTo == m_LineTo || (*pos)->m_LineTo == m_LineTo) {
				LintToCount++;
			}
			++pos;
		}

		if (LintToCount > 1) pDC->FillEllipse(&NewBrush, RectF((float)m_LineTo.x -2, (float)m_LineTo.y -2, 4.0f, 4.0f));
		if (MoveToCount > 1) pDC->FillEllipse(&NewBrush, RectF((float)m_MoveTo.x -2, (float)m_MoveTo.y -2, 4.0f, 4.0f));
	}
}

void CConnectLine::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{

}

CBaseElement* CConnectLine::Clone() 
{
	CConnectLine *pCloneElement = new CConnectLine;
	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	return pCloneElement;
}

void CConnectLine::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{

}

void CConnectLine::RotationElement()
{

}


//////////////////////////////////////////////// CPowerElement


CPowerElementDC::CPowerElementDC()
:CBaseElement(dcpower)
{
	//자식 객체의 멤버 초기화 필요( 초기값)

	m_InternalResistance = 1.0f;
	m_Voltage = 5.0f;
}


CPowerElementDC::~CPowerElementDC( void )
{

}

void CPowerElementDC::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/)
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 0 , 255), 1.0f);
	Pen NewBoldPen (Color(255, 0, 0 , 0),1.7f);
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		NewBoldPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
	}	

	//원래 각도
	pDC->DrawLine(&NewBoldPen, m_Position.x -5, m_Position.y -2, m_Position.x +5, m_Position.y -2);
	pDC->DrawLine(&NewPen, m_Position.x -10, m_Position.y -6, m_Position.x +10, m_Position.y -6);
	pDC->DrawLine(&NewPen, m_Position.x , m_Position.y -6, m_Position.x , m_Position.y -20);

	pDC->DrawLine(&NewPen, m_Position.x -10, m_Position.y +2, m_Position.x +10, m_Position.y +2);
	pDC->DrawLine(&NewBoldPen, m_Position.x -5, m_Position.y +6, m_Position.x +5, m_Position.y +6);
	pDC->DrawLine(&NewPen, m_Position.x , m_Position.y +6, m_Position.x , m_Position.y +20);

	CString NewString = m_Name;
	CString PowerData;
	PowerData.Format(_T("  %.0fV"), m_Voltage);
	NewString += PowerData;
	pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x +7, (float)m_Position.y -20, (float)m_Position.x +7, (float)m_Position.y -20), &format, &sbrush);

	CBaseElement::Draw(pDC, Collection, Modifypoint);
}


void CPowerElementDC::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("Voltage"))) {
				m_Voltage = (*pos).Value;
				break;
			}
			++pos;
		}
	}
	else {
		CEditPropertyDlg dlg;
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		Property Voltage;
		Voltage.PropertyName = CString(_T("Voltage"));
		Voltage.PropertyValue = m_Voltage;
		Voltage.Unit = _T("V");
		PropertyList.push_back(Voltage);

		Property InternalResistance;
		InternalResistance.PropertyName = CString(_T("InternalResistance"));
		InternalResistance.PropertyValue = m_InternalResistance;
		InternalResistance.Unit = _T("kΩ");
		PropertyList.push_back(InternalResistance);

		dlg.InitializeProperty(PropertyList);	
		if (dlg.DoModal() == IDOK) {
			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			m_Voltage = PropertyList[0].PropertyValue;
			m_InternalResistance = PropertyList[1].PropertyValue;
		}
	}
}

CBaseElement* CPowerElementDC::Clone() 
{
	CPowerElementDC *pCloneElement = new CPowerElementDC;
	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	pCloneElement->m_Voltage = m_Voltage;
	pCloneElement->m_InternalResistance = m_InternalResistance;

	return pCloneElement;
}

void CPowerElementDC::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewVoltage;
	NewVoltage.Text = CString(_T("Volatage"));
	NewVoltage.Value = m_Voltage;
	List.push_back(NewVoltage);

	ElementData Newinternal;
	Newinternal.Text = CString(_T("InternalResistance"));
	Newinternal.Value = m_InternalResistance;
	List.push_back(NewVoltage);
}

void CPowerElementDC::RotationElement()
{

}



//////////////////////////////////////////////// CResistanceElement

CResistanceElement::CResistanceElement()
:CBaseElement(resistance)
{
	//자식 객체의 멤버 초기화 필요( 초기값)
	m_Power = m_Temperture = m_MaximumVoltage = 1.0f;
	m_Resistance = 1;
	m_Unit = 1e+3;

	//1k -> 1000;
	//1m -> 0.001;

	//m(e-3), k(e3)
}


CResistanceElement::~CResistanceElement( void )
{

}

void CResistanceElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/)
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
	sbrush.SetColor(Color(255, 0, 128, 128));
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
	}

	if (!m_RotationFlag) { //원래 각도
		PointF Point1((float)m_Position.x -20, (float)m_Position.y);
		PointF Point2((float)m_Position.x -13, (float)m_Position.y);
		PointF Point3((float)m_Position.x -10, (float)m_Position.y -5);
		PointF Point4((float)m_Position.x -6, (float)m_Position.y +5);
		PointF Point5((float)m_Position.x -2, (float)m_Position.y-5);

		PointF Point6((float)m_Position.x +2,(float) m_Position.y +5);
		PointF Point7((float)m_Position.x +6, (float)m_Position.y-5);
		PointF Point8((float)m_Position.x +10, (float)m_Position.y +5);
		PointF Point9((float)m_Position.x +13, (float)m_Position.y);
		PointF Point10((float)m_Position.x +20, (float)m_Position.y);

		PointF Points[10] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10};

		NewPen.SetLineJoin(LineJoinRound);
		pDC->DrawLines(&NewPen, Points, 10);

		CString NewString = m_Name;
		CString ResistanceData;
		ResistanceData.Format(_T("  %.1fk"), m_Resistance);
		NewString += ResistanceData;
		pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x -17, (float)m_Position.y -20, (float)m_Position.x -17, (float)m_Position.y -20), &format, &sbrush);
	}
	else { //회전 각도
		PointF Point1((float)m_Position.x, (float)m_Position.y -20);
		PointF Point2((float)m_Position.x, (float)m_Position.y -13);
		PointF Point3((float)m_Position.x -5, (float)m_Position.y -10);
		PointF Point4((float)m_Position.x +5, (float)m_Position.y -6);
		PointF Point5((float)m_Position.x -5, (float)m_Position.y -2);

		PointF Point6((float)m_Position.x +5,(float) m_Position.y +2);
		PointF Point7((float)m_Position.x -5, (float)m_Position.y +6);
		PointF Point8((float)m_Position.x +5, (float)m_Position.y +10);
		PointF Point9((float)m_Position.x, (float)m_Position.y +13);
		PointF Point10((float)m_Position.x, (float)m_Position.y +20);

		PointF Points[10] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10};

		NewPen.SetLineJoin(LineJoinRound);
		pDC->DrawLines(&NewPen, Points, 10);

		format.SetFormatFlags(StringFormatFlagsDirectionVertical);
		CString NewString = m_Name;
		CString ResistanceData;
		ResistanceData.Format(_T("  %.1fk"), m_Resistance);
		NewString += ResistanceData;
		pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x +10, (float)m_Position.y-17, (float)m_Position.x +10, (float)m_Position.y -17), &format, &sbrush);
	}
	CBaseElement::Draw(pDC, Collection, Modifypoint);
}

void CResistanceElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("Resistance"))) {
				m_Resistance = (*pos).Value;
				break;
			}
			++pos;
		}
	}
	else {
		CEditPropertyDlg dlg;
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		Property Resistance;
		Resistance.PropertyName = CString(_T("Resistance"));
		Resistance.PropertyValue = m_Resistance;
		Resistance.Unit = _T("kΩ");
		PropertyList.push_back(Resistance);

		Property Power;
		Power.PropertyName = CString(_T("Power"));
		Power.PropertyValue = m_Power;
		Power.Unit = _T("W");
		PropertyList.push_back(Power);

		Property Temperture;
		Temperture.PropertyName = CString(_T("Temperture"));
		Temperture.PropertyValue = m_Temperture;
		Temperture.Unit = _T("℃");
		PropertyList.push_back(Temperture);

		Property MaximumVoltage;
		MaximumVoltage.PropertyName = CString(_T("MaximumVoltage"));
		MaximumVoltage.PropertyValue = m_MaximumVoltage;
		MaximumVoltage.Unit = _T("V");
		PropertyList.push_back(MaximumVoltage);

		dlg.InitializeProperty(PropertyList);
		if (dlg.DoModal() == IDOK) {

			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			m_Resistance = PropertyList[0].PropertyValue;
			m_Power = PropertyList[1].PropertyValue;
			m_Temperture = PropertyList[2].PropertyValue;
			m_MaximumVoltage = PropertyList[3].PropertyValue;
		}
	}
}

CBaseElement* CResistanceElement::Clone()
{
	CResistanceElement *pCloneElement = new CResistanceElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	pCloneElement->m_Resistance = m_Resistance;
	pCloneElement->m_Power = m_Power;
	pCloneElement->m_Temperture = m_Temperture;
	pCloneElement->m_MaximumVoltage = m_MaximumVoltage;
	return pCloneElement;
}

void CResistanceElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{

	ElementData NewResistance;
	NewResistance.Text = CString(_T("Resistance"));
	NewResistance.Value = m_Resistance*m_Unit;
	List.push_back(NewResistance);

	ElementData NewPower;
	NewPower.Text = CString(_T("Power"));
	NewPower.Value = m_Power;
	List.push_back(NewPower);

	ElementData NewTemperture;
	NewTemperture.Text = CString(_T("Temperture"));
	NewTemperture.Value = m_Temperture;
	List.push_back(NewTemperture);

	ElementData NewMaximumVoltage;
	NewMaximumVoltage.Text = CString(_T("MaximumVoltage"));
	NewMaximumVoltage.Value = m_MaximumVoltage;
	List.push_back(NewMaximumVoltage);
}

void CResistanceElement::RotationElement()
{
	CBaseElement::RotationElement();
}


////////////////////////////////////////////////////////////////////////// Ground

CGroundElement::CGroundElement()
:CBaseElement(ground)
{

}

CGroundElement::~CGroundElement()
{

}

void CGroundElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
	}	

	pDC->DrawLine(&NewPen, m_Position.x -12, m_Position.y -6, m_Position.x +12, m_Position.y -6);
	pDC->DrawLine(&NewPen, m_Position.x -8, m_Position.y -2, m_Position.x +8, m_Position.y -2);
	pDC->DrawLine(&NewPen, m_Position.x -4, m_Position.y +2, m_Position.x +4, m_Position.y +2);
	pDC->DrawLine(&NewPen, m_Position.x -2, m_Position.y +6, m_Position.x +2, m_Position.y +6);
}

void CGroundElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{

}

CBaseElement* CGroundElement::Clone()
{
	CGroundElement *pCloneElement = new CGroundElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	return pCloneElement;
}

void CGroundElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{

}

void CGroundElement::RotationElement()
{

}



////////////////////////////////////////////////////////////////////////// 코일 


CInductorElement::CInductorElement()
:CBaseElement(inductor)
{
	m_Temperture = m_MaximumVoltage = 1.0f;
	m_Inductance  = 10.0f;
	m_Unit = 1;

	// m(default), H(1e+1)
}

CInductorElement::~CInductorElement()
{

}



void CInductorElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
	sbrush.SetColor(Color(255, 0, 128, 128));
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
	}

	if (!m_RotationFlag) {
		PointF Point1((float)m_Position.x -15, (float)m_Position.y);
		PointF Point2((float)m_Position.x -12, (float)m_Position.y -5);
		PointF Point3((float)m_Position.x -9, (float)m_Position.y +5);
		PointF Point4((float)m_Position.x -11, (float)m_Position.y +5);
		PointF Point5((float)m_Position.x -6, (float)m_Position.y -5);
		PointF Point6((float)m_Position.x -3, (float)m_Position.y +5);
		PointF Point7((float)m_Position.x -5, (float)m_Position.y +5);
		PointF Point8((float)m_Position.x, (float)m_Position.y -5);
		PointF Point9((float)m_Position.x +3, (float)m_Position.y +5);
		PointF Point10((float)m_Position.x +1, (float)m_Position.y +5);
		PointF Point11((float)m_Position.x +6, (float)m_Position.y -5);
		PointF Point12((float)m_Position.x +9, (float)m_Position.y +5);
		PointF Point13((float)m_Position.x +7, (float)m_Position.y +5);
		PointF Point14((float)m_Position.x +12, (float)m_Position.y -5);
		PointF Point15((float)m_Position.x +15, (float)m_Position.y);

		PointF Points[15] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10, Point11, Point12, Point13, Point14, Point15};

		NewPen.SetLineJoin(LineJoinRound);
		pDC->DrawCurve(&NewPen, Points, 15, 1.0f);
		pDC->DrawLine(&NewPen, (float)m_Position.x -20, (float)m_Position.y, (float)m_Position.x -15, (float)m_Position.y);
		pDC->DrawLine(&NewPen, (float)m_Position.x +15, (float)m_Position.y, (float)m_Position.x +20, (float)m_Position.y);

		CString NewString = m_Name;
		CString InductorData;
		InductorData.Format(_T("  %.0fH"), m_Inductance);
		NewString += InductorData;
		pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x -20, (float)m_Position.y -20, (float)m_Position.x -20, (float)m_Position.y -20), &format, &sbrush);
	}
	else {
		PointF Point1((float)m_Position.x, (float)m_Position.y -15);
		PointF Point2((float)m_Position.x -5, (float)m_Position.y -12);
		PointF Point3((float)m_Position.x +5, (float)m_Position.y -9);
		PointF Point4((float)m_Position.x +5, (float)m_Position.y -11);
		PointF Point5((float)m_Position.x -5, (float)m_Position.y -6);
		PointF Point6((float)m_Position.x +5, (float)m_Position.y -3);
		PointF Point7((float)m_Position.x +5, (float)m_Position.y -5);
		PointF Point8((float)m_Position.x -5, (float)m_Position.y);
		PointF Point9((float)m_Position.x +5, (float)m_Position.y  +3);
		PointF Point10((float)m_Position.x +5, (float)m_Position.y +1);
		PointF Point11((float)m_Position.x -5, (float)m_Position.y +6);
		PointF Point12((float)m_Position.x +5, (float)m_Position.y +9);
		PointF Point13((float)m_Position.x +5, (float)m_Position.y +7);
		PointF Point14((float)m_Position.x -5, (float)m_Position.y +12);
		PointF Point15((float)m_Position.x, (float)m_Position.y +15);

		PointF Points[15] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10, Point11, Point12, Point13, Point14, Point15};

		NewPen.SetLineJoin(LineJoinRound);
		pDC->DrawCurve(&NewPen, Points, 15, 1.0f);
		pDC->DrawLine(&NewPen, (float)m_Position.x, (float)m_Position.y -20, (float)m_Position.x, (float)m_Position.y -15);
		pDC->DrawLine(&NewPen, (float)m_Position.x, (float)m_Position.y +15, (float)m_Position.x, (float)m_Position.y +20);

		format.SetFormatFlags(StringFormatFlagsDirectionVertical);

		CString NewString = m_Name;
		CString InductorData;
		InductorData.Format(_T("  %.0fH"), m_Inductance);
		NewString += InductorData;
		pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x +10, (float)m_Position.y-20, (float)m_Position.x +10, (float)m_Position.y -20), &format, &sbrush);
	}

	CBaseElement::Draw(pDC, Collection, Modifypoint);
}

void CInductorElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("Frequency"))) {
				m_Frequency = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("Inductance"))) {
				m_Inductance = (*pos).Value;
			}
			++pos;
		}
	}
	else {
		CEditPropertyDlg dlg;
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		Property Inductance;
		Inductance.PropertyName = CString(_T("Inductance"));
		Inductance.PropertyValue = m_Inductance;
		Inductance.Unit = _T("H");
		PropertyList.push_back(Inductance);

		Property Temperture;
		Temperture.PropertyName = CString(_T("Temperture"));
		Temperture.PropertyValue = m_Temperture;
		Temperture.Unit = _T("C");
		PropertyList.push_back(Temperture);

		Property MaximumVoltage;
		MaximumVoltage.PropertyName = CString(_T("MaximumVoltage"));
		MaximumVoltage.PropertyValue = m_MaximumVoltage;
		MaximumVoltage.Unit = _T("V");
		PropertyList.push_back(MaximumVoltage);

		dlg.InitializeProperty(PropertyList);
		if (dlg.DoModal() == IDOK) {

			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			m_Inductance = PropertyList[0].PropertyValue;
			m_Temperture = PropertyList[1].PropertyValue;
			m_MaximumVoltage = PropertyList[2].PropertyValue;
		}
	}
}

CBaseElement* CInductorElement::Clone()
{
	CInductorElement *pCloneElement = new CInductorElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	pCloneElement->m_Inductance = m_Inductance;
	pCloneElement->m_Temperture = m_Temperture;
	pCloneElement->m_MaximumVoltage = m_MaximumVoltage;
	return pCloneElement;
}

void CInductorElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewImpedance;
	NewImpedance.Text = CString(_T("Impedance"));
	NewImpedance.Value = (2*PIE*m_Frequency*m_Inductance*m_Unit);
	List.push_back(NewImpedance);

	ElementData NewInductance;
	NewInductance.Text = CString(_T("Inductance"));
	NewInductance.Value = m_Inductance*m_Unit;
	List.push_back(NewInductance);

	ElementData NewTemperture;
	NewTemperture.Text = CString(_T("Temperture"));
	NewTemperture.Value = m_Temperture;
	List.push_back(NewTemperture);

	ElementData NewMaximumVoltage;
	NewMaximumVoltage.Text = CString(_T("MaximumVoltage"));
	NewMaximumVoltage.Value = m_MaximumVoltage;
	List.push_back(NewMaximumVoltage);
}

void CInductorElement::RotationElement()
{
	CBaseElement::RotationElement();
}

////////////////////////////////////////////////////////////////////////// 캐패시터

CCapacityElement::CCapacityElement()
:CBaseElement(capacity)
{
	m_Capacitance = m_Temperture = m_MaximumVoltage = 1.0f;
	m_Unit = 1e-6;
	m_NullFlag = FALSE;
	//p(e-12), n(e-9), u(e-6), 
}

CCapacityElement::~CCapacityElement()
{

}

void CCapacityElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
	sbrush.SetColor(Color(255, 0, 128, 128));
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
	}

	if (!m_RotationFlag) {
		pDC->DrawLine(&NewPen, (float)m_Position.x -20, (float)m_Position.y, (float)m_Position.x -4, (float)m_Position.y);
		pDC->DrawLine(&NewPen, (float)m_Position.x -4, (float)m_Position.y -10, (float)m_Position.x -4, (float)m_Position.y +10);

		pDC->DrawLine(&NewPen, (float)m_Position.x +4, (float)m_Position.y -10, (float)m_Position.x +4, (float)m_Position.y +10);
		pDC->DrawLine(&NewPen, (float)m_Position.x +4, (float)m_Position.y, (float)m_Position.x +20, (float)m_Position.y);

		CString NewString = m_Name;
		CString CapacityrData;
		CapacityrData.Format(_T("  %.0fu"), m_Capacitance);
		NewString += CapacityrData;
		pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x -17, (float)m_Position.y -23, (float)m_Position.x-17, (float)m_Position.y -23), &format, &sbrush);
	}
	else {
		pDC->DrawLine(&NewPen, (float)m_Position.x, (float)m_Position.y -20, (float)m_Position.x, (float)m_Position.y -4);
		pDC->DrawLine(&NewPen, (float)m_Position.x -10, (float)m_Position.y -4, (float)m_Position.x +10, (float)m_Position.y -4);

		pDC->DrawLine(&NewPen, (float)m_Position.x -10, (float)m_Position.y +4, (float)m_Position.x +10, (float)m_Position.y +4);
		pDC->DrawLine(&NewPen, (float)m_Position.x, (float)m_Position.y +4, (float)m_Position.x, (float)m_Position.y +20);

		format.SetFormatFlags(StringFormatFlagsDirectionVertical);

		CString NewString = m_Name;
		CString CapacityrData;
		CapacityrData.Format(_T("  %.0fu"), m_Capacitance);
		NewString += CapacityrData;
		pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x +10, (float)m_Position.y-17, (float)m_Position.x +10, (float)m_Position.y -17), &format, &sbrush);
	}

	CBaseElement::Draw(pDC, Collection, Modifypoint);
}

void CCapacityElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("Frequency"))) {
				m_Frequency = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("NullFlag"))) {
				m_NullFlag = (BOOL)(*pos).Value;
			}
			else if ((*pos).Text == CString(_T("Capacitance"))) {
				m_Capacitance = (*pos).Value;
			}
			++pos;
		}
	}
	else {
		CEditPropertyDlg dlg;
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		Property Capacitance;
		Capacitance.PropertyName = CString(_T("Capacitance"));
		Capacitance.PropertyValue = m_Capacitance;
		Capacitance.Unit = _T("u");
		PropertyList.push_back(Capacitance);

		Property Temperture;
		Temperture.PropertyName = CString(_T("Temperture"));
		Temperture.PropertyValue = m_Temperture;
		Temperture.Unit = _T("℃");
		PropertyList.push_back(Temperture);

		Property MaximumVoltage;
		MaximumVoltage.PropertyName = CString(_T("MaximumVoltage"));
		MaximumVoltage.PropertyValue = m_MaximumVoltage;
		MaximumVoltage.Unit = _T("V");
		PropertyList.push_back(MaximumVoltage);

		dlg.InitializeProperty(PropertyList);
		if (dlg.DoModal() == IDOK) {

			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			m_Capacitance = PropertyList[0].PropertyValue;
			m_Temperture = PropertyList[1].PropertyValue;
			m_MaximumVoltage = PropertyList[2].PropertyValue;
		}
	}
}

CBaseElement* CCapacityElement::Clone()
{
	CCapacityElement *pCloneElement = new CCapacityElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	pCloneElement->m_Capacitance = m_Capacitance;
	pCloneElement->m_Temperture = m_Temperture;
	pCloneElement->m_MaximumVoltage = m_MaximumVoltage;
	return pCloneElement;
}

void CCapacityElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewImpedance;
	NewImpedance.Text = CString(_T("Impedance"));
	NewImpedance.Value = 1/(2*PIE*m_Frequency*m_Capacitance*m_Unit);
	if (m_NullFlag) NewImpedance.Value = 0;									//TR 예외처리
	List.push_back(NewImpedance);

	ElementData NewCapacitance;
	NewCapacitance.Text = CString(_T("Capacitance"));
	NewCapacitance.Value = m_Capacitance*m_Unit;
	List.push_back(NewCapacitance);

	ElementData NewTemperture;
	NewTemperture.Text = CString(_T("Temperture"));
	NewTemperture.Value = m_Temperture;
	List.push_back(NewTemperture);

	ElementData NewMaximumVoltage;
	NewMaximumVoltage.Text = CString(_T("MaximumVoltage"));
	NewMaximumVoltage.Value = m_MaximumVoltage;
	List.push_back(NewMaximumVoltage);
}

void CCapacityElement::RotationElement()
{
	CBaseElement::RotationElement();
}

////////////////////////////////////////////////////////////////////////// Prov

CProvElement::CProvElement()
:CBaseElement(prov)
{
	m_ProvColor = darkred;
}

CProvElement::~CProvElement()
{

}

void CProvElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Pen NewPen(Color(255, 75, 75, 75), 1.0f);

	GraphicsPath path;
	path.AddLine(m_Position.x, m_Position.y, m_Position.x +5, m_Position.y-27);
	path.AddLine(m_Position.x+5, m_Position.y-27, m_Position.x+15, m_Position.y-25);
	path.AddLine(m_Position.x+15, m_Position.y-25, m_Position.x, m_Position.y);

	SolidBrush NewBrush(Color(255, 75, 75, 75));
	if (m_ProvActivityFlag) {

		switch (m_ProvColor) {
			case darkred:
				NewBrush.SetColor(Color(255, 171, 40, 38));
				break;

			case darkgreen:
				NewBrush.SetColor(Color(255, 74, 186, 26));
				break;

			case darkyellow:
				NewBrush.SetColor(Color(255,  200, 130, 35));
				break;

			case darkblue:
				NewBrush.SetColor(Color(255, 35, 99, 200));
				break;
		}
	}


	pDC->FillPath(&NewBrush, &path);
	pDC->DrawPath(&NewPen, &path);
	pDC->FillEllipse(&NewBrush, RectF((float)m_Position.x+3, (float)m_Position.y-41, 18, 18));
	pDC->DrawEllipse(&NewPen, RectF((float)m_Position.x+3, (float)m_Position.y-41, 18, 18));

	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::White);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);
	pDC->DrawString(m_Name, -1, &font, RectF((float)m_Position.x +5, (float)m_Position.y -38, (float)m_Position.x +5, (float)m_Position.y -38), &format, &sbrush);

}

void CProvElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("ProvColor"))) {
				m_ProvColor = (int)(*pos).Value;
			}
			++pos;
		}
	}
}

CBaseElement* CProvElement::Clone()
{
	return NULL;
}

void CProvElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewProvColor;
	NewProvColor.Text = CString(_T("ProvColor"));
	NewProvColor.Value = m_ProvColor;
	List.push_back(NewProvColor);
}

void CProvElement::RotationElement()
{

}


////////////////////////////////////////////////////////////////////////// Diode

CDiodeElement::CDiodeElement()
:CBaseElement(diode)
{
	//const
	m_ThermalVoltage = 0.025875;
	m_OperatingPoint = 0.9;

	//non const
	m_SaturationCurrent = 403;  //m(e-3) u(e-6), n(e-9), p(e-12), f(e-15)
	m_EmissonConefficient = 1.57;
	m_AntidromicFlag = FALSE;
	m_Unit = 1e-12;
}

CDiodeElement::~CDiodeElement()
{

}

void CDiodeElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
	sbrush.SetColor(Color(255, 0, 128, 128));
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
	}

	switch (m_RotationFlag) {
	case Angle0:
		{
			GraphicsPath path;
			path.AddLine(m_Position.x -8, m_Position.y -9, m_Position.x -8, m_Position.y +9); 
			path.AddLine(m_Position.x -8, m_Position.y +9, m_Position.x +8, m_Position.y);
			path.AddLine(m_Position.x +8, m_Position.y, m_Position.x -8, m_Position.y -9);

			pDC->FillPath(&sbrush, &path);
			pDC->DrawLine(&NewPen, m_Position.x +8, m_Position.y -9, m_Position.x +8, m_Position.y +9);

			pDC->DrawLine(&NewPen, m_Position.x -20, m_Position.y, m_Position.x -8, m_Position.y);
			pDC->DrawLine(&NewPen, m_Position.x +8, m_Position.y, m_Position.x +20, m_Position.y);

			pDC->DrawString(m_Name, -1, &font, RectF((float)m_Position.x -25, (float)m_Position.y -20, (float)m_Position.x +20, (float)m_Position.y -20), &format, &sbrush);
		}
		break;

	case Angle90:
		{
			GraphicsPath path;
			path.AddLine(m_Position.x -9, m_Position.y -8, m_Position.x +9, m_Position.y -8); 
			path.AddLine(m_Position.x +9, m_Position.y -8, m_Position.x, m_Position.y +8);
			path.AddLine(m_Position.x, m_Position.y +8, m_Position.x -9, m_Position.y -8);


			pDC->FillPath(&sbrush, &path);
			pDC->DrawLine(&NewPen, m_Position.x -9, m_Position.y +8, m_Position.x +9, m_Position.y +8);

			pDC->DrawLine(&NewPen, m_Position.x, m_Position.y -20, m_Position.x, m_Position.y -8);
			pDC->DrawLine(&NewPen, m_Position.x, m_Position.y +8, m_Position.x, m_Position.y +20);

			format.SetFormatFlags(StringFormatFlagsDirectionVertical);
			pDC->DrawString(m_Name, -1, &font, RectF((float)m_Position.x +10, (float)m_Position.y-25, (float)m_Position.x +10, (float)m_Position.y -20), &format, &sbrush);
		}
		break;

	case Angle180:
		{
			GraphicsPath path;
			path.AddLine(m_Position.x +8, m_Position.y -9, m_Position.x +8, m_Position.y +9); 
			path.AddLine(m_Position.x +8, m_Position.y +9, m_Position.x -8, m_Position.y);
			path.AddLine(m_Position.x -8, m_Position.y, m_Position.x +8, m_Position.y -9);

			pDC->FillPath(&sbrush, &path);
			pDC->DrawLine(&NewPen, m_Position.x -8, m_Position.y -9, m_Position.x -8, m_Position.y +9);

			pDC->DrawLine(&NewPen, m_Position.x -20, m_Position.y, m_Position.x -8, m_Position.y);
			pDC->DrawLine(&NewPen, m_Position.x +8, m_Position.y, m_Position.x +20, m_Position.y);

			pDC->DrawString(m_Name, -1, &font, RectF((float)m_Position.x -25, (float)m_Position.y -20, (float)m_Position.x +20, (float)m_Position.y -20), &format, &sbrush);
		}
		break;

	case Angle270:
		{
			GraphicsPath path;
			path.AddLine(m_Position.x -9, m_Position.y +8, m_Position.x +9, m_Position.y +8); 
			path.AddLine(m_Position.x +9, m_Position.y +8, m_Position.x, m_Position.y -8);
			path.AddLine(m_Position.x, m_Position.y -8, m_Position.x -9, m_Position.y +8);

			pDC->FillPath(&sbrush, &path);
			pDC->DrawLine(&NewPen, m_Position.x -9, m_Position.y -8, m_Position.x +9, m_Position.y -8);

			pDC->DrawLine(&NewPen, m_Position.x, m_Position.y -20, m_Position.x, m_Position.y -8);
			pDC->DrawLine(&NewPen, m_Position.x, m_Position.y +8, m_Position.x, m_Position.y +20);

			format.SetFormatFlags(StringFormatFlagsDirectionVertical);
			pDC->DrawString(m_Name, -1, &font, RectF((float)m_Position.x +10, (float)m_Position.y-25, (float)m_Position.x +10, (float)m_Position.y -20), &format, &sbrush);
		}
		break;
	}


	CBaseElement::Draw(pDC, Collection, Modifypoint);
}

void CDiodeElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if((*pos).Text == CString(_T("AntidromicFlag"))) {
				m_AntidromicFlag = (int)(*pos).Value;
			}
			++pos;
		}	
	}
	else {
		CEditPropertyDlg dlg;
		dlg.SetElementFlag(Fdiode);
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		Property OperatingPoint;
		OperatingPoint.PropertyName = CString(_T("OperatingPoint"));
		OperatingPoint.PropertyValue = m_OperatingPoint;
		OperatingPoint.Unit = _T("V");
		PropertyList.push_back(OperatingPoint);

		Property ThermalVoltage;
		ThermalVoltage.PropertyName = CString(_T("ThermalVoltage"));
		ThermalVoltage.PropertyValue = m_ThermalVoltage;
		ThermalVoltage.Unit = _T("V");
		PropertyList.push_back(ThermalVoltage);

		Property SaturationCurrent;
		SaturationCurrent.PropertyName = CString(_T("SaturationCurrent"));
		SaturationCurrent.PropertyValue = m_SaturationCurrent;
		SaturationCurrent.Unit = _T("e-12");
		PropertyList.push_back(SaturationCurrent);

		Property EmissonConefficient;
		EmissonConefficient.PropertyName = CString(_T("EmissonConefficient"));
		EmissonConefficient.PropertyValue = m_EmissonConefficient;
		EmissonConefficient.Unit = _T("N");
		PropertyList.push_back(EmissonConefficient);

		dlg.InitializeProperty(PropertyList);
		if (dlg.DoModal() == IDOK) {

			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			m_OperatingPoint = PropertyList[0].PropertyValue;
			m_ThermalVoltage = PropertyList[1].PropertyValue;
			m_SaturationCurrent = PropertyList[2].PropertyValue;
			m_EmissonConefficient = PropertyList[3].PropertyValue;
		}
	}
}

CBaseElement* CDiodeElement::Clone()
{
	CDiodeElement *pCloneElement = new CDiodeElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	pCloneElement->m_SaturationCurrent = m_SaturationCurrent;
	pCloneElement->m_EmissonConefficient = m_EmissonConefficient;

	return pCloneElement;
}

void CDiodeElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewSaturationCurrent;
	NewSaturationCurrent.Text = CString(_T("SaturationCurrent"));
	NewSaturationCurrent.Value = m_SaturationCurrent*m_Unit;
	List.push_back(NewSaturationCurrent);

	ElementData NewThermalVoltage;
	NewThermalVoltage.Text = CString(_T("ThermalVoltage"));
	NewThermalVoltage.Value = m_ThermalVoltage;
	List.push_back(NewThermalVoltage);

	ElementData NewEmissonConefficient;
	NewEmissonConefficient.Text = CString(_T("EmissonConefficient"));
	NewEmissonConefficient.Value = m_EmissonConefficient;
	List.push_back(NewEmissonConefficient);

	ElementData NewOperatingPoint;
	NewOperatingPoint.Text = CString(_T("OperatingPoint"));
	NewOperatingPoint.Value = m_OperatingPoint;
	List.push_back(NewOperatingPoint);

	ElementData NewAntidromicFlag;
	NewAntidromicFlag.Text = CString(_T("AntidromicFlag"));
	NewAntidromicFlag.Value = m_AntidromicFlag;
	List.push_back(NewAntidromicFlag);
}

void CDiodeElement::RotationElement()
{
	CBaseElement::RotationElement();
}


////////////////////////////////////////////////////////////////////////// Leq class
CLeqElement::CLeqElement()
:CBaseElement(leq)
{
	m_pGeqElement = NULL;
	m_SaturationCurrent = m_ThermalVoltage = m_EmissonConefficient = m_OperatingPoint = 0;
}

CLeqElement::~CLeqElement()
{

}

void CLeqElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{

}

void CLeqElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("SaturationCurrent"))) {
				m_SaturationCurrent = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("ThermalVoltage"))) {
				m_ThermalVoltage = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("EmissonConefficient"))) {
				m_EmissonConefficient = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("OperatingPoint"))) {
				m_OperatingPoint = (*pos).Value;

				if (m_AntidromicFlag) {
					m_OperatingPoint *= -1;
				}
			}
			else if ((*pos).Text == CString(_T("GeqElement"))) {
				m_pGeqElement = (CBaseElement*)(*pos).pElement;
			}
			++pos;
		}	
	}
}

CBaseElement* CLeqElement::Clone()
{
	return NULL;
}

void CLeqElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	double nVt = m_EmissonConefficient * m_ThermalVoltage;
	double Ido = m_SaturationCurrent*(exp(m_OperatingPoint/nVt) -1);

	list<ElementData> GeqList;
	m_pGeqElement->GetProperties(GeqList);

	double Geq = 0;
	list<ElementData>::iterator pos = GeqList.begin();
	while (pos != GeqList.end()) {
		if ((*pos).Text == CString(_T("Geq"))) {
			Geq = (*pos).Value;
		}
		++pos;
	}


	ElementData NewOperationPoint;
	NewOperationPoint.Text = CString(_T("OperatingPoint"));
	NewOperationPoint.Value = m_OperatingPoint;
	List.push_back(NewOperationPoint);

	ElementData NewLeq;
	NewLeq.Text = CString(_T("Leq"));
	NewLeq.Value = Ido - Geq*m_OperatingPoint;
	if (m_AntidromicFlag) NewLeq.Value *= -1;
	List.push_back(NewLeq);

	ElementData NewGeq;
	NewGeq.Text = CString(_T("GeqElement"));
	NewGeq.pElement = m_pGeqElement;
	List.push_back(NewGeq);

	TRACE("Leq : %lf\n", NewLeq.Value);
}

void CLeqElement::RotationElement()
{

}


////////////////////////////////////////////////////////////////////////// Geq class
CGeqElement::CGeqElement()
:CBaseElement(geq)
{
	m_SaturationCurrent = m_ThermalVoltage = m_EmissonConefficient = m_OperatingPoint = 0;
}

CGeqElement::~CGeqElement()
{

}

void CGeqElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{

}

void CGeqElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)
{
	if (List != NULL) {

		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("SaturationCurrent"))) {
				m_SaturationCurrent = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("ThermalVoltage"))) {
				m_ThermalVoltage = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("EmissonConefficient"))) {
				m_EmissonConefficient = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("OperatingPoint"))) {
				m_OperatingPoint = (*pos).Value;

				if (m_AntidromicFlag) {
					m_OperatingPoint *= -1;
				}
			}
			++pos;
		}
	}
}

CBaseElement* CGeqElement::Clone()
{
	return NULL;
}

void CGeqElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	// 	double m_OperatingPoint;			//Vdo = 0.9
	// 	double m_ThermalVoltage;			//Vt = 0.02587
	// 	double m_SaturationCurrent;			//Is = 403e-12, p = e-12
	// 	double m_EmissonConefficient;		//n = 1.57;

	ElementData NewGeq;
	NewGeq.Text = CString(_T("Geq"));

	double nVt = m_EmissonConefficient * m_ThermalVoltage;
	NewGeq.Value = (m_SaturationCurrent/nVt)*(exp(m_OperatingPoint/nVt) - 1);

	TRACE("Geq : %lf\n", NewGeq.Value);
	List.push_back(NewGeq);
}

void CGeqElement::RotationElement()
{

}



////////////////////////////////////////////////////////////////////////// ACPower

CPowerElementAC::CPowerElementAC()
:CBaseElement(acpower)
{
	m_Frequency = 50;
	m_Amplitude = 1.0;
	m_WaveModel = 0;		//0(정현파), 1(구형파), 2(삼각파)
	m_Unit = 1e-3;
}

CPowerElementAC::~CPowerElementAC()
{

}

void CPowerElementAC::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 0 , 255), 1.0f);
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
	}

	pDC->DrawEllipse(&NewPen, RectF( (float)m_Position.x -10,  (float)m_Position.y -10, 20, 20));
	pDC->DrawLine(&NewPen, m_Position.x +6, m_Position.y -14, m_Position.x +14, m_Position.y -14);
	pDC->DrawLine(&NewPen, m_Position.x +10, m_Position.y -10, m_Position.x +10, m_Position.y -18);
	pDC->DrawLine(&NewPen, m_Position.x, m_Position.y -20, m_Position.x, m_Position.y -10);
	pDC->DrawLine(&NewPen, m_Position.x, m_Position.y +10, m_Position.x, m_Position.y +20);

	GraphicsPath path;
	path.AddLine(m_Position.x -8, m_Position.y +5, m_Position.x -4, m_Position.y +5);
	path.AddLine(m_Position.x -4, m_Position.y +5, m_Position.x +4, m_Position.y -5);
	path.AddLine(m_Position.x +4, m_Position.y -5, m_Position.x +8, m_Position.y -5);
	pDC->DrawPath(&NewPen, &path);

	CString NewString = m_Name;
	CString PowerData;
	PowerData.Format(_T("  %.0fV"), m_Amplitude);
	NewString += PowerData;
	pDC->DrawString(NewString, -1, &font, RectF((float)m_Position.x +10, (float)m_Position.y -10, (float)m_Position.x +10, (float)m_Position.y -10), &format, &sbrush);

	CBaseElement::Draw(pDC, Collection, Modifypoint);
}

void CPowerElementAC::OnEditProperties( list<ElementData> *List /*= NULL*/, CPoint point /*= CPoint(0,0)*/ )
{
	CEditPropertyDlg dlg;
	vector<Property> PropertyList;
	dlg.m_strElementName = m_Name;
	dlg.m_GroupName = m_GroupName;
	dlg.m_ElementFlag = m_Flag;

	Property Amplitude;
	Amplitude.PropertyName = CString(_T("Amplitude"));
	Amplitude.PropertyValue = m_Amplitude;
	Amplitude.Unit = _T("V");
	PropertyList.push_back(Amplitude);

	Property Frequency;
	Frequency.PropertyName = CString(_T("Frequency"));
	Frequency.PropertyValue = m_Frequency;
	Frequency.Unit = _T("Hz");
	PropertyList.push_back(Frequency);

	Property WaveModel;
	WaveModel.PropertyName = CString(_T("WaveModel"));
	WaveModel.PropertyValue = m_WaveModel;
	PropertyList.push_back(WaveModel);

	dlg.InitializeProperty(PropertyList);
	if (dlg.DoModal() == IDOK) {

		PropertyList.clear();
		PropertyList.resize(dlg.m_PropertyList.size());
		copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

		m_Amplitude = PropertyList[0].PropertyValue;
		m_Frequency = PropertyList[1].PropertyValue;
		m_WaveModel = (int)PropertyList[2].PropertyValue;
	}
}

CBaseElement* CPowerElementAC::Clone()
{
	CPowerElementAC *pCloneElement = new CPowerElementAC;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	pCloneElement->m_Amplitude = m_Amplitude;
	pCloneElement->m_Frequency = m_Frequency;
	pCloneElement->m_WaveModel = m_WaveModel;

	return pCloneElement;
}

void CPowerElementAC::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewAmplitude;
	NewAmplitude.Text = CString(_T("Amplitude"));
	NewAmplitude.Value = m_Amplitude;
	List.push_back(NewAmplitude);

	ElementData NewFrequency;
	NewFrequency.Text = CString(_T("Frequency"));
	NewFrequency.Value = m_Frequency;
	List.push_back(NewFrequency);

	ElementData NewWaveModel;
	NewWaveModel.Text = CString(_T("WaveModel"));
	NewWaveModel.Value = m_WaveModel;
	List.push_back(NewWaveModel);
}

void CPowerElementAC::RotationElement()
{

}


////////////////////////////////////////////////////////////////////////// 트랜지스터
CTransistorElement::CTransistorElement()
:CBaseElement(transistor)
{
	m_OperatingPoint = 0.7;
	m_SaturatingPoint = 0.2;
	m_Beta = 100;
	m_OperatorFlag = trnormal;
}

CTransistorElement::~CTransistorElement()
{

}

void CTransistorElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Gdiplus::Font font(_T("Arial"), 20, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);
	sbrush.SetColor(Color(255, 0, 128, 128));

	Pen NewPen(Color(255, 0, 128, 128), 1.0f);
	Pen NewBoldPen(Color(255, 0, 128, 128), 2.0f);
	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		NewBoldPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
	}

	if (m_OperatorFlag != trnormal) {

		CString OperatorString;
		switch (m_OperatorFlag) {
			case off:
				sbrush.SetColor(Color(255, 192, 192, 192));
				NewPen.SetColor(Color(255, 192, 192, 192));
				NewBoldPen.SetColor(Color(255, 192, 192, 192));
				OperatorString = _T("OFF");
				break;

			case active:
				sbrush.SetColor(Color(255, 0, 221, 0));
				NewPen.SetColor(Color(255, 0, 221, 0));
				NewBoldPen.SetColor(Color(255, 0, 221, 0));
				OperatorString = _T("ACTIVE");
				break;

			case saturated:
				sbrush.SetColor(Color(255, 255, 204, 0));
				NewPen.SetColor(Color(255, 255, 204, 0));
				NewBoldPen.SetColor(Color(255, 255, 204, 0));
				OperatorString = _T("SAT");
				break;
		}

		pDC->DrawString(OperatorString, -1, &font, RectF((float)m_Position.x +10, (float)m_Position.y -11, (float)m_Position.x +10, (float)m_Position.y -11), &format, &sbrush);
	}

	pDC->DrawLine(&NewPen, m_Position.x -30, m_Position.y, m_Position.x -15, m_Position.y);
	pDC->DrawLine(&NewBoldPen, m_Position.x -15, m_Position.y-10, m_Position.x -15, m_Position.y+10);

	pDC->DrawLine(&NewPen, m_Position.x -15, m_Position.y-5, m_Position.x, m_Position.y-13);
	pDC->DrawLine(&NewPen, m_Position.x -15, m_Position.y+5, m_Position.x, m_Position.y+13);

	pDC->DrawLine(&NewPen, m_Position.x, m_Position.y-20, m_Position.x, m_Position.y-13);
	pDC->DrawLine(&NewPen, m_Position.x, m_Position.y+20, m_Position.x, m_Position.y+13);

	GraphicsPath path;
	path.AddLine(m_Position.x -9, m_Position.y +4, m_Position.x -2, m_Position.y +12); 
	path.AddLine(m_Position.x -2, m_Position.y +12, m_Position.x -12, m_Position.y +12);
	path.AddLine(m_Position.x -12, m_Position.y +12, m_Position.x -9, m_Position.y +4);

	pDC->FillPath(&sbrush, &path);

	
	Gdiplus::Font strfont(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	pDC->DrawString(m_Name, -1, &strfont, RectF((float)m_Position.x -25, (float)m_Position.y -25, (float)m_Position.x -25, (float)m_Position.y -25), &format, &sbrush);

	CBaseElement::Draw(pDC, Collection, Modifypoint);
}

void CTransistorElement::OnEditProperties( list<ElementData> *List /*= NULL*/, CPoint point /*= CPoint(0,0)*/ )
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("OperatorFlag"))) {
				m_OperatorFlag = (int)(*pos).Value;
			}
			++pos;
		}
	}
	else {
		CEditPropertyDlg dlg;
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		Property OperatingPoint;
		OperatingPoint.PropertyName = CString(_T("OperatingPoint"));
		OperatingPoint.PropertyValue = m_OperatingPoint;
		OperatingPoint.Unit = _T("V");
		PropertyList.push_back(OperatingPoint);

		Property SaturatingPoint;
		SaturatingPoint.PropertyName = CString(_T("SaturatingPoint"));
		SaturatingPoint.PropertyValue = m_SaturatingPoint;
		SaturatingPoint.Unit = _T("V");
		PropertyList.push_back(SaturatingPoint);

		Property Beta;
		Beta.PropertyName = CString(_T("Beta"));
		Beta.PropertyValue = m_Beta;
		Beta.Unit = _T("β");
		PropertyList.push_back(Beta);

		dlg.InitializeProperty(PropertyList);
		if (dlg.DoModal() == IDOK) {

			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			m_OperatingPoint = PropertyList[0].PropertyValue;
			m_SaturatingPoint = PropertyList[1].PropertyValue;
			m_Beta = (int)PropertyList[2].PropertyValue;
		}
	}
}

CBaseElement* CTransistorElement::Clone()
{
	CTransistorElement *pCloneElement = new CTransistorElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	return pCloneElement;
}

void CTransistorElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewOperatingPoint;
	NewOperatingPoint.Text = CString(_T("OperatingPoint"));
	NewOperatingPoint.Value = m_OperatingPoint;
	List.push_back(NewOperatingPoint);

	ElementData NewSaturatingPoint;
	NewSaturatingPoint.Text = CString(_T("SaturatingPoint"));
	NewSaturatingPoint.Value = m_SaturatingPoint;
	List.push_back(NewSaturatingPoint);

	ElementData NewBeta;
	NewBeta.Text = CString(_T("Beta"));
	NewBeta.Value = m_Beta;
	List.push_back(NewBeta);
}

void CTransistorElement::RotationElement()
{

}

////////////////////////////////////////////////////////////////////////// 합성저항 클래스 (복소수 적용), 내부 사용

CCompositionElement::CCompositionElement()
:CBaseElement(composition)
{
	m_Unit = 1e+3;
}

CCompositionElement::~CCompositionElement()
{

}

void CCompositionElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{

}

void CCompositionElement::OnEditProperties( list<ElementData> *List /*= NULL*/, CPoint point /*= CPoint(0,0)*/ )
{
	if (List != NULL) {
		list<ElementData>::iterator pos = List->begin();
		while (pos != List->end()) {

			if ((*pos).Text == CString(_T("Resistance"))) {
				m_Resistance = (*pos).Value;
			}
			else if ((*pos).Text == CString(_T("Complexnumber"))) {
				m_Complexnumber = (*pos).Value;
			}
			++pos;
		}
	}
}

CBaseElement* CCompositionElement::Clone()
{
	return NULL;
}

void CCompositionElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewResistance;
	NewResistance.Text = CString(_T("Resistance"));
	NewResistance.Value = m_Resistance*m_Unit;
	List.push_back(NewResistance);

	ElementData NewComplexnumber;
	NewComplexnumber.Text = CString(_T("Complexnumber"));
	NewComplexnumber.Value = m_Complexnumber;
	List.push_back(NewComplexnumber);
}

void CCompositionElement::RotationElement()
{

}


////////////////////////////////////////////////////////////////////////// 반전 증폭기
CInvertAmpElement::CInvertAmpElement()
:CBaseElement(InvertingAmplifier)
{
	m_LeftPower = m_LeftTemperture = m_LeftMaximumVoltage = 1.0f;
	m_LeftResistance = 1;
	m_Unit = 1e+3;

	m_UpPower = m_UpTemperture = m_UpMaximumVoltage = 1.0f;
	m_UpResistance = 1;

	m_MaxVoltage = 12;
	m_MinVoltage = -12;

	//firstresistance, secodersistance, maxvoltage, minvoltage

	OPAmpInsideArea LeftResistance;
	LeftResistance.Insidetype = leftresistance;
	LeftResistance.Area = CRect(-130, -20, -70, 20);
	m_InsideAreaList.push_back(LeftResistance);

	OPAmpInsideArea UpResistance;
	UpResistance.Insidetype = upresistance;
	UpResistance.Area = CRect(-40, -110, 20, -70);
	m_InsideAreaList.push_back(UpResistance);

	OPAmpInsideArea MaxVoltage;
	MaxVoltage.Insidetype = maxvoltage;
	MaxVoltage.Area = CRect(-5, -75, 25, -55);
	m_InsideAreaList.push_back(MaxVoltage);

	OPAmpInsideArea MinVoltage;
	MinVoltage.Insidetype = minvoltage;
	MinVoltage.Area = CRect(-5, +35, 25, +50);
	m_InsideAreaList.push_back(MinVoltage);
}

CInvertAmpElement::~CInvertAmpElement()
{

}

void CInvertAmpElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	SolidBrush Greensbrush(Color(255, 0, 128, 128));
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 0 ,0), 1.0f);
	Pen BluePen(Color(255, 28, 28, 160), 2.0f);
	Pen GreenPen(Color(255, 0, 128, 128), 1.0f);
	Pen RedPen(Color(255, 255, 0 , 0), 1.0f);

	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
		Greensbrush.SetColor(Color(255, 255, 0 , 0));
		BluePen.SetColor(Color(255, 255, 0 , 0));
		GreenPen.SetColor(Color(255, 255, 0 , 0));
	}

	//삼각형
	GraphicsPath path;
	path.AddLine(m_Position.x -20, m_Position.y -34, m_Position.x -20, m_Position.y +10);
	path.AddLine(m_Position.x -20, m_Position.y +10, m_Position.x +20, m_Position.y -12);
	path.AddLine(m_Position.x +20, m_Position.y -12, m_Position.x -20, m_Position.y -34);
	pDC->DrawPath(&BluePen, &path);
	CString Name(_T("OP1 uA741"));
	pDC->DrawString(Name, -1, &font, RectF((float)m_Position.x, (float)m_Position.y +5, (float)m_Position.x, (float)m_Position.y +5), &format, &Greensbrush);

	//왼쪽 라인
	pDC->DrawLine(&NewPen, m_Position.x -80, m_Position.y, m_Position.x -20, m_Position.y);
	pDC->DrawLine(&GreenPen, m_Position.x -17, m_Position.y +2, m_Position.x - 11, m_Position.y +2);

	pDC->DrawLine(&RedPen, m_Position.x -24, m_Position.y -2, m_Position.x -24, m_Position.y +2);		//red
	pDC->DrawLine(&RedPen, m_Position.x -26, m_Position.y -2, m_Position.x -26, m_Position.y +2);

	//왼쪽 아래 저항
	PointF Point1((float)m_Position.x -120, (float)m_Position.y);
	PointF Point2((float)m_Position.x -113, (float)m_Position.y);
	PointF Point3((float)m_Position.x -110, (float)m_Position.y -5);
	PointF Point4((float)m_Position.x -106, (float)m_Position.y +5);
	PointF Point5((float)m_Position.x -102, (float)m_Position.y-5);

	PointF Point6((float)m_Position.x -98,(float) m_Position.y +5);
	PointF Point7((float)m_Position.x -94, (float)m_Position.y-5);
	PointF Point8((float)m_Position.x -90, (float)m_Position.y +5);
	PointF Point9((float)m_Position.x -87, (float)m_Position.y);
	PointF Point10((float)m_Position.x -80, (float)m_Position.y);

	PointF Points[10] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10};

	NewPen.SetLineJoin(LineJoinRound);
	pDC->DrawLines(&GreenPen, Points, 10);

	pDC->DrawLine(&RedPen, m_Position.x -80, m_Position.y -2, m_Position.x -80, m_Position.y +2);		//red
	pDC->DrawLine(&RedPen, m_Position.x -82, m_Position.y -2, m_Position.x -82, m_Position.y +2);		//red

	CString LeftResistanName;
	LeftResistanName.Format(_T("  R %.0fk"), m_LeftResistance);
	pDC->DrawString(LeftResistanName, -1, &font, RectF((float)m_Position.x -115, (float)m_Position.y -20, (float)m_Position.x -115, (float)m_Position.y -20), &format, &Greensbrush);

	//왼쪽 위 그라운드

	pDC->DrawLine(&NewPen, m_Position.x - 35, m_Position.y -24, m_Position.x -20, m_Position.y - 24);
	pDC->DrawLine(&NewPen, m_Position.x - 35, m_Position.y -24, m_Position.x -35, m_Position.y - 16);
	pDC->DrawLine(&GreenPen, m_Position.x - 42, m_Position.y -16, m_Position.x -28, m_Position.y -16);
	pDC->DrawLine(&GreenPen, m_Position.x - 39, m_Position.y -13, m_Position.x -31, m_Position.y -13);
	pDC->DrawLine(&GreenPen, m_Position.x - 36, m_Position.y -10, m_Position.x -34, m_Position.y -10);

	pDC->DrawLine(&RedPen, m_Position.x -24, m_Position.y - 26, m_Position.x -24, m_Position.y -22);		//red
	pDC->DrawLine(&RedPen, m_Position.x -26, m_Position.y - 26, m_Position.x -26, m_Position.y -22);

	pDC->DrawLine(&RedPen, m_Position.x -37, m_Position.y - 21, m_Position.x -33, m_Position.y -21);		//red
	pDC->DrawLine(&RedPen, m_Position.x -37, m_Position.y - 19, m_Position.x -33, m_Position.y -19);		//red

	//왼쪽 위 +
	pDC->DrawLine(&GreenPen, m_Position.x -17, m_Position.y -24, m_Position.x - 11, m_Position.y -24);
	pDC->DrawLine(&GreenPen, m_Position.x -14, m_Position.y -27, m_Position.x - 14, m_Position.y -21);

	//Plus voltage
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y -25, m_Position.x -5, m_Position.y -50);
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y -50, m_Position.x +5, m_Position.y -50);
	pDC->DrawLine(&GreenPen, m_Position.x +5, m_Position.y -54, m_Position.x +5, m_Position.y -46);

	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y -30, m_Position.x -2, m_Position.y -30);		//red
	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y -32, m_Position.x -2, m_Position.y -32);		//red
	pDC->DrawLine(&RedPen, m_Position.x, m_Position.y -52, m_Position.x, m_Position.y -48);		//red
	pDC->DrawLine(&RedPen, m_Position.x +2, m_Position.y -52, m_Position.x +2, m_Position.y -48);		//red

	CString MaxVoltageName;
	MaxVoltageName.Format(_T("%.0fV"), m_MaxVoltage);
	pDC->DrawString(MaxVoltageName, -1, &font, RectF((float)m_Position.x, (float)m_Position.y -70, (float)m_Position.x, (float)m_Position.y -70), &format, &Greensbrush);

	//오른쪽 위 +
	pDC->DrawLine(&GreenPen, m_Position.x -8, m_Position.y -18, m_Position.x -2, m_Position.y -18);
	pDC->DrawLine(&GreenPen, m_Position.x -5, m_Position.y -21, m_Position.x -5, m_Position.y -15);

	//Minus voltage
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y +3, m_Position.x -5, m_Position.y +28);
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y +28, m_Position.x +5, m_Position.y +28);
	pDC->DrawLine(&GreenPen, m_Position.x +5, m_Position.y +24, m_Position.x +5, m_Position.y +32);

	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y +7, m_Position.x -2, m_Position.y +7);		//red
	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y +9, m_Position.x -2, m_Position.y +9);		//red
	pDC->DrawLine(&RedPen, m_Position.x, m_Position.y +26, m_Position.x, m_Position.y +30);		//red
	pDC->DrawLine(&RedPen, m_Position.x +2, m_Position.y +26, m_Position.x +2, m_Position.y +30);		//red

	CString MinVoltageName;
	MinVoltageName.Format(_T("%.0fV"), m_MinVoltage);
	pDC->DrawString(MinVoltageName, -1, &font, RectF((float)m_Position.x, (float)m_Position.y +35, (float)m_Position.x, (float)m_Position.y +35), &format, &Greensbrush);

	//오른쪽 라인
	pDC->DrawLine(&NewPen, m_Position.x +20, m_Position.y -12, m_Position.x +80, m_Position.y -12);
	pDC->DrawLine(&NewPen, m_Position.x +80, m_Position.y -12, m_Position.x +80, m_Position.y);

	//병렬 저항 라인
	pDC->DrawLine(&NewPen, m_Position.x - 70, m_Position.y, m_Position.x -70, m_Position.y -90);
	pDC->DrawLine(&NewPen, m_Position.x - 70, m_Position.y -90, m_Position.x-30, m_Position.y -90);
	{
		PointF Point1((float)m_Position.x -30, (float)m_Position.y -90);
		PointF Point2((float)m_Position.x -23, (float)m_Position.y -90);
		PointF Point3((float)m_Position.x -20, (float)m_Position.y -95);
		PointF Point4((float)m_Position.x -16, (float)m_Position.y -85);
		PointF Point5((float)m_Position.x -12, (float)m_Position.y -95);

		PointF Point6((float)m_Position.x -8,(float) m_Position.y -85);
		PointF Point7((float)m_Position.x -4, (float)m_Position.y -95);
		PointF Point8((float)m_Position.x, (float)m_Position.y -85);
		PointF Point9((float)m_Position.x +3, (float)m_Position.y -90);
		PointF Point10((float)m_Position.x +10, (float)m_Position.y -90);

		PointF Points[10] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10};

		NewPen.SetLineJoin(LineJoinRound);
		pDC->DrawLines(&GreenPen, Points, 10);

		pDC->DrawLine(&RedPen, m_Position.x -32, m_Position.y - 92, m_Position.x -32, m_Position.y -88);		//red
		pDC->DrawLine(&RedPen, m_Position.x -30, m_Position.y - 92, m_Position.x -30, m_Position.y -88);		//red

		pDC->DrawLine(&RedPen, m_Position.x +12, m_Position.y - 92, m_Position.x +12, m_Position.y -88);		//red
		pDC->DrawLine(&RedPen, m_Position.x +10, m_Position.y - 92, m_Position.x +10, m_Position.y -88);		//red

		CString UpResistanceName;
		UpResistanceName.Format(_T("  R %.0fk"), m_UpResistance);
		pDC->DrawString(UpResistanceName, -1, &font, RectF((float)m_Position.x -25, (float)m_Position.y -110, (float)m_Position.x -25, (float)m_Position.y -110), &format, &Greensbrush);

	}
	pDC->DrawLine(&NewPen, m_Position.x+10, m_Position.y -90, m_Position.x +50, m_Position.y -90);
	pDC->DrawLine(&NewPen, m_Position.x +50, m_Position.y -90, m_Position.x +50, m_Position.y -12);

	pDC->FillEllipse(&sbrush, RectF((float)m_Position.x -73, (float)m_Position.y -3, (float)6, (float)6));
	pDC->FillEllipse(&sbrush, RectF((float)m_Position.x +47, (float)m_Position.y -15, (float)6, (float)6));


	//라인 연결 구간
	{
		Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
		Pen RedPen(Color(255, 255, 0 , 0), 1.0f);
		if (Collection.Selectflag) {
			NewPen.SetColor(Color(255, 255, 0 , 0));
		}


		int iCount = 0;
		list<CPoint>::iterator pos = m_ConnectPoint.begin();	//연결 구간
		while (pos != m_ConnectPoint.end()) {

			BOOL nFlag = FALSE;
			list<CBaseElement*>::iterator connectpos = m_ConnectElement.begin();
			while (connectpos != m_ConnectElement.end()) {
				if ((*connectpos)->m_Flag == lines && 
					((*connectpos)->m_LineTo == m_Position + (*pos) || (*connectpos)->m_MoveTo == m_Position + (*pos))) {
						nFlag = TRUE;
						break;
				}
				++connectpos;
			}

			if (!nFlag) { 
				if (Collection.LineFlag) {
					SolidBrush NewBrush(Color(255, 255, 0 , 0));
					pDC->FillRectangle(&NewBrush, m_Position.x + (*pos).x -m_ConnectPointArea, 
						m_Position.y + (*pos).y -m_ConnectPointArea, 2*m_ConnectPointArea, 2*m_ConnectPointArea);	
				}
				else {
					pDC->DrawRectangle(&NewPen, m_Position.x + (*pos).x -m_ConnectPointArea/2, m_Position.y + (*pos).y-m_ConnectPointArea/2
						, m_ConnectPointArea, m_ConnectPointArea);	
				}
			}
			else {
				if (iCount == 0) {
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -2, m_Position.y + (*pos).y -4, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y -4);
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -2, m_Position.y + (*pos).y -2, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y -2);
				}
				else {
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x +4, m_Position.y + (*pos).y -2, m_Position.x + (*pos).x +4, m_Position.y + (*pos).y +2);
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y -2, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y +2);
				}
			}
			++iCount;
			++pos;
		}
	}
}

void CInvertAmpElement::OnEditProperties( list<ElementData> *List /*= NULL*/, CPoint point /*= CPoint(0,0)*/ )
{
	if (List != NULL) {

	}
	else {
		CEditPropertyDlg dlg;
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		int AreaType = normalarea;
		list<OPAmpInsideArea>::iterator pos = m_InsideAreaList.begin();
		while (pos != m_InsideAreaList.end()) {

			if (point.x > m_Position.x + (*pos).Area.left && point.x < m_Position.x +(*pos).Area.right
				&& point.y > m_Position.y + (*pos).Area.top && point.y < m_Position.y +(*pos).Area.bottom) {
					AreaType = (*pos).Insidetype;
					break;
			}
			++pos;
		}

		switch (AreaType) {
		case normalarea:
			break;

		case leftresistance:
			{
				dlg.m_ElementFlag = resistance;
				Property Resistance;
				Resistance.PropertyName = CString(_T("Resistance"));
				Resistance.PropertyValue = m_LeftResistance;
				Resistance.Unit = _T("kΩ");
				PropertyList.push_back(Resistance);

				Property Power;
				Power.PropertyName = CString(_T("Power"));
				Power.PropertyValue = m_LeftPower;
				Power.Unit = _T("V");
				PropertyList.push_back(Power);

				Property Temperture;
				Temperture.PropertyName = CString(_T("Temperture"));
				Temperture.PropertyValue = m_LeftTemperture;
				Temperture.Unit = _T("℃");
				PropertyList.push_back(Temperture);

				Property MaximumVoltage;
				MaximumVoltage.PropertyName = CString(_T("MaximumVoltage"));
				MaximumVoltage.PropertyValue = m_LeftMaximumVoltage;
				MaximumVoltage.Unit = _T("V");
				PropertyList.push_back(MaximumVoltage);

			}
			break;

		case upresistance:
			{
				dlg.m_ElementFlag = resistance;
				Property Resistance;
				Resistance.PropertyName = CString(_T("Resistance"));
				Resistance.PropertyValue = m_UpResistance;
				Resistance.Unit = _T("kΩ");
				PropertyList.push_back(Resistance);

				Property Power;
				Power.PropertyName = CString(_T("Power"));
				Power.PropertyValue = m_UpPower;
				Power.Unit = _T("V");
				PropertyList.push_back(Power);

				Property Temperture;
				Temperture.PropertyName = CString(_T("Temperture"));
				Temperture.PropertyValue = m_UpTemperture;
				Temperture.Unit = _T("℃");
				PropertyList.push_back(Temperture);

				Property MaximumVoltage;
				MaximumVoltage.PropertyName = CString(_T("MaximumVoltage"));
				MaximumVoltage.PropertyValue = m_UpMaximumVoltage;
				MaximumVoltage.Unit = _T("V");
				PropertyList.push_back(MaximumVoltage);
			}
			break;

		case maxvoltage:
			{
				Property MaxVoltage;
				MaxVoltage.PropertyName = CString(_T("MaxVoltage"));
				MaxVoltage.PropertyValue = m_MaxVoltage;
				MaxVoltage.Unit = _T("V");
				PropertyList.push_back(MaxVoltage);
			}
			break;

		case minvoltage:
			{
				Property MinVoltage;
				MinVoltage.PropertyName = CString(_T("MinVoltage"));
				MinVoltage.PropertyValue = m_MinVoltage;
				MinVoltage.Unit = _T("V");
				PropertyList.push_back(MinVoltage);
			}
			break;
		}


		dlg.InitializeProperty(PropertyList);	
		if (dlg.DoModal() == IDOK) {
			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			switch (AreaType) {
			case normalarea:
				break;

			case leftresistance:
				{
					m_LeftResistance = PropertyList[0].PropertyValue;
					m_LeftPower = PropertyList[1].PropertyValue;
					m_LeftTemperture = PropertyList[2].PropertyValue;
					m_LeftMaximumVoltage = PropertyList[3].PropertyValue;
				}
				break;

			case upresistance:
				{
					m_UpResistance = PropertyList[0].PropertyValue;
					m_UpPower = PropertyList[1].PropertyValue;
					m_UpTemperture = PropertyList[2].PropertyValue;
					m_UpMaximumVoltage = PropertyList[3].PropertyValue;
				}
				break;

			case maxvoltage:
				{
					m_MaxVoltage = PropertyList[0].PropertyValue;
					if (m_MaxVoltage <0) {
						m_MaxVoltage = -m_MaxVoltage;
					}
				}

				break;

			case minvoltage:
				{
					m_MinVoltage = PropertyList[0].PropertyValue;
					if (m_MinVoltage > 0) {
						m_MinVoltage = -m_MinVoltage;
					}
				}
				break;
			}
		}
	}
}

CBaseElement* CInvertAmpElement::Clone()
{
	CInvertAmpElement *pCloneElement = new CInvertAmpElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	////////
	pCloneElement->m_LeftResistance = m_LeftResistance;
	pCloneElement->m_LeftPower = m_LeftPower;
	pCloneElement->m_LeftTemperture = m_LeftTemperture;
	pCloneElement->m_LeftMaximumVoltage = m_LeftMaximumVoltage;

	pCloneElement->m_UpResistance = m_UpResistance;
	pCloneElement->m_UpPower = m_UpPower;
	pCloneElement->m_UpTemperture = m_UpTemperture;
	pCloneElement->m_UpMaximumVoltage = m_UpMaximumVoltage;

	pCloneElement->m_MaxVoltage = m_MaxVoltage;
	pCloneElement->m_MinVoltage = m_MinVoltage;
	return pCloneElement;
}

void CInvertAmpElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewLeftResistance;
	NewLeftResistance.Text = CString(_T("LeftResistance"));
	NewLeftResistance.Value = m_LeftResistance;
	List.push_back(NewLeftResistance);

	ElementData NewUpResistance;
	NewUpResistance.Text = CString(_T("UpResistance"));
	NewUpResistance.Value = m_UpResistance;
	List.push_back(NewUpResistance);

	ElementData NewMaxVoltage;
	NewMaxVoltage.Text = CString(_T("MaxVoltage"));
	NewMaxVoltage.Value = m_MaxVoltage;
	List.push_back(NewMaxVoltage);

	ElementData NewMinVoltage;
	NewMinVoltage.Text = CString(_T("MinVoltage"));
	NewMinVoltage.Value = m_MinVoltage;
	List.push_back(NewMinVoltage);

	ElementData NewGainVoltagerate;
	NewGainVoltagerate.Text = CString(_T("GainVoltagerate"));
	NewGainVoltagerate.Value = -(m_UpResistance/m_LeftResistance);
	List.push_back(NewGainVoltagerate);
}

void CInvertAmpElement::RotationElement()
{

}


////////////////////////////////////////////////////////////////////////// 비반전 증폭기
CNonInverAmpElement::CNonInverAmpElement()
:CBaseElement(NonInvertingAmplifier)
{
	m_LeftDownPower = m_LeftDownTemperture = m_LeftDownMaximumVoltage = 1.0f;
	m_LeftDownResistance = 1;
	m_Unit = 1e+3;

	m_BottomPower = m_BottomTemperture = m_BottomMaximumVoltage = 1.0f;
	m_BottomResistance = 1;

	m_MaxVoltage = 12;
	m_MinVoltage = -12;

	OPAmpInsideArea LeftBottomResistance;
	LeftBottomResistance.Insidetype = leftbottomresistance;
	LeftBottomResistance.Area = CRect(-75, 90, -40, 150);
	m_InsideAreaList.push_back(LeftBottomResistance);

	OPAmpInsideArea BottomResistance;
	BottomResistance.Insidetype = bottomresistance;
	BottomResistance.Area = CRect(-35, 70, 25, 100);
	m_InsideAreaList.push_back(BottomResistance);

	OPAmpInsideArea MaxVoltage;
	MaxVoltage.Insidetype = maxvoltage;
	MaxVoltage.Area = CRect(-5, -55, 25, -25);
	m_InsideAreaList.push_back(MaxVoltage);

	OPAmpInsideArea MinVoltage;
	MinVoltage.Insidetype = minvoltage;
	MinVoltage.Area = CRect(-5, +50, 25, +70);
	m_InsideAreaList.push_back(MinVoltage);
}

CNonInverAmpElement::~CNonInverAmpElement()
{

}

void CNonInverAmpElement::Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint /*= CPoint(0,0)*/ )
{
	Gdiplus::Font font(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	SolidBrush Greensbrush(Color(255, 0, 128, 128));
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen NewPen(Color(255, 0, 0 ,0), 1.0f);
	Pen BluePen(Color(255, 28, 28, 160), 2.0f);
	Pen GreenPen(Color(255, 0, 128, 128), 1.0f);
	Pen RedPen(Color(255, 255, 0 , 0), 1.0f);

	if (Collection.Selectflag) {
		NewPen.SetColor(Color(255, 255, 0 , 0));
		sbrush.SetColor(Color(255, 255, 0 , 0));
		Greensbrush.SetColor(Color(255, 255, 0 , 0));
		BluePen.SetColor(Color(255, 255, 0 , 0));
		GreenPen.SetColor(Color(255, 255, 0 , 0));
	}

	//삼각형
	GraphicsPath path;
	path.AddLine(m_Position.x -20, m_Position.y -10, m_Position.x -20, m_Position.y +34);
	path.AddLine(m_Position.x -20, m_Position.y +34, m_Position.x +20, m_Position.y +12);
	path.AddLine(m_Position.x +20, m_Position.y +12, m_Position.x -20, m_Position.y -10);
	pDC->DrawPath(&BluePen, &path);
	CString Name(_T("OP1 uA741"));
	pDC->DrawString(Name, -1, &font, RectF((float)m_Position.x, (float)m_Position.y -20, (float)m_Position.x, (float)m_Position.y -20), &format, &Greensbrush);

	//왼쪽 라인
	pDC->DrawLine(&NewPen, m_Position.x -100, m_Position.y, m_Position.x -20, m_Position.y);
	pDC->DrawLine(&GreenPen, m_Position.x -17, m_Position.y, m_Position.x - 11, m_Position.y);
	pDC->DrawLine(&GreenPen, m_Position.x -14, m_Position.y -3, m_Position.x - 14, m_Position.y +3);

	pDC->DrawLine(&RedPen, m_Position.x -26, m_Position.y -2, m_Position.x -26, m_Position.y +2);		//red
	pDC->DrawLine(&RedPen, m_Position.x -24, m_Position.y -2, m_Position.x -24, m_Position.y +2);		//red

	//왼쪽 아래 및 저항 + 그라운드

	pDC->DrawLine(&NewPen, m_Position.x - 55, m_Position.y +26, m_Position.x -20, m_Position.y +26);
	pDC->DrawLine(&NewPen, m_Position.x - 55, m_Position.y +26, m_Position.x -55, m_Position.y +100);

	pDC->DrawLine(&RedPen, m_Position.x -24, m_Position.y +24, m_Position.x -24, m_Position.y +28);		//red
	pDC->DrawLine(&RedPen, m_Position.x -26, m_Position.y +24, m_Position.x -26, m_Position.y +28);

	//왼쪽 아래 저항
	PointF Point1((float)m_Position.x -55, (float)m_Position.y +100);
	PointF Point2((float)m_Position.x -55, (float)m_Position.y +107);
	PointF Point3((float)m_Position.x -60, (float)m_Position.y +110);
	PointF Point4((float)m_Position.x -50, (float)m_Position.y +114);
	PointF Point5((float)m_Position.x -60, (float)m_Position.y +118);

	PointF Point6((float)m_Position.x -50,(float) m_Position.y +122);
	PointF Point7((float)m_Position.x -60, (float)m_Position.y +126);
	PointF Point8((float)m_Position.x -50, (float)m_Position.y +130);
	PointF Point9((float)m_Position.x -55, (float)m_Position.y +133);
	PointF Point10((float)m_Position.x -55, (float)m_Position.y +140);

	PointF Points[10] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10};

	NewPen.SetLineJoin(LineJoinRound);
	pDC->DrawLines(&GreenPen, Points, 10);

	pDC->DrawLine(&RedPen, m_Position.x -53, m_Position.y +100, m_Position.x -57, m_Position.y +100);		//red
	pDC->DrawLine(&RedPen, m_Position.x -53, m_Position.y +102, m_Position.x -57, m_Position.y +102);		//red
	pDC->DrawLine(&RedPen, m_Position.x -53, m_Position.y +138, m_Position.x -57, m_Position.y +138);		//red
	pDC->DrawLine(&RedPen, m_Position.x -53, m_Position.y +140, m_Position.x -57, m_Position.y +140);		//red

	CString m_LeftDownResistanceName;
	m_LeftDownResistanceName.Format(_T("  R %.0fk"), m_LeftDownResistance);
	StringFormat Verformat;
	Verformat.SetFormatFlags(StringFormatFlagsDirectionVertical);
	pDC->DrawString(m_LeftDownResistanceName, -1, &font, RectF((float)m_Position.x -50, (float)m_Position.y +105, (float)m_Position.x -50, (float)m_Position.y +105), &Verformat, &Greensbrush);

	//그라운드
	pDC->DrawLine(&NewPen, m_Position.x -55, m_Position.y +140, m_Position.x -55, m_Position.y +160);
	pDC->DrawLine(&GreenPen, m_Position.x -62, m_Position.y +160, m_Position.x -48, m_Position.y +160);
	pDC->DrawLine(&GreenPen, m_Position.x -59, m_Position.y +163, m_Position.x -51, m_Position.y +163);
	pDC->DrawLine(&GreenPen, m_Position.x - 56, m_Position.y +166, m_Position.x -54, m_Position.y +166);

	pDC->DrawLine(&RedPen, m_Position.x -57, m_Position.y +154, m_Position.x -53, m_Position.y +154);	//red
	pDC->DrawLine(&RedPen, m_Position.x -57, m_Position.y +156, m_Position.x -53, m_Position.y +156);	//red


	//병렬 라인 및 저항
	pDC->DrawLine(&NewPen, m_Position.x -55, m_Position.y +90, m_Position.x-20, m_Position.y +90);
	{
		PointF Point1((float)m_Position.x -20, (float)m_Position.y +90); 
		PointF Point2((float)m_Position.x -13, (float)m_Position.y +90); 
		PointF Point3((float)m_Position.x -10, (float)m_Position.y +85);
		PointF Point4((float)m_Position.x -6, (float)m_Position.y  +95);
		PointF Point5((float)m_Position.x -2, (float)m_Position.y +85); 

		PointF Point6((float)m_Position.x +2,(float) m_Position.y +95); 
		PointF Point7((float)m_Position.x +6, (float)m_Position.y +85);
		PointF Point8((float)m_Position.x +10, (float)m_Position.y +95); 
		PointF Point9((float)m_Position.x +13, (float)m_Position.y +90); 
		PointF Point10((float)m_Position.x +20, (float)m_Position.y +90);

		PointF Points[10] = {Point1, Point2, Point3, Point4, Point5, Point6, Point7, Point8, Point9, Point10};

		NewPen.SetLineJoin(LineJoinRound);
		pDC->DrawLines(&GreenPen, Points, 10);

		pDC->DrawLine(&RedPen, m_Position.x -20, m_Position.y +88, m_Position.x -20, m_Position.y +92);		//red
		pDC->DrawLine(&RedPen, m_Position.x -18, m_Position.y +88, m_Position.x -18, m_Position.y +92);		//red

		pDC->DrawLine(&RedPen, m_Position.x +20, m_Position.y +88, m_Position.x +20, m_Position.y +92);		//red
		pDC->DrawLine(&RedPen, m_Position.x +18, m_Position.y +88, m_Position.x +18, m_Position.y +92);		//red

		CString BottomResistanceName;
		BottomResistanceName.Format(_T("  R %.0fk"), m_BottomResistance);
		pDC->DrawString(BottomResistanceName, -1, &font, RectF((float)m_Position.x -19, (float)m_Position.y +70, (float)m_Position.x -19, (float)m_Position.y +70), &format, &Greensbrush);

	}
	pDC->DrawLine(&NewPen, m_Position.x +20, m_Position.y +90, m_Position.x +50, m_Position.y +90);
	pDC->DrawLine(&NewPen, m_Position.x +50, m_Position.y +90, m_Position.x +50, m_Position.y +12);

	pDC->FillEllipse(&sbrush, RectF((float)m_Position.x -58, (float)m_Position.y +87, (float)6, (float)6));
	pDC->FillEllipse(&sbrush, RectF((float)m_Position.x +47, (float)m_Position.y +9, (float)6, (float)6));


	//오른쪽 위 +
	pDC->DrawLine(&GreenPen, m_Position.x -8, m_Position.y +6, m_Position.x -2, m_Position.y +6);
	pDC->DrawLine(&GreenPen, m_Position.x -5, m_Position.y +3, m_Position.x -5, m_Position.y +9);

	//Plus voltage
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y -2, m_Position.x -5, m_Position.y -27);
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y -27, m_Position.x +5, m_Position.y -27);
	pDC->DrawLine(&GreenPen, m_Position.x +5, m_Position.y -31, m_Position.x +5, m_Position.y -23);

	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y -8, m_Position.x -2, m_Position.y -8);		//red
	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y -6, m_Position.x -2, m_Position.y -6);		//red
	pDC->DrawLine(&RedPen, m_Position.x, m_Position.y -29, m_Position.x, m_Position.y -25);			//red
	pDC->DrawLine(&RedPen, m_Position.x +2, m_Position.y -29, m_Position.x +2, m_Position.y -25);		//red

	CString MaxVoltageName;
	MaxVoltageName.Format(_T("%.0fV"), m_MaxVoltage);
	pDC->DrawString(MaxVoltageName, -1, &font, RectF((float)m_Position.x, (float)m_Position.y -45, (float)m_Position.x, (float)m_Position.y -45), &format, &Greensbrush);

	//오른쪽 아래 -
	pDC->DrawLine(&GreenPen, m_Position.x -17, m_Position.y+24, m_Position.x - 11, m_Position.y +24);

	//Minus voltage
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y +27, m_Position.x -5, m_Position.y +52);
	pDC->DrawLine(&NewPen, m_Position.x -5, m_Position.y +52, m_Position.x +5, m_Position.y +52);
	pDC->DrawLine(&GreenPen, m_Position.x +5, m_Position.y +48, m_Position.x +5, m_Position.y +56);

	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y +31, m_Position.x -2, m_Position.y +31);		//red
	pDC->DrawLine(&RedPen, m_Position.x -7, m_Position.y +33, m_Position.x -2, m_Position.y +33);		//red
	pDC->DrawLine(&RedPen, m_Position.x, m_Position.y +50, m_Position.x, m_Position.y +54);			//red
	pDC->DrawLine(&RedPen, m_Position.x +2, m_Position.y +50, m_Position.x +2, m_Position.y +54);		//red

	CString MinVoltageName;
	MinVoltageName.Format(_T("%.0fV"), m_MinVoltage);
	pDC->DrawString(MinVoltageName, -1, &font, RectF((float)m_Position.x, (float)m_Position.y +55, (float)m_Position.x, (float)m_Position.y +55), &format, &Greensbrush);

	//오른쪽 라인
	pDC->DrawLine(&NewPen, m_Position.x +20, m_Position.y +12, m_Position.x +100, m_Position.y +12);
	pDC->DrawLine(&NewPen, m_Position.x +100, m_Position.y +12, m_Position.x +100, m_Position.y);



	//라인 연결 구간
	{
		Pen NewPen(Color(255, 0, 128 , 128), 1.0f);
		Pen RedPen(Color(255, 255, 0 , 0), 1.0f);
		if (Collection.Selectflag) {
			NewPen.SetColor(Color(255, 255, 0 , 0));
		}

		int iCount = 0;
		list<CPoint>::iterator pos = m_ConnectPoint.begin();	//연결 구간
		while (pos != m_ConnectPoint.end()) {

			BOOL nFlag = FALSE;
			list<CBaseElement*>::iterator connectpos = m_ConnectElement.begin();
			while (connectpos != m_ConnectElement.end()) {
				if ((*connectpos)->m_Flag == lines && 
					((*connectpos)->m_LineTo == m_Position + (*pos) || (*connectpos)->m_MoveTo == m_Position + (*pos))) {
						nFlag = TRUE;
						break;
				}
				++connectpos;
			}

			if (!nFlag) { 
				if (Collection.LineFlag) {
					SolidBrush NewBrush(Color(255, 255, 0 , 0));
					pDC->FillRectangle(&NewBrush, m_Position.x + (*pos).x -m_ConnectPointArea, 
						m_Position.y + (*pos).y -m_ConnectPointArea, 2*m_ConnectPointArea, 2*m_ConnectPointArea);	
				}
				else {
					pDC->DrawRectangle(&NewPen, m_Position.x + (*pos).x -m_ConnectPointArea/2, m_Position.y + (*pos).y-m_ConnectPointArea/2
						, m_ConnectPointArea, m_ConnectPointArea);	
				}
			}
			else {
				if (iCount == 0) {
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -2, m_Position.y + (*pos).y +4, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y +4);
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x -2, m_Position.y + (*pos).y +2, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y +2);
				}
				else {
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x +4, m_Position.y + (*pos).y -2, m_Position.x + (*pos).x +4, m_Position.y + (*pos).y +2);
					pDC->DrawLine(&RedPen, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y -2, m_Position.x + (*pos).x +2, m_Position.y + (*pos).y +2);
				}
			}
			++iCount;
			++pos;
		}
	}
}

void CNonInverAmpElement::OnEditProperties( list<ElementData> *List /*= NULL*/, CPoint point /*= CPoint(0,0)*/ )
{
	if (List != NULL) {
	}
	else {
		CEditPropertyDlg dlg;
		vector<Property> PropertyList;
		dlg.m_strElementName = m_Name;
		dlg.m_GroupName = m_GroupName;
		dlg.m_ElementFlag = m_Flag;

		int AreaType = normalarea;
		list<OPAmpInsideArea>::iterator pos = m_InsideAreaList.begin();
		while (pos != m_InsideAreaList.end()) {

			if (point.x > m_Position.x + (*pos).Area.left && point.x < m_Position.x +(*pos).Area.right
				&& point.y > m_Position.y + (*pos).Area.top && point.y < m_Position.y +(*pos).Area.bottom) {
					AreaType = (*pos).Insidetype;
					break;
			}
			++pos;
		}

		switch (AreaType) {
		case normalarea:
			break;

		case leftbottomresistance:
			{
				dlg.m_ElementFlag = resistance;
				Property Resistance;
				Resistance.PropertyName = CString(_T("Resistance"));
				Resistance.PropertyValue = m_LeftDownResistance;
				Resistance.Unit = _T("kΩ");
				PropertyList.push_back(Resistance);

				Property Power;
				Power.PropertyName = CString(_T("Power"));
				Power.PropertyValue = m_LeftDownPower;
				Power.Unit = _T("V");
				PropertyList.push_back(Power);

				Property Temperture;
				Temperture.PropertyName = CString(_T("Temperture"));
				Temperture.PropertyValue = m_LeftDownTemperture;
				Temperture.Unit = _T("℃");
				PropertyList.push_back(Temperture);

				Property MaximumVoltage;
				MaximumVoltage.PropertyName = CString(_T("MaximumVoltage"));
				MaximumVoltage.PropertyValue = m_LeftDownMaximumVoltage;
				MaximumVoltage.Unit = _T("V");
				PropertyList.push_back(MaximumVoltage);

			}
			break;

		case bottomresistance:
			{
				dlg.m_ElementFlag = resistance;
				Property Resistance;
				Resistance.PropertyName = CString(_T("Resistance"));
				Resistance.PropertyValue = m_BottomResistance;
				Resistance.Unit = _T("kΩ");
				PropertyList.push_back(Resistance);

				Property Power;
				Power.PropertyName = CString(_T("Power"));
				Power.PropertyValue = m_BottomPower;
				Power.Unit = _T("V");
				PropertyList.push_back(Power);

				Property Temperture;
				Temperture.PropertyName = CString(_T("Temperture"));
				Temperture.PropertyValue = m_BottomTemperture;
				Temperture.Unit = _T("℃");
				PropertyList.push_back(Temperture);

				Property MaximumVoltage;
				MaximumVoltage.PropertyName = CString(_T("MaximumVoltage"));
				MaximumVoltage.PropertyValue = m_BottomMaximumVoltage;
				MaximumVoltage.Unit = _T("V");
				PropertyList.push_back(MaximumVoltage);
			}
			break;

		case maxvoltage:
			{
				Property MaxVoltage;
				MaxVoltage.PropertyName = CString(_T("MaxVoltage"));
				MaxVoltage.PropertyValue = m_MaxVoltage;
				MaxVoltage.Unit = _T("V");
				PropertyList.push_back(MaxVoltage);
			}
			break;

		case minvoltage:
			{
				Property MinVoltage;
				MinVoltage.PropertyName = CString(_T("MinVoltage"));
				MinVoltage.PropertyValue = m_MinVoltage;
				MinVoltage.Unit = _T("V");
				PropertyList.push_back(MinVoltage);
			}
			break;
		}


		dlg.InitializeProperty(PropertyList);	
		if (dlg.DoModal() == IDOK) {
			PropertyList.clear();
			PropertyList.resize(dlg.m_PropertyList.size());
			copy(dlg.m_PropertyList.begin(), dlg.m_PropertyList.end(), PropertyList.begin());

			switch (AreaType) {
			case normalarea:
				break;

			case leftbottomresistance:
				{
					m_LeftDownResistance = PropertyList[0].PropertyValue;
					m_LeftDownPower = PropertyList[1].PropertyValue;
					m_LeftDownTemperture = PropertyList[2].PropertyValue;
					m_LeftDownMaximumVoltage = PropertyList[3].PropertyValue;
				}
				break;

			case bottomresistance:
				{
					m_BottomResistance = PropertyList[0].PropertyValue;
					m_BottomPower = PropertyList[1].PropertyValue;
					m_BottomTemperture = PropertyList[2].PropertyValue;
					m_BottomMaximumVoltage = PropertyList[3].PropertyValue;
				}
				break;

			case maxvoltage:
				{
					m_MaxVoltage = PropertyList[0].PropertyValue;
					if (m_MaxVoltage <0) {
						m_MaxVoltage = -m_MaxVoltage;
					}
				}
				break;

			case minvoltage:
				{
					m_MinVoltage = PropertyList[0].PropertyValue;
					if (m_MinVoltage > 0) {
						m_MinVoltage = -m_MinVoltage;
					}
				}
				break;
			}
		}
	}
}

CBaseElement* CNonInverAmpElement::Clone()
{
	CNonInverAmpElement *pCloneElement = new CNonInverAmpElement;

	pCloneElement->m_GroupName = m_GroupName;
	pCloneElement->m_Name = m_Name;
	pCloneElement->m_Flag = m_Flag;
	pCloneElement->m_ElementNumber = m_ElementNumber;

	pCloneElement->m_Position = m_Position;
	pCloneElement->m_PositionArea = m_PositionArea;

	pCloneElement->m_ConnectPoint.clear();
	pCloneElement->m_ConnectPoint.resize(m_ConnectPoint.size());
	std::copy(m_ConnectPoint.begin(), m_ConnectPoint.end(), pCloneElement->m_ConnectPoint.begin());
	pCloneElement->m_ConnectPointArea = m_ConnectPointArea;

	pCloneElement->m_LineTo = m_LineTo;
	pCloneElement->m_MoveTo = m_MoveTo;

	pCloneElement->m_Axis = m_Axis;
	pCloneElement->m_LineMoveDirection = m_LineMoveDirection;
	pCloneElement->m_ConnnectCount = m_ConnnectCount;
	pCloneElement->m_CheckFlag = m_CheckFlag;
	pCloneElement->m_GroundFlag = m_GroundFlag;
	pCloneElement->m_RotationFlag = m_RotationFlag;
	pCloneElement->m_UpdataFlag = m_UpdataFlag;

	////
	pCloneElement->m_LeftDownResistance = m_LeftDownResistance;
	pCloneElement->m_LeftDownPower = m_LeftDownPower;
	pCloneElement->m_LeftDownTemperture = m_LeftDownTemperture;
	pCloneElement->m_LeftDownMaximumVoltage = m_LeftDownMaximumVoltage;

	pCloneElement->m_BottomResistance = m_BottomResistance;
	pCloneElement->m_BottomPower = m_BottomPower;
	pCloneElement->m_BottomTemperture = m_BottomTemperture;
	pCloneElement->m_BottomMaximumVoltage = m_BottomMaximumVoltage;

	pCloneElement->m_MaxVoltage = m_MaxVoltage;
	pCloneElement->m_MinVoltage = m_MinVoltage;

	return pCloneElement;
}

void CNonInverAmpElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/ )
{
	ElementData NewMaxVoltage;
	NewMaxVoltage.Text = CString(_T("MaxVoltage"));
	NewMaxVoltage.Value = m_MaxVoltage;
	List.push_back(NewMaxVoltage);

	ElementData NewMinVoltage;
	NewMinVoltage.Text = CString(_T("MinVoltage"));
	NewMinVoltage.Value = m_MinVoltage;
	List.push_back(NewMinVoltage);

	ElementData NewGainVoltagerate;
	NewGainVoltagerate.Text = CString(_T("GainVoltagerate"));
	NewGainVoltagerate.Value = (1+ m_BottomResistance/m_LeftDownResistance);
	List.push_back(NewGainVoltagerate);
}

void CNonInverAmpElement::RotationElement()
{

}
