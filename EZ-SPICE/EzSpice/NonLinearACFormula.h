#pragma once
#include "StructInformation.h"
#include <complex>

class NonLinearACFormula
{
public:
	NonLinearACFormula(void);
	~NonLinearACFormula(void);

private:
	list<RecoveryConnectElment>	m_CircuitRecoveryList;	//회로 연결정보 복사
	list<CBaseElement*>			m_ReconstructionList;	//DC로 재구성 리스트


private:
	list<Node>			m_NodeList;
	list<CBaseElement*> m_ProvList;
	list<CBaseElement*> m_TempList;

	list<Node>			m_TempNodeList;		//잠시 복사용
	list<CBaseElement*>	m_TempProvList;		//잠시 복사용
	list<IPCData>		m_TempDataList;		//잠시 복사용
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


	void LinearStencilConvert(double fluidVoltage);											//선형 스텐실화  (메트릭스 변환)
	double GetElementProperty(CBaseElement *pElement, CString &PropertyString);				//해당 소자의 특성값 반환
	BOOL GaussianElimination();																//가우시안 소거법 적용
	double GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//스텐실(input 측정)
	double GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer);			//스텐실(output 측정)
	int IsDuplicateNode(Node &BaseNode, CBaseElement *pElement);							//스텐실(중복 측정)
	double GetCombinedResistance(Node &CurrentNode);										//스텐실(전체 측정)
	void IPCDataConvert(list<IPCData> &Datalist, int iCount);//데이터 전송
	void RecoveryNode(list<Node> &CurrentList);												//초기화

	void Initialize(list<IPCData> &Datalist, list<CBaseElement*> &Provlist);				//데이터 초기화
	void CircultBackUpList();																//리스트 빽업
	void RestitutionList();																	//리스트 복구
	void RearProcessing(list<IPCData> &Datalist);											//후처리

	void Thevenintheorem();																	//테브난 정리 적용 모든 회로를 직렬로 변경
	void SeriesMerger(BOOL &OperationFlag);													//직렬구간 통합
	void RarallelMerger(BOOL &OperationFlag);												//병렬구간 통합 (조건 - 최 하위 레벨)
	void DuplicateMerger(BOOL &OperationFlag);												//출,입 노드가 같은 구간 통합
	BOOL IsProvArea(Node &CurrentNode);														//현재의 노드가 Prov노드인지 판별
	complex<double> GetElementInpedance(CBaseElement *pElement);									//소자의 임피던스 추출
	void GetFamilyNode(Node &CurrentNode, CBaseElement *CompareElement, Node **ResultNode);	//CurrentNode를 제외하고 CompareElement가 속한 노드 반환
	int DeleteElementInNode(Node &CurrentNode, CBaseElement *pElement);						//해당노드에서 소자 제거
	BOOL IsAnotherNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode);		//pElement가 현재 노드를 제외한 다른 노드에 포함이 되어있느지?
	void GetDuplicateNode(Node &CurrentNode, list<CBaseElement*> &MergerList);				//중복 노드 제거
	void Phasemovement(list<IPCData> &Datalist);											//위상 이동

	void SectionCompositonImpedance(list<IPCData> &Datalist);								//각 구간 합성 인피던스 구하는 총 로직
	void CompostionProvPostion(list<IPCData> &Datalist, BOOL nFlag = FALSE);				//프로브 위치 합성 인피던스 구함(테브닌 정리 이후)	
	double GetCapacitance();																//R-C회로의 캐패시턴스 획득
	void RCAheadeProcessing(list<CBaseElement*> &Provlist);

	//R-C
	double VoltageDropping(); //전압강하

	//Op-amp
	BOOL OPAmpProcessing(list<IPCData> &VoltageList, list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist);
	
};