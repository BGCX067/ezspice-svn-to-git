#pragma once
#include "StructInformation.h"
#include "BaseElement.h"
#include <algorithm>

class OpampACFormula
{
public:
	OpampACFormula(void);
	~OpampACFormula(void);

public:
	list<CBaseElement*>			m_CircuitList;			//ȸ�� ����
	list<RecoveryConnectElment>	m_CircuitRecoveryList;	//ȸ�� �������� ����

	list<CBaseElement*>			m_TempCircuitList;		//ȸ�� ���Һ�
	list<CBaseElement*>			m_TempList;				//��ü ������ �޸� ���� ����

public:
	void Initialize();																									//�ʱ�ȭ
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);						//��ü ���μ���
	void AheadProcessing(list<IPCData> &Datalist);																		//��ó��
	void RearProcessing(list<IPCData> &Datalist);																		//��ó��

	int CheckCircuit(list<CBaseElement*> &CurrentList);																	//AC,DC �˻�
	BOOL DivideLeftCircuit(list<IPCData> &LeftCircuitDataList, list<IPCData> &LeftRealDatalist, list<CBaseElement*> &Provlist);	//Opamp�� �������� ���� ȸ�� �и� �� ����
	BOOL DivideRightCircuit(list<IPCData> LeftCircuitDataList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);			//Opamp�� �������� ������ ȸ�� �и� �� ����
	void RecursiveCircuitSearch(CBaseElement* pElement);																		//��� ȸ�� Ž��
	CBaseElement* GetElementGroup(int Collectiontype, double inputVoltage, CBaseElement *pOPampElement);						//���� ���� �÷��� ������ ����
};
