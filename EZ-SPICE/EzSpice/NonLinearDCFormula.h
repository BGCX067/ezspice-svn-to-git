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

	double				m_Voltage; //전압원 값
	int					m_MaxCount;

public:
	BOOL Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);
	void NonlinearStencilConvert(int NodeSize);															//비선형 스텐실화
	double GetElementProperty(CBaseElement *pElement, CString &PropertyString);				//해당 소자의 특성값 반환
	BOOL GaussianElimination(int NodeSize);													//가우시안 소거법 적용
	double GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//스텐실(input 측정)
	double GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//스텐실(output 측정)
	int IsDuplicateNode(Node &BaseNode, CBaseElement *pElement);							//스텐실(중복 측정)
	double GetCombinedResistance(Node &CurrentNode);										//스텐실(전체 측정)
	void IPCDataConvert(list<IPCData> &Datalist, list<CBaseElement*> &Provlist);			//데이터 전송

	BOOL IsDiodeInclusion(Node &CurrentNode);												//내부에 다이오드 변환 회로가 포함인가?
	double GetCombineLeq(Node &CurrentNode);												//모든 Leq값 획득
	BOOL IsCompareAndInsert();																//랩손법 조건문
	CBaseElement* SearchElement(Node &CurrentNode, int Flag);								//노드에서 객체 찾기
	double GetOutputPlusCombinedResistance(Node &CurrentNode, double *StencilBuffer);
	int IsGetNextNode(int CurrentNodeNumber, CBaseElement* CompareElement);
	double RoundXL(double x, int digits);													//double 원하는 소숫점 반올림
};

