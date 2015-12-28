#pragma once
#include "afxwin.h"


// CElementSearchDlg ��ȭ �����Դϴ�.

class CEzSpiceView;

class CElementSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CElementSearchDlg)

public:
	CElementSearchDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CElementSearchDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ELEMENTSEARCH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
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
