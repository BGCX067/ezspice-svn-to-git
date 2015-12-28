#pragma once

#include "StructInformation.h"

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
	double Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);	//전달된 point와 라인의 거리 측정
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

class CConnectLine : public CBaseElement
{
public:
	CConnectLine();
	virtual ~CConnectLine();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};


class CPowerElementDC : public CBaseElement
{
public:
	double		m_Voltage;					
	double		m_InternalResistance;

public:
	CPowerElementDC();
	virtual ~CPowerElementDC();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};


class CResistanceElement : public CBaseElement
{
public:
	double		m_Resistance;
	double		m_Power;
	double		m_Temperture;
	double		m_MaximumVoltage;

public:
	CResistanceElement();
	virtual ~CResistanceElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0));
	virtual void RotationElement();	
};

class CGroundElement : public CBaseElement
{
public:
	CGroundElement();
	virtual ~CGroundElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};


class CInductorElement : public CBaseElement
{
private:
	double		m_Frequency;
public:
	double		m_Inductance;
	double		m_Temperture;
	double		m_MaximumVoltage;

public:
	CInductorElement();
	virtual ~CInductorElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

class CCapacityElement : public CBaseElement
{
private:
	double		m_Frequency;
public:
	double		m_Capacitance;
	double		m_Temperture;
	double		m_MaximumVoltage;

	BOOL		m_NullFlag;

public:
	CCapacityElement();
	virtual ~CCapacityElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

class CProvElement : public CBaseElement
{
public:
	int		m_ProvColor;
public:
	CProvElement();
	virtual ~CProvElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

class CDiodeElement : public CBaseElement
{
public:
	double	m_AntidromicFlag;
	double	m_OperatingPoint;			//0.9
	double	m_ThermalVoltage;			//0.02587
	double	m_SaturationCurrent;		//Is = 403e-12, p = e-12
	double	m_EmissonConefficient;		//n = 1.57;
	
public:
	CDiodeElement();
	virtual ~CDiodeElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

//내부사용 클래스
class CLeqElement : public CBaseElement
{
public:
	CBaseElement *m_pGeqElement;

	double m_OperatingPoint;			//Vdo = 0.9
	double m_ThermalVoltage;			//Vt = 0.02587
	double m_SaturationCurrent;			//Is = 403e-12, p = e-12
	double m_EmissonConefficient;		//n = 1.57;

public:
	CLeqElement();
	virtual ~CLeqElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

//내부사용 클래스
class CGeqElement : public CBaseElement
{
public:
	//default
	double m_OperatingPoint;			//Vdo = 0.9
	double m_ThermalVoltage;			//Vt = 0.02587
	double m_SaturationCurrent;			//Is = 403e-12, p = e-12
	double m_EmissonConefficient;		//n = 1.57;

public:
	CGeqElement();
	virtual ~CGeqElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

class CPowerElementAC : public CBaseElement
{
public:
	double	m_Frequency;	//주파수
	double	m_Amplitude;	//최대 전압
	int		m_WaveModel;

public:
	CPowerElementAC();
	virtual ~CPowerElementAC();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

class CTransistorElement : public CBaseElement
{
public:
	int		m_OperatorFlag;
	double  m_OperatingPoint;
	double	m_SaturatingPoint;
	double	m_Beta;

public:
	CTransistorElement();
	virtual ~CTransistorElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

//내부사용 클래스
class CCompositionElement : public CBaseElement
{
private:
	double		m_Resistance;
	double		m_Complexnumber;
public:
	CCompositionElement();
	virtual ~CCompositionElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

class CInvertAmpElement : public CBaseElement
{
public:
	//name R
	double					m_LeftResistance;
	double					m_LeftPower;
	double					m_LeftTemperture;
	double					m_LeftMaximumVoltage;

	//name R
	double					m_UpResistance;
	double					m_UpPower;
	double					m_UpTemperture;
	double					m_UpMaximumVoltage;

	double					m_MaxVoltage;
	double					m_MinVoltage;

	list<OPAmpInsideArea>	m_InsideAreaList;

public:
	CInvertAmpElement();
	virtual ~CInvertAmpElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};

class CNonInverAmpElement :public CBaseElement
{
public:
	//name R
	double					m_LeftDownResistance;
	double					m_LeftDownPower;
	double					m_LeftDownTemperture;
	double					m_LeftDownMaximumVoltage;

	//name R
	double					m_BottomResistance;
	double					m_BottomPower;
	double					m_BottomTemperture;
	double					m_BottomMaximumVoltage;

	double					m_MaxVoltage;
	double					m_MinVoltage;

	list<OPAmpInsideArea>	m_InsideAreaList;

public:
	CNonInverAmpElement();
	virtual ~CNonInverAmpElement();

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};