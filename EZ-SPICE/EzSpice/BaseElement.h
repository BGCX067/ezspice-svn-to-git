#pragma once

#include "StructInformation.h"

// enum ElementKind {
// 	lines, dcpower, acpower, resistance, ground, inductor, capacity, transistor, prov, diode, leq, geq,
// };

enum ConnectDirection {
	LineToConnect, MoveToConnect, inputposition, outputposition
};


class CEzSpiceView;
class CBaseElement
{
public:
	CBaseElement(int Flag);
	virtual ~CBaseElement(void);

	static int					m_DrawState;
public:	
	CString						m_GroupName;						//소자 그룹
	CString						m_Name;								//소자 이름 (생성시 달라짐)
	UINT						m_Flag;								//소자 구분 넘버
	UINT						m_ElementNumber;					//소자 유니트 넘버 (같은 객체 생성시 구분)

	CPoint						m_Position;							//소자 위치
	CRect						m_PositionArea;						//소자 범위

	std::list<CPoint>			m_ConnectPoint;						//포지션에서의 연결점 위치
	int							m_ConnectPointArea;					//포지션에서의 연결점 범위

	CPoint						m_LineTo;							//라인의 경우에만 사용
	CPoint						m_MoveTo;							//라인의 경우에만 사용
	int							m_Axis;								//라인의 축 
	int							m_LineMoveDirection;				//라인 이동시 이동되는 점 (LineTo or MoveTo is one)
	int							m_ConnnectCount;					//라인에 연결된 갯수

	int							m_CheckFlag;						//진행방향 결정시 사용됨
	BOOL						m_GroundFlag;						//Ground 범위인지 체크
	int							m_RotationFlag;						//회전 Flag;
	BOOL						m_ProvActivityFlag;					//프로그에 라인이 삽입 됬는지 체크
	BOOL						m_UpdataFlag;						//뉴톤 랩손법 적용시 Updata 체크
	BOOL						m_AntidromicFlag;					//객체 회전에 따른 역방향 표시
	
	double						m_Unit;								//단위

public:	
	std::list<CBaseElement*>	m_ConnectElement;					//모든 회로 임시 저장
	std::list<CBaseElement*>	m_InputElement;						
	std::list<CBaseElement*>	m_OutputElement;

public:
	BOOL ObjectInPoint(const CPoint& point);						//전달된 point가 해당 객체 내부에 있는지 검출
	void DeleteElement(CBaseElement *pElement);						//내 연결정보에서 소자 삭제
	CPoint ConnectRectDirection(const CPoint& point);				//전달된 point가 소자의 연결 영역에 포함됬으면 해당 연결영역 리턴, 없으면 0,0 리턴
	CPoint ComparePosition(CPoint &pos);							//전달된 point가 라인의 끝일경우 해당 위치 리턴
	double Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);
	int	GetPositonDirection(CPoint point);							//해당위치가 해당 객체(라인의)와 LineToConnect, MoveToConnect 인지 결정
	int ConnectPositionDirection(const CPoint& point);				//소자의 입력부분인지 출력부분인지 판별
	BOOL IsDuplicateConnectPoint(CPoint &point);					//소자 연결부위에 이미 라인이 연결되어 있냐? (중복 금지)

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};


class CompareBuffer
{
private:
	int		m_ElementNumber;
	int		m_Flag;

public:
	CompareBuffer(int ElementNumber, int Flag) {
		m_ElementNumber = ElementNumber;
		m_Flag = Flag;
	}

	void SetElementNumber(int ElementNumber, int Flag) {
		m_ElementNumber = ElementNumber;
		m_Flag = Flag;
	}

	bool operator()(class CBaseElement *rs) const {
		return (m_ElementNumber == rs->m_ElementNumber && m_Flag == rs->m_Flag);
	}
};
