#pragma once
#include "afxwin.h"


// CElementGroupInsertDlg ��ȭ �����Դϴ�.

class CElementGroupInsertDlg : public CDialog
{
	DECLARE_DYNAMIC(CElementGroupInsertDlg)

public:
	CElementGroupInsertDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CElementGroupInsertDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_GROUPINSERT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString m_strGroupName;
	CEdit m_GroupCtrl;
	virtual BOOL OnInitDialog();
};
