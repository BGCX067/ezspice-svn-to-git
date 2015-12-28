#pragma once
#include "afxwin.h"
#include "StructInformation.h"

// CDigitalMultimeter 대화 상자입니다.
class CEzSpiceView;

class CDigitalMultimeter : public CDialog
{
	DECLARE_DYNAMIC(CDigitalMultimeter)

public:
	CDigitalMultimeter(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDigitalMultimeter();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MULTMITER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDcVoltage();
	afx_msg void OnBnClickedAcVoltage();
	afx_msg void OnBnClickedDcElectric();
	afx_msg void OnBnClickedAcElectric();
	afx_msg void OnBnClickedOhm();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeMultcombo();

protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();

public:
	CStatic			m_ViewCtrl;
	CComboBox		m_MultComboCtrl;
	CBrush			m_BackBrush;
	CString			m_ViewValue;
	CEzSpiceView	*m_pView;
	int				m_SelectProvNumber;

	int				m_SelectButton;
	list<IPCData>	m_IPCDataList;

public:
	void SetCEzSpiceViewHandle(CEzSpiceView *pView);
	void InitializeProvList(list<IPCData> &List);
	void ReloadIPCData();
};
