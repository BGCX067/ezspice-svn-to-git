#pragma once


// CAmplification ��ȭ �����Դϴ�.

class CAmplification : public CDialog
{
	DECLARE_DYNAMIC(CAmplification)

public:
	CAmplification(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CAmplification();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	double m_RateValue;
	afx_msg void OnBnClickedOk();
};
