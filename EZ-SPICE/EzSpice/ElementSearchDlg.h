#pragma once
#include "afxwin.h"


// CElementSearchDlg 대화 상자입니다.

class CEzSpiceView;

class CElementSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CElementSearchDlg)

public:
	CElementSearchDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CElementSearchDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ELEMENTSEARCH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void PostNcDestroy();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSearch();
	afx_msg void OnBnClickedSearchInsert();
	afx_msg void OnBnClickedSearchClose();

private:
	CEzSpiceView	*m_pView;

public:
	CEdit			m_SearchEdit;
	CString			m_SearchString;
	CListBox		m_SearchList;
	int				m_FilterValue;
	int				m_SelectElementNumber;

	void SetCEzSpiceViewHandle(CEzSpiceView *pView); 
	virtual BOOL OnInitDialog();
	
	CStatic m_TotalCtrl;
	afx_msg void OnLbnSelchangeSearchlist();
};
