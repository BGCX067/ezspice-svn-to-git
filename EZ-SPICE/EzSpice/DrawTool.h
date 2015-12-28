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
	int						m_DrawState;					//Tool 종류 표현
	static BOOL				m_LButtonDownFlag;				//OnLbutton을 눌렀는지 검사	
	static UINT				m_DownFlag;						//OnLbutton시 속성 저장
	static CPoint			m_DownPoint;					//마우스 다운 위치
	static CPoint			m_MovePoint;					//마우스 이동 위치
	static UINT				m_InsertKind;					//삽입시 선택한 소자 종류
	
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
	CPoint Near_LineToPoint(double x1, double y1, double x2, double y2, double x,double y);		//라인 분할시 가장 가까운 직선의 위치 탐색
	void LinkConnect(CEzSpiceView *pView, CBaseElement* NewElement, CPoint &ComparePoint);		//라인 분할시 링크 연결역활
	void DivideLinkConnect();																	//라인 분할시 링크 분할역활 (m_pFirstDivideElement, m_pSecondDivideElement) 으로 고정
	void SensingConnect(CEzSpiceView *pView, CPoint &point);									//insert 모드시 라인 그리면서 Sensing
	void NonClickSensingConnect(CEzSpiceView *pView, CPoint &point);							//insert모드시 sensing

public:
	// Implementation
	virtual void OnLButtonDown(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CEzSpiceView* pView, UINT nFlags, const CPoint& point);
	virtual void OnEditProperties(CEzSpiceView* pView);
	virtual void OnTabDown(CEzSpiceView* pView);
};