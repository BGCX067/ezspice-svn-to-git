// Amplification.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Ociloscope.h"
#include "Amplification.h"


// CAmplification 대화 상자입니다.

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


// CAmplification 메시지 처리기입니다.

void CAmplification::OnBnClickedOk()
{
	OnOK();
}
