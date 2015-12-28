#pragma once

#include "StructInformation.h"

class CBaseElement
{
public:
	CBaseElement(int Flag);
	virtual ~CBaseElement(void);

	static int					m_DrawState;			
public:	
	CString						m_GroupName;						//���� �׷�
	CString						m_Name;								//���� �̸� (������ �޶���)
	UINT						m_Flag;								//���� ���� �ѹ�
	UINT						m_ElementNumber;					//���� ����Ʈ �ѹ� (���� ��ü ������ ����)

	CPoint						m_Position;							//���� ��ġ
	CRect						m_PositionArea;						//���� ����
		
	std::list<CPoint>			m_ConnectPoint;						//�����ǿ����� ������ ��ġ
	int							m_ConnectPointArea;					//�����ǿ����� ������ ����

	CPoint						m_LineTo;							//������ ��쿡�� ���
	CPoint						m_MoveTo;							//������ ��쿡�� ���
	int							m_Axis;								//������ �� 
	int							m_LineMoveDirection;				//���� �̵��� �̵��Ǵ� �� (LineTo or MoveTo is one)
	int							m_ConnnectCount;					//���ο� ����� ����

	int							m_CheckFlag;						//������� ������ ����
	BOOL						m_GroundFlag;						//Ground �������� üũ
	int							m_RotationFlag;						//ȸ�� Flag;
	BOOL						m_ProvActivityFlag;					//���α׿� ������ ���� ����� üũ
	BOOL						m_UpdataFlag;						//���� ���չ� ����� Updata üũ
	BOOL						m_AntidromicFlag;					//��ü ȸ���� ���� ������ ǥ��

	double						m_Unit;								//����

public:	
	std::list<CBaseElement*>	m_ConnectElement;					//��� ȸ�� �ӽ� ����
	std::list<CBaseElement*>	m_InputElement;						
	std::list<CBaseElement*>	m_OutputElement;

public:
	BOOL ObjectInPoint(const CPoint& point);						//���޵� point�� �ش� ��ü ���ο� �ִ��� ����
	void DeleteElement(CBaseElement *pElement);						//�� ������������ ���� ����
	CPoint ConnectRectDirection(const CPoint& point);				//���޵� point�� ������ ���� ������ ���ԉ����� �ش� ���῵�� ����, ������ 0,0 ����
	CPoint ComparePosition(CPoint &pos);							//���޵� point�� ������ ���ϰ�� �ش� ��ġ ����
	double Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);	//���޵� point�� ������ �Ÿ� ����
	int	GetPositonDirection(CPoint point);							//�ش���ġ�� �ش� ��ü(������)�� LineToConnect, MoveToConnect ���� ����
	int ConnectPositionDirection(const CPoint& point);				//������ �Էºκ����� ��ºκ����� �Ǻ�
	BOOL IsDuplicateConnectPoint(CPoint &point);					//���� ��������� �̹� ������ ����Ǿ� �ֳ�? (�ߺ� ����)

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

//���λ�� Ŭ����
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

//���λ�� Ŭ����
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
	double	m_Frequency;	//���ļ�
	double	m_Amplitude;	//�ִ� ����
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

//���λ�� Ŭ����
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