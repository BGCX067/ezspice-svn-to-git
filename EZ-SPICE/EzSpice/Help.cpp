// Help.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "Help.h"
#include "MainFrm.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"

// CHelp ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CHelp, CDialog)

CHelp::CHelp(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_HELPLE, pParent)
{

}

CHelp::~CHelp()
{
}

void CHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXAPLE, m_TreeCtrl);
}


BEGIN_MESSAGE_MAP(CHelp, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_EXAPLE, &CHelp::OnTvnSelchangedExaple)
	ON_NOTIFY(NM_DBLCLK, IDC_EXAPLE, &CHelp::OnNMDblclkExaple)
END_MESSAGE_MAP()


// CHelp �޽��� ó�����Դϴ�.

BOOL CHelp::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect ClientRect;
	GetClientRect(&ClientRect);	

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/3*2;
	int DlgY = (yWin-ClientRect.bottom + 30)/3;

	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;

	InitializeTree();
	return TRUE;  // return TRUE unless you set the focus to a control
}


void CHelp::InitializeTree()
{
	CBitmap Bmp;
	Bmp.LoadBitmap(IDB_FILESMALL);

	ImgList.Create(16, 16, ILC_COLOR24, 6, 0);
	ImgList.Add(&Bmp, RGB(255, 0, 0));
	m_TreeCtrl.SetImageList(&ImgList, TVSIL_NORMAL);

	HTREEITEM hItem = NULL, LDCItem = NULL, LACItem = NULL, NLDCItem = NULL, NLACItem = NULL, AMPItem = NULL;
	hItem = m_TreeCtrl.InsertItem(_T("Example"), 1, 1, TVI_ROOT);
	LDCItem = m_TreeCtrl.InsertItem(_T("Linear DC"), 1, 1, hItem);
	LACItem = m_TreeCtrl.InsertItem(_T("Linear AC"), 1, 1, hItem);
	NLDCItem = m_TreeCtrl.InsertItem(_T("NonLinear DC"), 1, 1, hItem);
	NLACItem = m_TreeCtrl.InsertItem(_T("NonLinear AC"), 1, 1, hItem);
	AMPItem = m_TreeCtrl.InsertItem(_T("Op-Amp"), 1, 1, hItem);

	//LinearDC
	{
		m_TreeCtrl.InsertItem(_T("���� ��ø ȸ��"), 0, 0, LDCItem);
		m_TreeCtrl.InsertItem(_T("�δ��� ȸ��"), 0, 0, LDCItem);
		m_TreeCtrl.InsertItem(_T("Ŀ�н��� ȸ��"), 0, 0, LDCItem);
		m_TreeCtrl.InsertItem(_T("���� DC ��ø ȸ��"), 0, 0, LDCItem);
	}

	//Linear AC
	{
		m_TreeCtrl.InsertItem(_T("RC ȸ��"), 0, 0, LACItem);
		m_TreeCtrl.InsertItem(_T("RC ������ ȸ��"), 0, 0, LACItem);
		m_TreeCtrl.InsertItem(_T("RL ȸ��"), 0, 0, LACItem);
		m_TreeCtrl.InsertItem(_T("RR ȸ��"), 0, 0, LACItem);
		m_TreeCtrl.InsertItem(_T("RLC ȸ��"), 0, 0, LACItem);
	}

	//NonLinear DC
	{
		m_TreeCtrl.InsertItem(_T("������ ���̿��� ���� ȸ��"), 0, 0, NLDCItem);
		m_TreeCtrl.InsertItem(_T("������ ���̿��� ���� ȸ��"), 0, 0, NLDCItem);
		m_TreeCtrl.InsertItem(_T("��,������ ���̿��� ���� ȸ��"), 0, 0, NLDCItem);
		m_TreeCtrl.InsertItem(_T("Ʈ�������� ���� ȸ��"), 0, 0, NLDCItem);
	}

	//NonLinear AC
	{
		m_TreeCtrl.InsertItem(_T("������ ���̿��� ���� ȸ��"), 0, 0, NLACItem);
		m_TreeCtrl.InsertItem(_T("������ ���̿��� ���� ȸ��"), 0, 0, NLACItem);
		m_TreeCtrl.InsertItem(_T("��,������ ���̿��� ���� ȸ��"), 0, 0, NLACItem);
		m_TreeCtrl.InsertItem(_T("���̿��� �긮�� ���� ȸ��"), 0, 0, NLACItem);
		m_TreeCtrl.InsertItem(_T("���̿��� �긮��+ ���� ȸ��"), 0, 0, NLACItem);
		m_TreeCtrl.InsertItem(_T("Ʈ�������� ���� ȸ��"), 0, 0, NLACItem);
	}

	//Op-Amp
	{
		m_TreeCtrl.InsertItem(_T("���������� ���� ȸ��"), 0, 0, AMPItem);
		m_TreeCtrl.InsertItem(_T("���������� ���� ȸ��"), 0, 0, AMPItem);
		m_TreeCtrl.InsertItem(_T("���������� + ��������̿���"), 0, 0, AMPItem);
		m_TreeCtrl.InsertItem(_T("���������� + ��������̿���"), 0, 0, AMPItem);
		m_TreeCtrl.InsertItem(_T("����������� ���� ȸ��"), 0, 0, AMPItem);
		m_TreeCtrl.InsertItem(_T("����������� ���� ȸ��"), 0, 0, AMPItem);


	}
	m_TreeCtrl.EnsureVisible(AMPItem);
}

void CHelp::PostNcDestroy()
{
	((CMainFrame*)AfxGetMainWnd())->m_pFormuladlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CHelp::OnCancel()
{
	DestroyWindow();
}

void CHelp::SetEzView(CEzSpiceView *pView)
{
	m_pView = pView;
}

void CHelp::OnTvnSelchangedExaple(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	CString strItem = m_TreeCtrl.GetItemText(pNMTreeView->itemNew.hItem);

	if (strItem != _T("Example")) {
		CString AheadAdree = _T("C:\\Program Files\\Ez-Spice\\Example\\");
		AheadAdree.Append(strItem);	
		m_pView->GetDocument()->LoadDorgFile(AheadAdree);
	}
	*pResult = 0;
}

void CHelp::OnNMDblclkExaple(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;
}
