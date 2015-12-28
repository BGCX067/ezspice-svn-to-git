#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CFunctionGenerator 대화 상자입니다.

class CFunctionGenerator : public CDialog
{
	DECLARE_DYNAMIC(CFunctionGenerator)

public:
	CFunctionGenerator(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CFunctionGenerator();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClicked10kHz();
	afx_msg void OnBnClicked1kHz();
	afx_msg void OnBnClicked100Hz();
	afx_msg void OnBnClicked10Hz();
	afx_msg void OnBnClickedSign();
	afx_msg void OnBnClickedSqua();
	afx_msg void OnBnClickedTria();
	afx_msg void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	

protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();

public:
	CEdit			m_HzCtrl;
	CSpinButtonCtrl m_SpinCtrl;
	int				m_HzInt;
	CString			m_Hzvalue;
	CBrush			m_BackBrush;

	int				m_Unit;
	int				m_WaveModel;

	BOOL			m_EnableFlag;

	CButton			m_Signbutton;
	CButton			m_Squarbutton;
	CButton			m_Tributton;

public:
	virtual BOOL OnInitDialog();
	
};
