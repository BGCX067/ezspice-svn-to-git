#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CPowerSupply ��ȭ �����Դϴ�.

class CPowerSupply : public CDialog
{
	DECLARE_DYNAMIC(CPowerSupply)

public:
	CPowerSupply(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CPowerSupply();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClicked1v();
	afx_msg void OnBnClickedFloat1v();
//	afx_msg void OnBnClickedRequest();
	afx_msg void OnBnClickedON();
	afx_msg void OnBnClickedOFF();
	afx_msg void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

public:
	CEdit				m_DisplayCtrl;
	CSpinButtonCtrl		m_SpinCtrl;
	CBrush				m_BackBrush;

	CString				m_VoltageValue;
	double				m_VoltageInt;
	double				m_Unit;

	CButton				m_On;
	CButton				m_Off;

	BOOL				m_OperationFlag;
};	
