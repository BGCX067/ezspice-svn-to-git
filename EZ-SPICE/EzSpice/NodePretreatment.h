#pragma once

#include <list>
#include "StructInformation.h"
class CBaseElement;
class CMainFrame;

class NodePretreatment
{
private:
	CMainFrame					*m_pFrame;
	list<CBaseElement*>			m_CircultList;


private:
	list<CBaseElement*>			m_TempInductor;	//linearDC시 인턱더(코일) Flag -> lines 변환하면서 임시 저장 리스트 (수행 이후 다시 Flag -> inductor
	list<CapacityNode>			m_TempCapacity;
public:
	NodePretreatment(void);
	~NodePretreatment(void);

public:
	BOOL Processing(list<CBaseElement*> &CircultList, list<Node> &NodeList);	//전체 로직

private:
	void AheadProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList);		//선행 처리 (linearDC일결우 캐패시터 주변 역방향 처리)
	void RearProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList);		//후행 처리 (linearDC일경우 인턱더 포함 노드 합치기

	void PrintOutString(CString &str);													//출력
	void PrintElementInfo(CBaseElement *pElement);										//해당 소자의 특성 정보 출력

	BOOL CheckCircult();																//회로가 정상인지 체크
	void PreferenceRank();																//회로 연결중 병렬 구간에 대한 우선 순위 지정 (좌상당이 우선권)
	void DirectionDecide();																//진행 방향 결정	
	int RecusiveDecide(CBaseElement *CurrentElement);									//진행방향 결정 재귀함수
	BOOL IsParentChild(CBaseElement *CurrentElement, CBaseElement* CompareLink);		//삼거리시 부모의 다른애들 구분
	CBaseElement* IsOutletOK(CBaseElement* CurrentElement);								//출구 탐색
	BOOL IsAntidromic(CBaseElement* CurrentElement);									//역방향 체크
	BOOL IsRemainOutlet(CBaseElement* CurrentElement);									//역방향이 존재하지만 가보지 않은 경로가 있나?
	int IsExceptionElement(CBaseElement* CurrentElement);								//경로 설정시 예외 소자가 있나?
	
	CBaseElement* IsGround(list<CBaseElement*> &ElementList);							//Ground 검출
	void GroundExtentSearch();															//Ground 범위 탐색 시작함수		
	int RecusiveExtent(CBaseElement *parent, CBaseElement *CurrentElement);				//Ground 범위 탐색 재귀 함수

	void NodeDistinction(list<Node> &NodeList);											//노드 판별 시작
	int RecusiveNode(CBaseElement *CurrentElement, Node &NewNode, BOOL InOutFlag);		//노드 판별 재귀 함수

	static BOOL RankingCompare(CBaseElement *pFirstElement, CBaseElement *pSecondElement);
	BOOL IsExceptionConnectPos(CBaseElement *parent, CBaseElement *child);				//다중 연결 소자일경우 진행방향 결정에서 연결한놈 체크
	BOOL IsDiodeAntidromic(Node &CurrentNode, CBaseElement *pDiodeElement, BOOL DirectionFlag);	//다이오드가 역방향인지 체크하여 Leq에 영향
	BOOL IsNegativeVoltage(list<Node> &NodeList);										//해당 회로가 음전압인지 체크

};
