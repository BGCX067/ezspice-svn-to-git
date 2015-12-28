// Amplification.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Ociloscope.h"
#include "Amplification.h"


// CAmplification ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CAmplification, CDialog)

CAmplification::CAmplification(CWnd* pParent /*=NULL*/)
	: CDialog(CAmplification::IDD, pParent)
	, m_RateValue(0)
{

}

CAmplification::~CAmplification()
{
}

void CAmplification::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RATE, m_RateValue);
}


BEGIN_MESSAGE_MAP(CAmplification, CDialog)
	ON_BN_CLICKED(IDOK, &CAmplification::OnBnClickedOk)
END_MESSAGE_MAP()


// CAmplification �޽��� ó�����Դϴ�.

void CAmplification::OnBnClickedOk()
{
	OnOK();
}
