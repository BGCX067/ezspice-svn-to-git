#pragma once

#include <complex>

enum ElementKind {
		acpower, dcpower, transistor, capacity, resistance, inductor, diode, ground, prov, leq, geq, composition, lines, InvertingAmplifier, NonInvertingAmplifier,
};

enum Wavemodel
{
	sign, square, triangular
};

enum CirCultCase
{
	linearDC, nonlinearDC, linearAC, nonlinearAC, DCOpamp, ACOpamp, transistorFormula
};

enum Voltagegenerator
{
	DC, AC
};

enum ElementCollection //다중소자 집합
{
	dcpowercollection, acpowercollection, resistancecollection
};

enum RLCCase
{
	rr, rc, rl, rlc				//rr(R-R), rc(R-C), rl(R-L), rlc(R-L-C)
};

enum ConnectPostion
{
	non, inputlist, outputlist
};

enum RotationFlag
{
	Angle0, Angle90, Angle180, Angle270,
};

enum TransistorCircuit
{
	base, collector, emitter, baseemitter, collectoremitter	//base 왼쪽, 왼쪽위 collecotr, 아래 emitter, 왼쪽+아래 baseemitter, 위+아래 collectormitter
};

enum TROperator
{
	trnormal, off, active, saturated
};

#define PIE 3.1415929
#define RADTOANGLE 180.0/3.1415929

#define DivideSector 360
#define MultSector DivideSector/360
#define MaxAmplitude 0.707106		//최대 진폭(실효값 적용시)

//복소수 계산용
typedef std::complex<double> double_complex;	

typedef struct _DivideNode
{
	UINT			ElementNumber;
	double_complex	DivideComplexNumber;
	double			DivideElectric;

	_DivideNode()
	{
		DivideElectric = 0;
	}

	bool operator()(struct _DivideNode &rs) const {
		return ElementNumber == rs.ElementNumber;
	}
}DivideNode;

typedef struct _Node
{
	int						NodeNumber;
	BOOL					FirstFlag;				//시작위치는 Vs = 머시기 이다
	BOOL					DupulicateFlag;			//중복 검사시 Flag
	
	list<CBaseElement*>		AllElement;				
	list<CBaseElement*>		InputElement;
	list<CBaseElement*>		OutputElement;
	list<CBaseElement*>		LineElement;

	_Node() 
	{
		static int UniqueNumber = 0;
		NodeNumber = ++UniqueNumber;
		FirstFlag = FALSE;
		DupulicateFlag = FALSE;
		DivideFlag = FALSE;
	}
	bool operator()(struct _Node &rs) const {
		return NodeNumber == rs.NodeNumber;
	}

	//예외처리
	BOOL				DivideFlag;
	list<DivideNode>	DivideElement;
}Node;

typedef struct _TRNode
{
	Node			NodeData;
	CBaseElement	*CompareElement;
}TRNode;

typedef struct _RaphsonData
{
	CBaseElement*	GeqElement;
	CBaseElement*	LeqElement;
	int				InputNodeNumber;
	int				OutputNodeNumber;
	double			Value;
	double			DifferenceVlaue;

	BOOL			LimitFlag;
	double			LimitVoltage;
}RaphsonData;

typedef struct _RaphsonNode
{
	list<RaphsonData>	RaphsonList;
}RaphsonNode;

typedef struct _ElementData
{
	CString		Text;
	double		Value;
	void*		pElement;
}ElementData;

typedef struct _CapacityNode
{
	CBaseElement			*BaseElement;
	list<CBaseElement*>		m_ConnectElement;
}CapacityNode;

typedef struct _SearchData
{
	CString	 string;
	bool operator()(struct _ElementData &rs) const {
		return string == rs.Text;
	}
}SearchData;


enum Direction
{
	input, output,
};

enum IPCDataType
{
	OciloscopeStart, VoltageData, OciloscopeEnd
};

typedef struct _IPCData
{
	int				NodeNumber;
	int				ProvNumber;
	BOOL			FirstFlag;
	BOOL			ComplexCheckFlag;
	double			NodeElectric;
	double_complex	ComplexNumber;
	double			GrapehValue[DivideSector];
}IPCData;

typedef struct _IPCHeader
{
	int		DataType;
	int		DataCount;
	int		SecMax;
	int		VoltMax; 
}IPCHeader;

typedef struct _IPCSectorHeader
{
	int		Circitcase;				//AC?
	double	Frequency;
	int		DataColor;
}IPCSectorHeader;

typedef struct _BOOLCollection
{
	BOOL LineFlag;
	BOOL Selectflag;
	BOOL GroundAreaFlag;
	BOOL DecideFlag;
	BOOL LineStringFlag;
}BOOLCollection;

typedef struct _DuplicateCPoint
{
	CPoint point;

	bool operator()(CPoint &rs) const {
		return (point.x == rs.x && point.y == rs.y);
	}
}DuplicateCPoint;


typedef struct _ACPowerData
{
	double	Amplitude;
	double	Frequency;
	int		WaveModel;

	_ACPowerData()
	{
		Amplitude = -1;
		Frequency = -1;
		WaveModel = -1;
	}
}ACPowerData;


typedef struct _NodeTree
{
	int						NodeNumber;
	double					CompositionResistance;
	CBaseElement			*pElement;
	list<struct _NodeTree>	ChildNodeList;
}NodeTree;

typedef struct _DuplicateNode
{
	CBaseElement	*pElement;
	Node			*AnotherNode;

	_DuplicateNode()
	{
		pElement = NULL;
		AnotherNode = NULL;
	}
}DuplicateNode;



typedef struct _RecoveryProgress	//진행방향 복구
{
	CBaseElement*		pElement;
	list<CBaseElement*> InputElement;
	list<CBaseElement*> OutputElement;
}RecoveryProgress;

typedef struct _RecoveryConnectElment
{
	CBaseElement*		pElement;
	list<CBaseElement*> ConnectElement;
}RecoveryConnectElment;

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


typedef struct _SearchIPCData
{
	int ProvNumber;

	bool operator()(IPCData &rs) const {
		return ProvNumber == rs.ProvNumber;
	}
}SearchIPCData;


typedef struct _CopyNodeElement
{
	CBaseElement* BaseElement;
	CBaseElement* CopyElement;

	bool operator()(_CopyNodeElement &rs) const {
		return BaseElement == rs.BaseElement;
	}
}CopyNodeElement;

typedef struct _MappingNodeNumber
{
	int			BaseNumber;
	int			CopyNumber;
}MappingNodeNumber;