#pragma once
#include "StructInformation.h"
#include "BaseElement.h"
#include <algorithm>

class OpampDCFormula
{
public:
	OpampDCFormula(void);
	~OpampDCFormula(void);

public:
	list<CBaseElement*>			m_CircuitList;			//회로 원본
	list<RecoveryConnectElment>	m_CircuitRecoveryList;	//회로 연결정보 복사

	list<CBaseElement*>			m_TempCircuitList;		//회로 분할본
	list<CBaseElement*>			m_TempList;				//객체 삭제시 메모리 해제 소자

public:
	void Initialize();	
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);		//전체 프로세스
	void AheadProcessing(list<IPCData> &Datalist);														//전처리
	void RearProcessing(list<IPCData> &Datalist);														//후처리

	int CheckCircuit(list<CBaseElement*> &CurrentList);													//AC,DC 검사
	BOOL DivideLeftCircuit(double &InputVoltage, list<IPCData> &LeftRealDatalist, list<CBaseElement*> &Provlist);//Opamp를 기준으로 왼쪽 회로 분리 및 수행
	BOOL DivideRightCircuit(double InputVoltage, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);//Opamp를 기준으로 오른쪽 회로 분리 및 수행
	void RecursiveCircuitSearch(CBaseElement* pElement);												//재귀 회로 탐색
	CBaseElement* GetElementGroup(int Collectiontype, double inputVoltage, CBaseElement *pOPampElement);//다중 소자 컬랙션 생성후 리턴
};
