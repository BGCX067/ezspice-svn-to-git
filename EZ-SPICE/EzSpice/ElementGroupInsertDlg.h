#pragma once
#include "afxwin.h"


// CElementGroupInsertDlg 대화 상자입니다.

class CElementGroupInsertDlg : public CDialog
{
	DECLARE_DYNAMIC(CElementGroupInsertDlg)

public:
	CElementGroupInsertDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CElementGroupInsertDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_GROUPINSERT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString m_strGroupName;
	CEdit m_GroupCtrl;
	virtual BOOL OnInitDialog();
};
