// TransientDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "TransientDlg.h"
#include "MainFrm.h"


// CTransientDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CTransientDlg, CDialog)

CTransientDlg::CTransientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TRANSIENT, pParent)
	, m_VoltValue(5)
	, m_SecValue(20)
{

}

CTransientDlg::~CTransientDlg()
{
}

void CTransientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEC, m_SecCtrl);
	DDX_Control(pDX, IDC_VOLT, m_VoltCtrl);
	DDX_Text(pDX, IDC_EDIT2, m_VoltValue);
	DDV_MinMaxInt(pDX, m_VoltValue, 0, 10);
	DDX_Text(pDX, IDC_EDIT1, m_SecValue);
	DDV_MinMaxInt(pDX, m_SecValue, 0, 10000);
}


BEGIN_MESSAGE_MAP(CTransientDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTransientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CTransientDlg::OnBnClickedCancel)
	ON_BN_CLICKED(ID_TR_HELP, &CTransientDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


// CTransientDlg 메시지 처리기입니다.

void CTransientDlg::OnBnClickedOk()
{
	OnOK();
	((CMainFrame*)AfxGetMainWnd())->m_SecMax = m_SecValue;
	((CMainFrame*)AfxGetMainWnd())->m_VoltMax = m_VoltValue;
	DestroyWindow();
}

void CTransientDlg::OnBnClickedCancel()
{
	DestroyWindow();
}

void CTransientDlg::OnBnClickedHelp()
{
	AfxMessageBox(_T("아직 도움말이 제공되지 않습니다."));
}

void CTransientDlg::PostNcDestroy()
{
	((CMainFrame*)AfxGetMainWnd())->m_pTransientDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CTransientDlg::OnCancel()
{
	OnBnClickedCancel();
}

BOOL CTransientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//Default Initialize
	CRect ClientRect;
	GetClientRect(&ClientRect);	

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/2;
	int DlgY = (yWin-ClientRect.bottom + 30)/2;

	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;

	
	return TRUE;  // return TRUE unless you set the focus to a control
}

