#pragma once
#include "StructInformation.h"
#include <complex>

typedef struct _DivideCircuitInfo
{
	int					CirciutType;
	double_complex		ComplexNumber;

	list<Node>			NodeList;
	list<IPCData>		DataList;
	list<CBaseElement*>	ProvList;

	//Debug and search
	CBaseElement		*pAddElement;
	int					TRNodeNumber;
}DivideCircuitInfo;

// typedef struct _TRData
// {
// 	double Rb, Re, Rc;
// 	double Ib, Ie, Ic;
// 	double Vb, Ve, Vc;
// 	double Vce;
// }TRData;


class TransistorFormula
{
public:
	TransistorFormula(void);
	~TransistorFormula(void);

private:
	//TR정보
	double						m_OperatingPoint;
	double						m_SaturatingPoint;
	double						m_Beta;

	//회로 정보
	CBaseElement				*m_Transistor;
	double						m_CollectorVcc;
	double						m_BaseVcc;

public:
	list<Node>					m_NodeList;
	list<CBaseElement*>			m_ProvList;
	list<CBaseElement*>			m_TempList;
	list<RecoveryConnectElment>	m_CircuitRecoveryList;	//회로 연결정보 복사

	list<DivideCircuitInfo>		m_DivideList;

public:
	list<Node>					m_TempNodeList;		//잠시 복사용
	list<CBaseElement*>			m_TempProvList;		//잠시 복사용
	list<IPCData>				m_TempDataList;		//잠시 복사용

public:
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);

	void CreateIPCDataList(list<Node> &CurrentNodeList, list<IPCData> &CurrentDataList, list<CBaseElement*> &CurrentProvList);		
	BOOL ElementException();																				//다이오드 검출시 FALSE
	void Initialize(list<Node> &NodeList, list<CBaseElement*> &Provlist);									//노드 및 프로브 정보 복사 및 IPCdata 생성
	void DivideCircit();																					//회로 분리
	CBaseElement* SearchElement(list<Node> &CurrentNodeList, int ElementType);								//타입에 따른 소자 검색(노드에서)
	void RecusiveNodeSearch(Node &CurrnetNode, CBaseElement *CompareElement, DivideCircuitInfo &NewInfo);	//분리 노드 탐색
	int GetTransistorDirection(Node &CurrnetNode, CBaseElement *pTransistor);								//분리 노드 방향 탐색
	CBaseElement* GetAlternationElement(int Collectiontype);												//새로운 구성 리턴
	void ChangeElement(Node &CurrentNode, CBaseElement* RemoveElement, CBaseElement* AlternationElement);	//노드 구성 소자를 다른것으로 교체
	void CreateProvList(DivideCircuitInfo &NewInfo);														//각 노드리스트에 대한 프로브 리스트 생성
	double GetDivideCircuitInformation(int DivideType, CString &attribute);									//분리 회로의 정보 획득
	void CheckVoltage(DivideCircuitInfo &NewInfo);															//노드정보에서 DC Vcc 추출
	DivideCircuitInfo GetDivideCircuitInfo(int DivideType);													//분리 노드 타입으로 획득
	void MergerDivideCircuit(DivideCircuitInfo &BaseInfo, DivideCircuitInfo &AddInfo);						//DivideCircuit 병합
	void IPCDataConvert(list<IPCData> &DataList);															//IPCData 이동
	void SetDivideCircuitInfo(int DivideType, DivideCircuitInfo &SaveInfo);
	int GetNodeType(int nodenumber);																		//현재 DataList의 위치 파악, base, collector, emitter
		

	//테브닌 정리 전체 로직
	void SectionCompositonImpedance();
	void Thevenintheorem();
	void CompostionProvPostion(list<IPCData> &Datalist, BOOL nFlag = FALSE);
	complex<double> GetElementInpedance(CBaseElement *pElement);
	void SeriesMerger(BOOL &OperationFlag);
	void RarallelMerger(BOOL &OperationFlag);
	void DuplicateMerger(BOOL &OperationFlag);
	double GetElementProperty(CBaseElement *pElement, CString &PropertyString);
	BOOL IsProvArea(Node &CurrentNode);
	void GetFamilyNode(Node &CurrentNode, CBaseElement *CompareElement, Node **ResultNode);
	int DeleteElementInNode(Node &CurrentNode, CBaseElement *pElement);
	BOOL IsAnotherNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode);
	void GetDuplicateNode(Node &CurrentNode, list<CBaseElement*> &MergerList);
	





};		
