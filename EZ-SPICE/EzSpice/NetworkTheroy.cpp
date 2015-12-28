// NetworkTheroy.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "NetworkTheroy.h"
#include "MainFrm.h"


// CNetworkTheroy ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CNetworkTheroy, CDialog)

CNetworkTheroy::CNetworkTheroy(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_NETWORKTHEROY, pParent)
{

}

CNetworkTheroy::~CNetworkTheroy()
{
}

void CNetworkTheroy::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONTENT, m_ContentCtrl);
	DDX_Control(pDX, IDC_THEROYPICTURE, m_PictureCtrl);
}


BEGIN_MESSAGE_MAP(CNetworkTheroy, CDialog)
	ON_LBN_SELCHANGE(IDC_CONTENT, &CNetworkTheroy::OnLbnSelchangeContent)
END_MESSAGE_MAP()


// CNetworkTheroy �޽��� ó�����Դϴ�.

void CNetworkTheroy::PostNcDestroy()
{
	((CMainFrame*)AfxGetMainWnd())->m_pNetworkTheroyDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CNetworkTheroy::OnCancel()
{
	DestroyWindow();
}

BOOL CNetworkTheroy::OnInitDialog()
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

	InitializeTheroy();
	UpdataPicture(3);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CNetworkTheroy::UpdataPicture(int picturenumber)
{
	HBITMAP hBit = NULL;
	switch (picturenumber) {
	case 1:
		hBit=LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_THEROY1));
		break;

	case 2:
		hBit=LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_THEROY2));
		break;

	case 3:
		hBit=LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_THEROY3));
		break;

	case 4:
		hBit=LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_THEROY4));
		break;

	case 5:
		hBit=LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_THEROY5));
		break;

	case 6:
		hBit=LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_THEROY6));
		break;
	}

	if (hBit) {
		m_PictureCtrl.SetBitmap(hBit);
	}
}

void CNetworkTheroy::InitializeTheroy()
{
	m_ContentCtrl.AddString(_T("[ȸ�θ��̷�]---------------------"));
	m_ContentCtrl.AddString(_T("1. DC�� AC��ȣ"));
	m_ContentCtrl.AddString(_T("2. ���Ǵ���"));
	m_ContentCtrl.AddString(_T("3. Ű����ȣ��  ��Ģ"));
	m_ContentCtrl.AddString(_T("4. �� �ؼ���"));
	m_ContentCtrl.AddString(_T("5. Thecenin / norton �ȸ��"));
	m_ContentCtrl.AddString(_T("6. ��İ� �� �ؼ���"));
	m_ContentCtrl.AddString(_T("7. �����ý���"));
	m_ContentCtrl.AddString(_T("8. �����ý���"));
	m_ContentCtrl.AddString(_T("9. ���ļ� ����"));
	m_ContentCtrl.AddString(_T("10. ����� ����"));
	m_ContentCtrl.AddString(_T("11. �ٻ��ؼ���"));
	m_ContentCtrl.AddString(_T(""));
	m_ContentCtrl.AddString(_T("[EZ-Spice�ؼ�]---------------------"));

	m_ContentCtrl.SetCurSel(3);
}
void CNetworkTheroy::OnLbnSelchangeContent()
{
	int CurrentPosition = m_ContentCtrl.GetCurSel();
	UpdataPicture(CurrentPosition);
}
