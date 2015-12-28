#pragma once
#include "afxwin.h"
#include "afxcmn.h"


enum RequestMode
{
	voltage = 1, electric, resistance
};

class CDigitalMultimeter : public CDialog
{
	DECLARE_DYNAMIC(CDigitalMultimeter)

public:
	CDigitalMultimeter(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDigitalMultimeter();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MULTMETER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClicked1mohm();
	afx_msg void OnBnClicked100kohm();
	afx_msg void OnBnClicked1kohm();
	afx_msg void OnBnClicked100ohmm();
	afx_msg void OnBnClickedRequest();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();

	CButton			m_1M;
	CButton			m_100k;
	CButton			m_1k;
	CButton			m_100;

	int				m_RadioValue;
	CButton			m_RadioCtrl;
	int				m_RequestMode;
	CBrush			m_BackBrush;
	CStatic			m_DisplayCtrl;
	CSpinButtonCtrl m_SpinCtrl;

	int				m_Unit;
	CString			m_UnitString;
	CString			m_DisVlaue;
	int				m_DisInt;

protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();
	void OhmVisible(BOOL nFlag);
public:

	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult);
	void SetMultimeterData(int DataType, int Data);
};
