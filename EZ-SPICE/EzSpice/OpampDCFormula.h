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
	list<CBaseElement*>			m_CircuitList;			//ȸ�� ����
	list<RecoveryConnectElment>	m_CircuitRecoveryList;	//ȸ�� �������� ����

	list<CBaseElement*>			m_TempCircuitList;		//ȸ�� ���Һ�
	list<CBaseElement*>			m_TempList;				//��ü ������ �޸� ���� ����

public:
	void Initialize();	
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);		//��ü ���μ���
	void AheadProcessing(list<IPCData> &Datalist);														//��ó��
	void RearProcessing(list<IPCData> &Datalist);														//��ó��

	int CheckCircuit(list<CBaseElement*> &CurrentList);													//AC,DC �˻�
	BOOL DivideLeftCircuit(double &InputVoltage, list<IPCData> &LeftRealDatalist, list<CBaseElement*> &Provlist);//Opamp�� �������� ���� ȸ�� �и� �� ����
	BOOL DivideRightCircuit(double InputVoltage, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);//Opamp�� �������� ������ ȸ�� �и� �� ����
	void RecursiveCircuitSearch(CBaseElement* pElement);												//��� ȸ�� Ž��
	CBaseElement* GetElementGroup(int Collectiontype, double inputVoltage, CBaseElement *pOPampElement);//���� ���� �÷��� ������ ����
};
