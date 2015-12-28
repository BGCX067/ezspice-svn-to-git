#pragma once


// CElementView

class CElementListCtrl : public CListCtrl
{
	// 생성입니다.
public:
	CElementListCtrl();
	virtual ~CElementListCtrl();

protected:
	DECLARE_MESSAGE_MAP()

public:
	int m_GroupNumber;	
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	void SetGroupNumber(int Number);
};


class CElementView : public CDockablePane
{
	DECLARE_DYNAMIC(CElementView)

public:
	CElementView();
	virtual ~CElementView();

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

public:
	void ReloadElementData();
	void RemoveAllPageList();
private:
	CWnd					m_wndChild;
	CMFCTabCtrl				m_wndTabs;
	CFont					m_Font;
	CImageList				m_ElementImageList;
	CElementListCtrl		m_Page;
	list<CElementListCtrl*>	m_PageList;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


