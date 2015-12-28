#pragma once

#include <list>
#include "StructInformation.h"

class CCircultSimulation
{
private:
	list<Node>			m_NodeList;
public:
	CCircultSimulation(void);
	~CCircultSimulation(void);

public:
	static	int					m_CircultCase;
	static	int					m_RLCCase;
	static	ACPowerData			m_ACData;
	static list<CBaseElement*>  m_CircuitList;

public:
	//Initialize -> 회로 검사, 진행방향 결정, 그라운드화, 노드 검출
	BOOL Initialize(list<CBaseElement*> &CircultList);
	//FormulaConversion -> 
	BOOL FormulaConversion(vector<IPCSectorHeader> &SectorList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);
	//enum CirCultCase 중 하나 결정 
	void CircultDistinction(list<CBaseElement*> &CircultList);
	//linearAC, nonlinearAC 일때 주파수 측정
	void FrequencyDitinction(list<CBaseElement*> &CircultList);
	//RC, RLC, RR, RL 측정
	void RLCCaseDitinction(list<CBaseElement*> &CircultList);
};
