#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CFunctionGenerator ��ȭ �����Դϴ�.

class CFunctionGenerator : public CDialog
{
	DECLARE_DYNAMIC(CFunctionGenerator)

public:
	CFunctionGenerator(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CFunctionGenerator();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_HzCtrl;
	CSpinButtonCtrl m_Spin;
	afx_msg void OnBnClickedLeft();
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedRight();
	afx_msg void OnBnClickedSign();
	afx_msg void OnBnClickedSqua();
	afx_msg void OnBnClickedTria();
	afx_msg void OnBnClickedSend();
	afx_msg void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult);
};
