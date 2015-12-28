// PlotterDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Ociloscope.h"
#include "PlotterDlg.h"
#include "MainFrm.h"
#include "OciloscopeDoc.h"
#include "OciloscopeView.h"


// CPlotterDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CPlotterDlg, CDialog)

CPlotterDlg::CPlotterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlotterDlg::IDD, pParent)
	, m_ABXValue(_T(""))
	, m_ABYValue(_T(""))
	, m_AXValue(_T(""))
	, m_AYValue(_T(""))
	, m_BXValue(_T(""))
	, m_BYValue(_T(""))
	, m_CXValue(_T(""))
	, m_CYValue(_T(""))
	, m_BCXValue(_T(""))
	, m_BCYValue(_T(""))
	, m_CAXValue(_T(""))
	, m_CAYValue(_T(""))
	, m_AX(0)
	, m_AY(0)
	, m_BX(0)
	, m_BY(0)
	, m_CX(0)
	, m_CY(0)

{

}

CPlotterDlg::~CPlotterDlg()
{
}

void CPlotterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLOTTER_A_X, m_AXCtrl);
	DDX_Control(pDX, IDC_PLOTTER_A_Y, m_AYCtrl);
	DDX_Control(pDX, IDC_PLOTTER_B_X, m_BXCtrl);
	DDX_Control(pDX, IDC_PLOTTER_B_Y, m_BYCtrl);
	DDX_Text(pDX, IDC_PLOTTER_AB_X, m_ABXValue);
	DDX_Text(pDX, IDC_PLOTTER_AB_Y, m_ABYValue);
	DDX_Text(pDX, IDC_PLOTTER_A_X, m_AXValue);
	DDX_Text(pDX, IDC_PLOTTER_A_Y, m_AYValue);
	DDX_Text(pDX, IDC_PLOTTER_B_X, m_BXValue);
	DDX_Text(pDX, IDC_PLOTTER_B_Y, m_BYValue);
	DDX_Control(pDX, IDC_PLOTTER_CX, m_CXCtrl);
	DDX_Text(pDX, IDC_PLOTTER_CX, m_CXValue);
	DDX_Control(pDX, IDC_PLOTTER_CY, m_CYCtrl);
	DDX_Text(pDX, IDC_PLOTTER_CY, m_CYValue);
	DDX_Text(pDX, IDC_PLOTTER_BC_X, m_BCXValue);
	DDX_Text(pDX, IDC_PLOTTER_BC_Y, m_BCYValue);
	DDX_Text(pDX, IDC_PLOTTER_CA_X, m_CAXValue);
	DDX_Text(pDX, IDC_PLOTTER_CA_Y, m_CAYValue);
}


BEGIN_MESSAGE_MAP(CPlotterDlg, CDialog)
END_MESSAGE_MAP()


// CPlotterDlg 메시지 처리기입니다.

void CPlotterDlg::PostNcDestroy()
{
	COciloscopeView *pView = (COciloscopeView*)((CMainFrame*)AfxGetMainWnd())->GetActiveView();
	pView->m_PlotterDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CPlotterDlg::OnCancel()
{
	DestroyWindow();
}

BOOL CPlotterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//Default Initialize
	CRect ClientRect;
	GetClientRect(&ClientRect);	

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/7*6;
	int DlgY = (yWin-ClientRect.bottom + 30)/5;

	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CPlotterDlg::SetPlotterData( list<PlotterData> &DataList )
{
	BOOL AFlag = FALSE, BFlag = FALSE, CFlag = FALSE;
	list<PlotterData>::iterator pos = DataList.begin();
	while (pos != DataList.end()) {

		if ((*pos).PlotterName == CString(_T("A"))) {
			m_AX = (*pos).XValue;
			m_AY = (*pos).YValue;
			m_AXValue.Format(_T("%.4lfm"), (*pos).XValue);
			m_AYValue.Format(_T("%.4lfv"), (*pos).YValue);
			AFlag = TRUE;
		}
		else if ((*pos).PlotterName == CString(_T("B"))) {
			m_BX = (*pos).XValue;
			m_BY = (*pos).YValue;
			m_BXValue.Format(_T("%.4lfm"), (*pos).XValue);
			m_BYValue.Format(_T("%.4lfv"), (*pos).YValue);
			BFlag = TRUE;
		}
		else if ((*pos).PlotterName == CString(_T("C"))) {
			m_CX = (*pos).XValue;
			m_CY = (*pos).YValue;
			m_CXValue.Format(_T("%.4lfm"), (*pos).XValue);
			m_CYValue.Format(_T("%.4lfv"), (*pos).YValue);
			CFlag = TRUE;
		}
		++pos;
	}

	if (AFlag && BFlag) {
		m_ABXValue.Format(_T("%.4fm"), m_AX - m_BX);
		m_ABYValue.Format(_T("%.4fv"), m_AY - m_BY);
	}
	else {
		m_ABXValue = CString(_T("0m"));
		m_ABYValue = CString(_T("0v"));
	}

	if (BFlag && CFlag) {
		m_BCXValue.Format(_T("%.4fm"), m_BX - m_CX);
		m_BCYValue.Format(_T("%.4fv"), m_BY - m_CY);
	}
	else {
		m_BCXValue = CString(_T("0m"));
		m_BCYValue = CString(_T("0v"));
	}

	if (CFlag && AFlag) {
		m_CAXValue.Format(_T("%.4fm"), m_CX - m_AX);
		m_CAYValue.Format(_T("%.4fv"), m_CY - m_AY);
	}
	else {
		m_CAXValue = CString(_T("0m"));
		m_CAYValue = CString(_T("0v"));
	}

	UpdateData(FALSE);
}

void CPlotterDlg::Reset()
{
	
	m_AXValue = CString(_T(""));
	m_AYValue = CString(_T(""));
	m_BXValue = CString(_T(""));
	m_BYValue = CString(_T(""));
	m_CXValue = CString(_T(""));
	m_CYValue = CString(_T(""));
	m_ABXValue = CString(_T(""));
	m_ABYValue = CString(_T(""));
	m_BCXValue = CString(_T(""));
	m_BCYValue = CString(_T(""));
	m_CAXValue = CString(_T(""));
	m_CAYValue = CString(_T(""));

	UpdateData(FALSE);
}

BOOL CPlotterDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->wParam == VK_RETURN) {
		UpdateData(TRUE);	
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
