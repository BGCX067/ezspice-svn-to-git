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
	UINT						m_Flag;								//소자 구분 넘버
	UINT						m_ElementNumber;					//소자 유니트 넘버 (같은 객체 생성시 구분)

	CPoint						m_Position;							//소자 위치
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
	double	m_Frequency;	//주파수
	double	m_Amplitude;	//최대 전압
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