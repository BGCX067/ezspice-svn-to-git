#pragma once
#include "afxwin.h"
#include "StructInformation.h"
#include <list>

// CPlotterDlg 대화 상자입니다.


class CPlotterDlg : public CDialog
{
	DECLARE_DYNAMIC(CPlotterDlg)

public:
	CPlotterDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CPlotterDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	//해당 플로터 컨트롤
	CEdit m_AXCtrl;
	CEdit m_AYCtrl;

	CEdit m_BXCtrl;
	CEdit m_BYCtrl;

	CEdit m_CXCtrl;
	CEdit m_CYCtrl;

	//해당 플로터 변수
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

	//차이 값
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
