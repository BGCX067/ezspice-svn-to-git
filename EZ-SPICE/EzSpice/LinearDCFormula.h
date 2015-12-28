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
	void LinearStencilConvert();															//선형 스텐실화  (메트릭스 변환)
	double GetElementProperty(CBaseElement *pElement, CString &PropertyString);				//해당 소자의 특성값 반환
	BOOL GaussianElimination();																//가우시안 소거법 적용
	double GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//스텐실(input 측정)
	double GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//스텐실(output 측정)
	int IsDuplicateNode(Node &BaseNode, CBaseElement *pElement);							//스텐실(중복 측정)
	double GetCombinedResistance(Node &CurrentNode);										//스텐실(전체 측정)
	void IPCDataConvert(list<IPCData> &Datalist, list<CBaseElement*> &Provlist);			//데이터 전송


};

