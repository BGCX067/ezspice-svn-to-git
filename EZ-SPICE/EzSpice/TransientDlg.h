#pragma once
#include "afxwin.h"


// CTransientDlg 대화 상자입니다.

class CTransientDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransientDlg)

public:
	CTransientDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CTransientDlg();

// 대화 상자 데이터입니다.
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedHelp();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void OnCancel();

public:
	CStatic m_SecCtrl;
	CStatic m_VoltCtrl;

	int m_VoltValue;
	int m_SecValue;
};
