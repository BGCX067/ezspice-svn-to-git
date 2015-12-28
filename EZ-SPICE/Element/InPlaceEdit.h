#pragma once


// CInPlaceEdit

class CInPlaceEdit : public CEdit
{
	DECLARE_DYNAMIC(CInPlaceEdit)

public:
	CInPlaceEdit(int iItem, int iSubItem, CString sInitText);
	virtual ~CInPlaceEdit();

protected:
	DECLARE_MESSAGE_MAP()

private:
	int                     m_iItem;
	int                     m_iSubItem;
	CString                 m_sInitText;
	BOOL					m_bESC;        // ESC 키가 눌렸는지 나타내기 위해

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


