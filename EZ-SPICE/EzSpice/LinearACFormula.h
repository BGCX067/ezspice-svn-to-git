#pragma once
#include "StructInformation.h"
#include <complex>

class LinearACFormula
{
public:
	LinearACFormula(void);
	~LinearACFormula(void);

public:
	static double		m_fluidVoltage;
private:
	list<Node>			m_NodeList;
	list<CBaseElement*> m_ProvList;
	list<CBaseElement*> m_TempList;


	list<Node>				m_IsopotentNodeList;
	list<IPCData>			m_IsopotentIPCList;
	list<MappingNodeNumber>	m_IsopotentMappingList;


	list<IPCData>		m_AllNodeDataList;
	list<Node>			m_TempNodeList;		//��� �����
	list<CBaseElement*>	m_TempProvList;		//��� �����
	list<IPCData>		m_TempDataList;		//��� �����
	CBaseElement*		m_TempACPower;
	CBaseElement*		m_TempResistance;

	double				**m_StencilBuffer;
	double				*m_Result;

	int					m_SecMax;
	int					m_VoltMax;

private: //Composition Tree 
	list<NodeTree>		m_NodeTree;

public:
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);
	void LinearStencilConvert(double fluidVoltage);											//���� ���ٽ�ȭ  (��Ʈ���� ��ȯ)
	double GetElementProperty(CBaseElement *pElement, CString &PropertyString);				//�ش� ������ Ư���� ��ȯ
	BOOL GaussianElimination();																//����þ� �ҰŹ� ����
	double GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//���ٽ�(input ����)
	double GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//���ٽ�(output ����)
	int IsDuplicateNode(Node &BaseNode, CBaseElement *pElement);							//���ٽ�(�ߺ� ����)
	double GetCombinedResistance(Node &CurrentNode);										//���ٽ�(��ü ����)
	void IPCDataConvert(list<IPCData> &Datalist, int iCount);//������ ����

	void Initialize(list<IPCData> &Datalist, list<CBaseElement*> &Provlist);				//������ �ʱ�ȭ
	void CircultBackUpList();																//����Ʈ ����
	void RestitutionList();																	//����Ʈ ����
	void AheadProcessing(list<IPCData> &Datalist);											//��ó��
	void RearProcessing(list<IPCData> &Datalist);											//��ó��

	void Thevenintheorem();																	//�׺곭 ���� ���� ��� ȸ�θ� ���ķ� ����
	void SeriesMerger(BOOL &OperationFlag);													//���ı��� ����
	void RarallelMerger(BOOL &OperationFlag);												//���ı��� ���� (���� - �� ���� ����)
	void DuplicateMerger(BOOL &OperationFlag);												//��,�� ��尡 ���� ���� ����
	BOOL IsProvArea(Node &CurrentNode);														//������ ��尡 Prov������� �Ǻ�
	complex<double> GetElementInpedance(CBaseElement *pElement);									//������ ���Ǵ��� ����
	void GetFamilyNode(Node &CurrentNode, CBaseElement *CompareElement, Node **ResultNode);	//CurrentNode�� �����ϰ� CompareElement�� ���� ��� ��ȯ
	int DeleteElementInNode(Node &CurrentNode, CBaseElement *pElement);						//�ش��忡�� ���� ����
	BOOL IsAnotherNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode);		//pElement�� ���� ��带 ������ �ٸ� ��忡 ������ �Ǿ��ִ���?
	void GetDuplicateNode(Node &CurrentNode, list<CBaseElement*> &MergerList);				//�ߺ� ��� ����
	void Phasemovement(list<IPCData> &Datalist);											//���� �̵�
	
	void SectionCompositonImpedance(list<IPCData> &Datalist);								//�� ���� �ռ� ���Ǵ��� ���ϴ� �� ����
	void CompostionProvPostion(list<IPCData> &Datalist, BOOL nFlag = FALSE);				//���κ� ��ġ �ռ� ���Ǵ��� ����(�׺�� ���� ����)	
	double GetCapacitance();																//R-Cȸ���� ĳ�н��Ͻ� ȹ��
	void RCAheadeProcessing(list<CBaseElement*> &Provlist);

	//R-C
	double VoltageDropping(); //���а���

	//Op-amp
	BOOL OPAmpProcessing(list<IPCData> &VoltageList, list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);

	//���� Ư�� ���� �߰�
	double RecusiveParallelSearch(Node &CurrentNode, CBaseElement* CallElement = NULL);
	Node* GetTempNode(int Nodenumber);
	void InsertDivideElement(int NodeNumber, CBaseElement *pElement, double_complex &Complex);
	double GetNodeElectric(int NodeNumber);
	double_complex GetNodeComplex(int NodeNumber);
	double RecusiveMergerSearch(Node &CurrentNode, CBaseElement* CallElement);
	BOOL IsAnotherTempNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode);
	


	//������ ��ȯ
	BOOL RecusiveAnotherDuplicateMerger(Node &CompareNode, BOOL FirstFlag, list<Node> &LowNodeList);
	void ConvertIsopotentialNode();
	void IncreaseNodeImpedance(Node &CurrentNode, list<Node> &LowNodeList);
	void CopyNodeGroup(list<Node> &EmptyList, list<Node> &CopyList, int AmplificationCount);
	void SetElementProperty(CBaseElement *pElement, CString &PropertyString, double Value);
	double_complex GetMappingComplex(int NodeNumber);
	int GetDivideCount(int NodeNumber);
	double LocalPhaseAngle(int NodeNumber);
	double RecusiveGetParallelNode(Node &CurrentNode, BOOL StartFlag, CBaseElement *CallElement, double_complex &DivideComplex, int &ParallelCount, int DivideCount);
};
