// ElementGroupInsertDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "ElementGroupInsertDlg.h"


// CElementGroupInsertDlg 대화 상자입니다.

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


// CElementGroupInsertDlg 메시지 처리기입니다.

void CElementGroupInsertDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	OnOK();
}

void CElementGroupInsertDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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
