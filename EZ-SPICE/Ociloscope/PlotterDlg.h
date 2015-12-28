#pragma once
#include "afxwin.h"
#include "StructInformation.h"
#include <list>

// CPlotterDlg ��ȭ �����Դϴ�.


class CPlotterDlg : public CDialog
{
	DECLARE_DYNAMIC(CPlotterDlg)

public:
	CPlotterDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CPlotterDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	//�ش� �÷��� ��Ʈ��
	CEdit m_AXCtrl;
	CEdit m_AYCtrl;

	CEdit m_BXCtrl;
	CEdit m_BYCtrl;

	CEdit m_CXCtrl;
	CEdit m_CYCtrl;

	//�ش� �÷��� ����
	CString m_AXValue;
	CString m_AYValue;
	double	m_AX;
	double	m_AY;

	CString m_BXValue;
	CString m_BYValue;
	double	m_BX;
	double	m_BY;

	CString m_CXValue;
	CString m_CYValue;
	double	m_CX;
	double	m_CY;

	//���� ��
	CString m_ABXValue;
	CString m_ABYValue;
	
	CString m_BCXValue;
	CString m_BCYValue;

	CString m_CAXValue;
	CString m_CAYValue;

protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();

public:
	virtual BOOL OnInitDialog();
	void SetPlotterData(list<PlotterData> &DataList);
	void Reset();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
