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
	list<CBaseElement*>			m_TempInductor;	//linearDC�� ���δ�(����) Flag -> lines ��ȯ�ϸ鼭 �ӽ� ���� ����Ʈ (���� ���� �ٽ� Flag -> inductor
	list<CapacityNode>			m_TempCapacity;
public:
	NodePretreatment(void);
	~NodePretreatment(void);

public:
	BOOL Processing(list<CBaseElement*> &CircultList, list<Node> &NodeList);	//��ü ����

private:
	void AheadProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList);		//���� ó�� (linearDC�ϰ�� ĳ�н��� �ֺ� ������ ó��)
	void RearProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList);		//���� ó�� (linearDC�ϰ�� ���δ� ���� ��� ��ġ��

	void PrintOutString(CString &str);													//���
	void PrintElementInfo(CBaseElement *pElement);										//�ش� ������ Ư�� ���� ���

	BOOL CheckCircult();																//ȸ�ΰ� �������� üũ
	void PreferenceRank();																//ȸ�� ������ ���� ������ ���� �켱 ���� ���� (�»���� �켱��)
	void DirectionDecide();																//���� ���� ����	
	int RecusiveDecide(CBaseElement *CurrentElement);									//������� ���� ����Լ�
	BOOL IsParentChild(CBaseElement *CurrentElement, CBaseElement* CompareLink);		//��Ÿ��� �θ��� �ٸ��ֵ� ����
	CBaseElement* IsOutletOK(CBaseElement* CurrentElement);								//�ⱸ Ž��
	BOOL IsAntidromic(CBaseElement* CurrentElement);									//������ üũ
	BOOL IsRemainOutlet(CBaseElement* CurrentElement);									//�������� ���������� ������ ���� ��ΰ� �ֳ�?
	int IsExceptionElement(CBaseElement* CurrentElement);								//��� ������ ���� ���ڰ� �ֳ�?
	
	CBaseElement* IsGround(list<CBaseElement*> &ElementList);							//Ground ����
	void GroundExtentSearch();															//Ground ���� Ž�� �����Լ�		
	int RecusiveExtent(CBaseElement *parent, CBaseElement *CurrentElement);				//Ground ���� Ž�� ��� �Լ�

	void NodeDistinction(list<Node> &NodeList);											//��� �Ǻ� ����
	int RecusiveNode(CBaseElement *CurrentElement, Node &NewNode, BOOL InOutFlag);		//��� �Ǻ� ��� �Լ�

	static BOOL RankingCompare(CBaseElement *pFirstElement, CBaseElement *pSecondElement);
	BOOL IsExceptionConnectPos(CBaseElement *parent, CBaseElement *child);				//���� ���� �����ϰ�� ������� �������� �����ѳ� üũ
	BOOL IsDiodeAntidromic(Node &CurrentNode, CBaseElement *pDiodeElement, BOOL DirectionFlag);	//���̿��尡 ���������� üũ�Ͽ� Leq�� ����
	BOOL IsNegativeVoltage(list<Node> &NodeList);										//�ش� ȸ�ΰ� ���������� üũ

};
