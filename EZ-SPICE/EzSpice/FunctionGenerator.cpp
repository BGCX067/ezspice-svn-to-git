// FunctionGenerator.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "FunctionGenerator.h"


// CFunctionGenerator 대화 상자입니다.

IMPLEMENT_DYNAMIC(CFunctionGenerator, CDialog)

CFunctionGenerator::CFunctionGenerator(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FUNC, pParent)
{

}

CFunctionGenerator::~CFunctionGenerator()
{
}

void CFunctionGenerator::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HZ, m_HzCtrl);
	DDX_Control(pDX, IDC_SPIN, m_Spin);
}


BEGIN_MESSAGE_MAP(CFunctionGenerator, CDialog)
	ON_BN_CLICKED(IDC_LEFT, &CFunctionGenerator::OnBnClickedLeft)
	ON_BN_CLICKED(IDC_EDIT, &CFunctionGenerator::OnBnClickedEdit)
	ON_BN_CLICKED(IDC_RIGHT, &CFunctionGenerator::OnBnClickedRight)
	ON_BN_CLICKED(IDC_SIGN, &CFunctionGenerator::OnBnClickedSign)
	ON_BN_CLICKED(IDC_SQUA, &CFunctionGenerator::OnBnClickedSqua)
	ON_BN_CLICKED(IDC_TRIA, &CFunctionGenerator::OnBnClickedTria)
	ON_BN_CLICKED(IDC_SEND, &CFunctionGenerator::OnBnClickedSend)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, &CFunctionGenerator::OnDeltaposSpin)
END_MESSAGE_MAP()


// CFunctionGenerator 메시지 처리기입니다.

void CFunctionGenerator::OnBnClickedLeft()
{
}

void CFunctionGenerator::OnBnClickedEdit()
{
}

void CFunctionGenerator::OnBnClickedRight()
{
}

void CFunctionGenerator::OnBnClickedSign()
{
}

void CFunctionGenerator::OnBnClickedSqua()
{
}

void CFunctionGenerator::OnBnClickedTria()
{
}

void CFunctionGenerator::OnBnClickedSend()
{
}

void CFunctionGenerator::OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	


	*pResult = 0;
}
