#pragma once
#include "StructInformation.h"

class LinearDCFormula
{
public:
	LinearDCFormula(void);
	~LinearDCFormula(void);

private:
	list<Node>			m_NodeList;

	double				**m_StencilBuffer;
	double				*m_Result;

public:
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);
	void LinearStencilConvert();															//���� ���ٽ�ȭ  (��Ʈ���� ��ȯ)
	double GetElementProperty(CBaseElement *pElement, CString &PropertyString);				//�ش� ������ Ư���� ��ȯ
	BOOL GaussianElimination();																//����þ� �ҰŹ� ����
	double GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//���ٽ�(input ����)
	double GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//���ٽ�(output ����)
	int IsDuplicateNode(Node &BaseNode, CBaseElement *pElement);							//���ٽ�(�ߺ� ����)
	double GetCombinedResistance(Node &CurrentNode);										//���ٽ�(��ü ����)
	void IPCDataConvert(list<IPCData> &Datalist, list<CBaseElement*> &Provlist);			//������ ����


};

