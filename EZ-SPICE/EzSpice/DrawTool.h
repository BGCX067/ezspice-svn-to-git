#pragma once


enum DrawStates {
	selection, Connectline, insert
};

class CEzSpiceView;
class CBaseElement;
class CLineInfomaton;

class CDrawTool
{
public:
	CDrawTool(int State);
	virtual ~CDrawTool(void);

public:
	int						m_DrawState;					//Tool ���� ǥ��
	static BOOL				m_LButtonDownFlag;				//OnLbutton�� �������� �˻�	
	static UINT				m_DownFlag;						//OnLbutton�� �Ӽ� ����
	static CPoint			m_DownPoint;					//���콺 �ٿ� ��ġ
	static CPoint			m_MovePoint;					//���콺 �̵� ��ġ
	static UINT				m_InsertKind;					//���Խ� ������ ���� ����
	
	static CBaseElement		*m_pNewElement;
	static CBaseElement		*m_pNewFirstLine;
	static CBaseElement		*m_pNewSecondLine;

	static CDrawTool* FindTool(CEzSpiceView* pView, int State);
	static void GridPostion(CPoint &position);
	static void ProvProcessing(CEzSpiceView *pView);

public:
	virtual void OnLButtonDown(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblck(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnEditProperties(CEzSpiceView* pView, UINT nFlags, const CPoint& point);	
	virtual void OnTabDown(CEzSpiceView* pView);
	
};



class CSelectTool : public CDrawTool
{
public:
	CSelectTool();
	virtual ~CSelectTool();

	int		m_MoveCount;

public:
	void DivideLinkConnect(CBaseElement *pFirstDivideElement, CBaseElement *pSecondDivideElement);
	
public:
	// Implementation
	virtual void OnLButtonDown(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnEditProperties(CEzSpiceView* pView);

	
};


class CElementTool : public CDrawTool
{
public:
	CElementTool();
	virtual ~CElementTool();

	// Implementation
	virtual void OnLButtonDown(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnEditProperties(CEzSpiceView* pView);
};

class CLineTool : public CDrawTool
{
public:
	CLineTool();
	virtual ~CLineTool();

	int						m_DrawLineType;
	static BOOL				m_MergerFlag;
	static CBaseElement*	m_pLastElement;
	static CBaseElement*	m_pFirstDivideElement;
	static CBaseElement*	m_pSecondDivideElement;

public:
	CPoint Near_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);		//���� ���ҽ� ���� ����� ������ ��ġ Ž��
	void LinkConnect(CEzSpiceView *pView, CBaseElement* NewElement, CPoint &ComparePoint);		//���� ���ҽ� ��ũ ���ῪȰ
	void DivideLinkConnect();																	//���� ���ҽ� ��ũ ���ҿ�Ȱ (m_pFirstDivideElement, m_pSecondDivideElement) ���� ����
	void SensingConnect(CEzSpiceView *pView, CPoint &point);									//insert ���� ���� �׸��鼭 Sensing
	void NonClickSensingConnect(CEzSpiceView *pView, CPoint &point);							//insert���� sensing

public:
	// Implementation
	virtual void OnLButtonDown(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnEditProperties(CEzSpiceView* pView);
	virtual void OnTabDown(CEzSpiceView* pView);
};