#pragma once
#include "StructInformation.h"

class NonLinearDCFormula
{
public:
	NonLinearDCFormula(void);
	~NonLinearDCFormula(void);

private:
	list<Node>			m_NodeList;
	list<RaphsonNode>	m_NewtonRaphsonList;

	double				**m_StencilBuffer;
	double				*m_Result;

	double				*m_BeforeResult;
	int					m_NodeSize;

	double				m_Voltage; //���п� ��
	int					m_MaxCount;

public:
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);
	void NonlinearStencilConvert(int NodeSize);															//���� ���ٽ�ȭ
	double GetElementProperty(CBaseElement *pElement, CString &PropertyString);				//�ش� ������ Ư���� ��ȯ
	BOOL GaussianElimination(int NodeSize);													//����þ� �ҰŹ� ����
	double GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//���ٽ�(input ����)
	double GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//���ٽ�(output ����)
	int IsDuplicateNode(Node &BaseNode, CBaseElement *pElement);							//���ٽ�(�ߺ� ����)
	double GetCombinedResistance(Node &CurrentNode);										//���ٽ�(��ü ����)
	void IPCDataConvert(list<IPCData> &Datalist, list<CBaseElement*> &Provlist);			//������ ����

	BOOL IsDiodeInclusion(Node &CurrentNode);												//���ο� ���̿��� ��ȯ ȸ�ΰ� �����ΰ�?
	double GetCombineLeq(Node &CurrentNode);												//��� Leq�� ȹ��
	BOOL IsCompareAndInsert();																//���չ� ���ǹ�
	CBaseElement* SearchElement(Node &CurrentNode, int Flag);								//��忡�� ��ü ã��
	double GetOutputPlusCombinedResistance(Node &CurrentNode, double *StencilBuffer);
	int IsGetNextNode(int CurrentNodeNumber, CBaseElement* CompareElement);
	double RoundXL(double x, int digits);													//double ���ϴ� �Ҽ��� �ݿø�
};

