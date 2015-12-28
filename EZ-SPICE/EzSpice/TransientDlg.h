#pragma once
#include "afxwin.h"


// CTransientDlg ��ȭ �����Դϴ�.

class CTransientDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransientDlg)

public:
	CTransientDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CTransientDlg();

// ��ȭ ���� �������Դϴ�.
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
