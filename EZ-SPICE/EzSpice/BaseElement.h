#pragma once

#include "StructInformation.h"

// enum ElementKind {
// 	lines, dcpower, acpower, resistance, ground, inductor, capacity, transistor, prov, diode, leq, geq,
// };

enum ConnectDirection {
	LineToConnect, MoveToConnect, inputposition, outputposition
};


class CEzSpiceView;
class CBaseElement
{
public:
	CBaseElement(int Flag);
	virtual ~CBaseElement(void);

	static int					m_DrawState;
public:	
	CString						m_GroupName;						//���� �׷�
	CString						m_Name;								//���� �̸� (������ �޶���)
	UINT						m_Flag;								//���� ���� �ѹ�
	UINT						m_ElementNumber;					//���� ����Ʈ �ѹ� (���� ��ü ������ ����)

	CPoint						m_Position;							//���� ��ġ
	CRect						m_PositionArea;						//���� ����

	std::list<CPoint>			m_ConnectPoint;						//�����ǿ����� ������ ��ġ
	int							m_ConnectPointArea;					//�����ǿ����� ������ ����

	CPoint						m_LineTo;							//������ ��쿡�� ���
	CPoint						m_MoveTo;							//������ ��쿡�� ���
	int							m_Axis;								//������ �� 
	int							m_LineMoveDirection;				//���� �̵��� �̵��Ǵ� �� (LineTo or MoveTo is one)
	int							m_ConnnectCount;					//���ο� ����� ����

	int							m_CheckFlag;						//������� ������ ����
	BOOL						m_GroundFlag;						//Ground �������� üũ
	int							m_RotationFlag;						//ȸ�� Flag;
	BOOL						m_ProvActivityFlag;					//���α׿� ������ ���� ����� üũ
	BOOL						m_UpdataFlag;						//���� ���չ� ����� Updata üũ
	BOOL						m_AntidromicFlag;					//��ü ȸ���� ���� ������ ǥ��
	
	double						m_Unit;								//����

public:	
	std::list<CBaseElement*>	m_ConnectElement;					//��� ȸ�� �ӽ� ����
	std::list<CBaseElement*>	m_InputElement;						
	std::list<CBaseElement*>	m_OutputElement;

public:
	BOOL ObjectInPoint(const CPoint& point);						//���޵� point�� �ش� ��ü ���ο� �ִ��� ����
	void DeleteElement(CBaseElement *pElement);						//�� ������������ ���� ����
	CPoint ConnectRectDirection(const CPoint& point);				//���޵� point�� ������ ���� ������ ���ԉ����� �ش� ���῵�� ����, ������ 0,0 ����
	CPoint ComparePosition(CPoint &pos);							//���޵� point�� ������ ���ϰ�� �ش� ��ġ ����
	double Distance_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);
	int	GetPositonDirection(CPoint point);							//�ش���ġ�� �ش� ��ü(������)�� LineToConnect, MoveToConnect ���� ����
	int ConnectPositionDirection(const CPoint& point);				//������ �Էºκ����� ��ºκ����� �Ǻ�
	BOOL IsDuplicateConnectPoint(CPoint &point);					//���� ��������� �̹� ������ ����Ǿ� �ֳ�? (�ߺ� ����)

public:
	virtual void Draw( Gdiplus::Graphics* pDC, BOOLCollection Collection, CPoint Modifypoint = CPoint(0,0));
	virtual void OnEditProperties(list<ElementData> *List = NULL, CPoint point = CPoint(0,0));	
	virtual CBaseElement* Clone();
	virtual void GetProperties(list<ElementData> &List, CPoint point = CPoint(0,0)); 
	virtual void RotationElement();	
};


class CompareBuffer
{
private:
	int		m_ElementNumber;
	int		m_Flag;

public:
	CompareBuffer(int ElementNumber, int Flag) {
		m_ElementNumber = ElementNumber;
		m_Flag = Flag;
	}

	void SetElementNumber(int ElementNumber, int Flag) {
		m_ElementNumber = ElementNumber;
		m_Flag = Flag;
	}

	bool operator()(class CBaseElement *rs) const {
		return (m_ElementNumber == rs->m_ElementNumber && m_Flag == rs->m_Flag);
	}
};
