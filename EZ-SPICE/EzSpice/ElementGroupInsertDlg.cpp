// ElementGroupInsertDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "ElementGroupInsertDlg.h"


// CElementGroupInsertDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CElementGroupInsertDlg, CDialog)

CElementGroupInsertDlg::CElementGroupInsertDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CElementGroupInsertDlg::IDD, pParent)
	, m_strGroupName(_T(""))
{

}

CElementGroupInsertDlg::~CElementGroupInsertDlg()
{
}

void CElementGroupInsertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strGroupName);
	DDX_Control(pDX, IDC_EDIT1, m_GroupCtrl);
}


BEGIN_MESSAGE_MAP(CElementGroupInsertDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CElementGroupInsertDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CElementGroupInsertDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CElementGroupInsertDlg �޽��� ó�����Դϴ�.

void CElementGroupInsertDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	OnOK();
}

void CElementGroupInsertDlg::OnBnClickedCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	OnCancel();
}

BOOL CElementGroupInsertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFont fnt;
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 30;
	lf.lfWeight = FW_BOLD;
	::lstrcpy(lf.lfFaceName, _T("Tahoma"));
	fnt.CreateFontIndirect(&lf);
	m_GroupCtrl.SetFont(&fnt);
	fnt.Detach();

	return TRUE;  // return TRUE unless you set the focus to a control
}
