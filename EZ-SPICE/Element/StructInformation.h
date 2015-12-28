#pragma once


enum ElementKind {
	acpower, dcpower, transistor, capacity, resistance, inductor, diode, ground, prov, leq, geq, composition, lines, InvertingAmplifier, NonInvertingAmplifier,
};

enum ConnectDirection {
	LineToConnect, MoveToConnect, inputposition, outputposition,
};

enum InsideArea {
	normalarea, leftresistance, upresistance, leftbottomresistance, bottomresistance, maxvoltage, minvoltage
};

enum RotationFlag
{
	Angle0, Angle90, Angle180, Angle270,
};

enum TROperator
{
	trnormal, off, active, saturated
};

typedef struct _OPAmpInsideArea
{
	int		Insidetype;
	CRect	Area;
}OPAmpInsideArea;


typedef struct _ElementInfo
{
public:
	CString		ElementGroupName;
	CString		ElementName;
	UINT		ElementNumber;
	UINT		ElementImageNumber;
	CString		ElementIntroduceString;

	bool operator()(struct _ElementInfo &rs) const {
		return ElementName == rs.ElementName;
	}
}ElementInfo;


typedef struct _ElementGroup
{
public:
	CString				GroupName;	
	list<ElementInfo>	ElementList;

	bool operator()(struct _ElementGroup &rs) const {
		return GroupName == rs.GroupName;
	}
}ElementGroup;

typedef struct _ElementData
{
	CString			Text;
	double			Value;
	void*			pElement;
}ElementData;

typedef struct _BOOLCollection
{
	BOOL LineFlag;
	BOOL Selectflag;
	BOOL GroundAreaFlag;
	BOOL DecideFlag;
	BOOL LineStringFlag;
}BOOLCollection;

typedef struct _ElementInformation
{
	UINT						m_Flag;								//���� ���� �ѹ�
	UINT						m_ElementNumber;					//���� ����Ʈ �ѹ� (���� ��ü ������ ����)

	CPoint						m_Position;							//���� ��ġ
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

	//CPowerElementDC
	double		m_Voltage;					
	double		m_InternalResistance;

	//CResistanceElement
	double		m_Resistance;
	double		m_Power;
	double		m_Temperture;
	double		m_MaximumVoltage;

	//CInductorElement
	double		m_Inductance;
	//	double		m_Temperture;
	//	double		m_MaximumVoltage;

	//CCapacityElement
	double		m_Capacitance;
	// 	double		m_Temperture;
	// 	double		m_MaximumVoltage;

	//CProvElement
	int		m_ProvColor;

	//CDiodeElement
	double m_OperatingPoint;			//0.9
	double m_ThermalVoltage;			//0.02587
	double m_SaturationCurrent;			//Is = 403e-12, p = e-12
	double m_EmissonConefficient;		//n = 1.57;

	//CPowerElementAC
	double	m_Frequency;	//���ļ�
	double	m_Amplitude;	//�ִ� ����
	int		m_WaveModel;

	//CTransistor
	double m_TROperatingPoint;
	double m_SaturatingPoint;
	double m_Beta;

	//Op-amp
	double	m_Firstresistance;
	double	m_Secondresistance;
	double	m_MaxVoltage;
	double	m_MinVoltage;

	void Serialize(CArchive& ar)
	{
		if (ar.IsStoring())
		{
			ar << m_Flag;
			ar << m_ElementNumber;
			ar << m_Position;
			ar << m_LineTo;
			ar << m_MoveTo;
			ar << m_Axis;
			ar << m_LineMoveDirection;
			ar << m_ConnnectCount;
			ar << m_CheckFlag;
			ar << m_GroundFlag;
			ar << m_RotationFlag;
			ar << m_ProvActivityFlag;
			ar << m_UpdataFlag;
			ar << m_Voltage;
			ar << m_InternalResistance;
			ar << m_Resistance;
			ar << m_Power;
			ar << m_Temperture;
			ar << m_MaximumVoltage;
			ar << m_Inductance;
			ar << m_Capacitance;
			ar << m_ProvColor;
			ar << m_OperatingPoint;
			ar << m_ThermalVoltage;
			ar << m_SaturationCurrent;
			ar << m_EmissonConefficient;
			ar << m_Frequency;
			ar << m_Amplitude;
			ar << m_WaveModel;
			ar << m_Firstresistance;
			ar << m_Secondresistance;
			ar << m_MaxVoltage;
			ar << m_MinVoltage;
			ar << m_TROperatingPoint;
			ar << m_SaturatingPoint;
			ar << m_Beta;
		}
		else
		{
			ar >> m_Flag;
			ar >> m_ElementNumber;
			ar >> m_Position;
			ar >> m_LineTo;
			ar >> m_MoveTo;
			ar >> m_Axis;
			ar >> m_LineMoveDirection;
			ar >> m_ConnnectCount;
			ar >> m_CheckFlag;
			ar >> m_GroundFlag;
			ar >> m_RotationFlag;
			ar >> m_ProvActivityFlag;
			ar >> m_UpdataFlag;
			ar >> m_Voltage;
			ar >> m_InternalResistance;
			ar >> m_Resistance;
			ar >> m_Power;
			ar >> m_Temperture;
			ar >> m_MaximumVoltage;
			ar >> m_Inductance;
			ar >> m_Capacitance;
			ar >> m_ProvColor;
			ar >> m_OperatingPoint;
			ar >> m_ThermalVoltage;
			ar >> m_SaturationCurrent;
			ar >> m_EmissonConefficient;
			ar >> m_Frequency;
			ar >> m_Amplitude;
			ar >> m_WaveModel;
			ar >> m_Firstresistance;
			ar >> m_Secondresistance;
			ar >> m_MaxVoltage;
			ar >> m_MinVoltage;
			ar >> m_TROperatingPoint;
			ar >> m_SaturatingPoint;
			ar >> m_Beta;
		}	
	}
}ElementInformation;