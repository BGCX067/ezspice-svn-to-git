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
	//Initialize -> ȸ�� �˻�, ������� ����, �׶���ȭ, ��� ����
	BOOL Initialize(list<CBaseElement*> &CircultList);
	//FormulaConversion -> 
	BOOL FormulaConversion(vector<IPCSectorHeader> &SectorList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);
	//enum CirCultCase �� �ϳ� ���� 
	void CircultDistinction(list<CBaseElement*> &CircultList);
	//linearAC, nonlinearAC �϶� ���ļ� ����
	void FrequencyDitinction(list<CBaseElement*> &CircultList);
	//RC, RLC, RR, RL ����
	void RLCCaseDitinction(list<CBaseElement*> &CircultList);
};
