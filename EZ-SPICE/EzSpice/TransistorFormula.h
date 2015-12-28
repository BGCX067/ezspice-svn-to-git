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
	//TR����
	double						m_OperatingPoint;
	double						m_SaturatingPoint;
	double						m_Beta;

	//ȸ�� ����
	CBaseElement				*m_Transistor;
	double						m_CollectorVcc;
	double						m_BaseVcc;

public:
	list<Node>					m_NodeList;
	list<CBaseElement*>			m_ProvList;
	list<CBaseElement*>			m_TempList;
	list<RecoveryConnectElment>	m_CircuitRecoveryList;	//ȸ�� �������� ����

	list<DivideCircuitInfo>		m_DivideList;

public:
	list<Node>					m_TempNodeList;		//��� �����
	list<CBaseElement*>			m_TempProvList;		//��� �����
	list<IPCData>				m_TempDataList;		//��� �����

public:
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);

	void CreateIPCDataList(list<Node> &CurrentNodeList, list<IPCData> &CurrentDataList, list<CBaseElement*> &CurrentProvList);		
	BOOL ElementException();																				//���̿��� ����� FALSE
	void Initialize(list<Node> &NodeList, list<CBaseElement*> &Provlist);									//��� �� ���κ� ���� ���� �� IPCdata ����
	void DivideCircit();																					//ȸ�� �и�
	CBaseElement* SearchElement(list<Node> &CurrentNodeList, int ElementType);								//Ÿ�Կ� ���� ���� �˻�(��忡��)
	void RecusiveNodeSearch(Node &CurrnetNode, CBaseElement *CompareElement, DivideCircuitInfo &NewInfo);	//�и� ��� Ž��
	int GetTransistorDirection(Node &CurrnetNode, CBaseElement *pTransistor);								//�и� ��� ���� Ž��
	CBaseElement* GetAlternationElement(int Collectiontype);												//���ο� ���� ����
	void ChangeElement(Node &CurrentNode, CBaseElement* RemoveElement, CBaseElement* AlternationElement);	//��� ���� ���ڸ� �ٸ������� ��ü
	void CreateProvList(DivideCircuitInfo &NewInfo);														//�� ��帮��Ʈ�� ���� ���κ� ����Ʈ ����
	double GetDivideCircuitInformation(int DivideType, CString &attribute);									//�и� ȸ���� ���� ȹ��
	void CheckVoltage(DivideCircuitInfo &NewInfo);															//����������� DC Vcc ����
	DivideCircuitInfo GetDivideCircuitInfo(int DivideType);													//�и� ��� Ÿ������ ȹ��
	void MergerDivideCircuit(DivideCircuitInfo &BaseInfo, DivideCircuitInfo &AddInfo);						//DivideCircuit ����
	void IPCDataConvert(list<IPCData> &DataList);															//IPCData �̵�
	void SetDivideCircuitInfo(int DivideType, DivideCircuitInfo &SaveInfo);
	int GetNodeType(int nodenumber);																		//���� DataList�� ��ġ �ľ�, base, collector, emitter
		

	//�׺�� ���� ��ü ����
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
