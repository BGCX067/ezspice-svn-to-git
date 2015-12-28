#pragma once


// CSubListCtrl

 
class CSubListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CSubListCtrl)

public:
	CSubListCtrl();
	virtual ~CSubListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	CEdit* EditSubLabel(int nItem, int nCol);
	int HitTestEx(CPoint &point, int *col) const;
	
public:
	int AddColumn(LPCTSTR sColHeading, int nWidth, int nFormat);
	
//	afx_msg void OnPaint();
};


