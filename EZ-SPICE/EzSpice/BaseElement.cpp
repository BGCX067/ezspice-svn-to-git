#include "StdAfx.h"
#include "BaseElement.h"

#include <algorithm>
#include <math.h>

enum DrawStates {
	selection, Connectline, insert
};

int	CBaseElement::m_DrawState = selection;

CBaseElement::CBaseElement( int Flag )
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

int CBaseElement::ConnectPositionDirection(const CPoint& point)
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

void CBaseElement::OnEditProperties(list<ElementData> *List/* = NULL*/, CPoint point /*= CPoint(0,0)*/)	
{

}

CBaseElement* CBaseElement::Clone()
{
	return NULL;
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

void CBaseElement::GetProperties( list<ElementData> &List, CPoint point /*= CPoint(0,0)*/)
{

}

BOOL CBaseElement::IsDuplicateConnectPoint( CPoint &point )
{
	//현재 위치에 라인이 이미 연결되어있는지 확인
	list<CBaseElement*>::iterator linepos = m_ConnectElement.begin();
	while (linepos != m_ConnectElement.end()) {

		if ((*linepos)->m_Flag == lines) {
			if ((*linepos)->m_LineTo == point || (*linepos)->m_MoveTo == point) {
				return TRUE;				
			}
		}
		++linepos;
	}
	return FALSE;
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



