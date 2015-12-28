#pragma once


// CAmplification 대화 상자입니다.

class CAmplification : public CDialog
{
	DECLARE_DYNAMIC(CAmplification)

public:
	CAmplification(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CAmplification();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	double m_RateValue;
	afx_msg void OnBnClickedOk();
};
