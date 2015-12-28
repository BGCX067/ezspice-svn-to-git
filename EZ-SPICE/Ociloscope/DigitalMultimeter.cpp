// DigitalMultimeter.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Ociloscope.h"
#include "DigitalMultimeter.h"
#include "MainFrm.h"


// CDigitalMultimeter 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDigitalMultimeter, CDialog)

CDigitalMultimeter::CDigitalMultimeter(CWnd* pParent /*=NULL*/)
	: CDialog(CDigitalMultimeter::IDD, pParent)
	, m_RadioValue(0)
{

}

CDigitalMultimeter::~CDigitalMultimeter()
{
	m_BackBrush.DeleteObject();
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->USB_HID_Sendreport(DISIT_REPOERID, 0, 0, 0);
}

void CDigitalMultimeter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DISPlAY, m_DisplayCtrl);
	DDX_Control(pDX, IDC_SPIN, m_SpinCtrl);
	DDX_Control(pDX, IDC_1Mohm, m_1M);
	DDX_Control(pDX, IDC_100kohm, m_100k);
	DDX_Control(pDX, IDC_1kohm, m_1k);
	DDX_Control(pDX, IDC_100ohmm, m_100);
	DDX_Radio(pDX, IDC_RADIO1, m_RadioValue);
	DDX_Control(pDX, IDC_RADIO1, m_RadioCtrl);
}


BEGIN_MESSAGE_MAP(CDigitalMultimeter, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_REQUEST, &CDigitalMultimeter::OnBnClickedRequest)
	ON_BN_CLICKED(IDC_RADIO1, &CDigitalMultimeter::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CDigitalMultimeter::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CDigitalMultimeter::OnBnClickedRadio3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, &CDigitalMultimeter::OnDeltaposSpin)
	ON_BN_CLICKED(IDC_1Mohm, &CDigitalMultimeter::OnBnClicked1mohm)
	ON_BN_CLICKED(IDC_100kohm, &CDigitalMultimeter::OnBnClicked100kohm)
	ON_BN_CLICKED(IDC_1kohm, &CDigitalMultimeter::OnBnClicked1kohm)
	ON_BN_CLICKED(IDC_100ohmm, &CDigitalMultimeter::OnBnClicked100ohmm)
END_MESSAGE_MAP()


// CDigitalMultimeter 메시지 처리기입니다.



void CDigitalMultimeter::OnBnClickedRequest()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();

	if (m_RequestMode != resistance) {
		pFrame->USB_HID_Sendreport(DISIT_REPOERID, (char)m_RequestMode, 0, 0);
	}
	else {
		pFrame->USB_HID_Sendreport(DISIT_REPOERID, (char)m_RequestMode, 0, ((char*)&m_DisInt)[0]);
	}
}

void CDigitalMultimeter::PostNcDestroy()
{
	((CMainFrame*)AfxGetMainWnd())->m_pMultimeterDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CDigitalMultimeter::OnCancel()
{
	DestroyWindow();
}

void CDigitalMultimeter::OnBnClickedRadio1() //전압
{
	m_RequestMode = voltage;
	m_UnitString = _T("V");
	m_DisplayCtrl.SetWindowText(_T("0V"));
	OhmVisible(FALSE);
}

void CDigitalMultimeter::OnBnClickedRadio2() //전류
{
	m_RequestMode = electric;
	m_UnitString = _T("A");
	m_DisplayCtrl.SetWindowText(_T("0mA"));
	OhmVisible(FALSE);
}

void CDigitalMultimeter::OnBnClickedRadio3() //저항
{
	m_RequestMode = resistance;
	m_UnitString = _T("Ω");
	OhmVisible(TRUE);
	OnBnClicked100ohmm();
}

void CDigitalMultimeter::OhmVisible(BOOL nFlag)
{
	if (nFlag) {
		m_1M.EnableWindow(TRUE);
		m_100k.EnableWindow(TRUE);
		m_1k.EnableWindow(TRUE);
		m_100.EnableWindow(TRUE);
	}
	else {
		m_1M.EnableWindow(FALSE);
		m_100k.EnableWindow(FALSE);
		m_1k.EnableWindow(FALSE);
		m_100.EnableWindow(FALSE);
	}
}
BOOL CDigitalMultimeter::OnInitDialog()
{
	CDialog::OnInitDialog();

	
	CRect ClientRect;
	GetClientRect(&ClientRect);

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/3*2;
	int DlgY = (yWin-ClientRect.bottom + 30)/3;
	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;

	m_BackBrush.CreateSolidBrush(BLACK_BRUSH);
	m_DisInt = 0;
	m_Unit = 1;
	m_UnitString = _T("V");

	CFont fnt;
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 40;
	lf.lfWeight = FW_NORMAL;
	::lstrcpy(lf.lfFaceName, _T("Tahoma"));
	fnt.CreateFontIndirect(&lf);
	m_DisplayCtrl.SetFont(&fnt);
	fnt.Detach();

	m_RequestMode = voltage;

	m_DisVlaue.Format(_T("%dV"), m_DisInt);
	m_DisplayCtrl.SetWindowText(m_DisVlaue);
	OhmVisible(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}

HBRUSH CDigitalMultimeter::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(nCtlColor) {
		case CTLCOLOR_STATIC:
			{
				if (pWnd->GetDlgCtrlID() == IDC_DISPlAY) {
					pDC->SetTextColor(RGB(0,255,0));
					pDC->SetBkMode(TRANSPARENT);
					hbr = (HBRUSH)m_BackBrush;
				}

			}
			break;
	}
	return hbr;
}

void CDigitalMultimeter::OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	
	if (pNMUpDown->iDelta == -1) { //up
		if (m_DisInt + m_Unit >= 10000) {
			m_DisInt = 10000;
			AfxMessageBox(_T("범위를 초과합니다."));
		}
		else {
			m_DisInt += m_Unit;
		}
	}
	else { //down
		if (m_DisInt - m_Unit < 0) {
			m_DisInt = 0;
			AfxMessageBox(_T("범위를 초과합니다."));
		}
		else {
			m_DisInt -= m_Unit;
		}
	}
	m_DisVlaue.Format(_T("%dΩ"), m_DisInt);
	m_DisplayCtrl.SetWindowText(m_DisVlaue);
	*pResult = 0;
}

void CDigitalMultimeter::OnBnClicked1mohm()
{
// 	m_DisVlaue.Format(_T("1MΩ"), m_DisInt);
// 	m_DisplayCtrl.SetWindowText(m_DisVlaue);
	m_DisInt = 4;
}

void CDigitalMultimeter::OnBnClicked100kohm()
{
// 	m_DisVlaue.Format(_T("100KΩ"), m_DisInt);
// 	m_DisplayCtrl.SetWindowText(m_DisVlaue);
	m_DisInt = 3;
}

void CDigitalMultimeter::OnBnClicked1kohm()
{
// 	m_DisVlaue.Format(_T("1KΩ"), m_DisInt);
// 	m_DisplayCtrl.SetWindowText(m_DisVlaue);
	m_DisInt = 2;
}

void CDigitalMultimeter::OnBnClicked100ohmm()
{
// 	m_DisVlaue.Format(_T("100Ω"), m_DisInt);
// 	m_DisplayCtrl.SetWindowText(m_DisVlaue);
	m_DisInt = 1;
}

void CDigitalMultimeter::SetMultimeterData(int DataType, int Data)
{
	switch (DataType) {
		case 2:	//디지털 전압
			{
				double ConvertData = Data;
				ConvertData/=100;

				m_DisVlaue.Format(_T("%.2fV"), ConvertData);
				m_DisplayCtrl.SetWindowText(m_DisVlaue);
			}
			break;

		case 3:	//디지털 전류
			{
				double ConvertData = Data;
				ConvertData/=100;

				m_DisVlaue.Format(_T("%.2fA"), ConvertData);
				m_DisplayCtrl.SetWindowText(m_DisVlaue);
			}
			break;

		case 4:	//디지털 저항
			{
				double ConvertData = Data;
				ConvertData/=10;

				m_DisVlaue.Format(_T("%.2fΩ"), ConvertData);
				m_DisplayCtrl.SetWindowText(m_DisVlaue);
			}
			break;

		case 5:	//디지털 저항 1K
			{
				double ConvertData = Data;
				ConvertData/=10;

				m_DisVlaue.Format(_T("%.2fkΩ"), ConvertData);
				m_DisplayCtrl.SetWindowText(m_DisVlaue);
			}
			break;

		case 6:	//디지털 저항 1M
			{
				double ConvertData = Data;
				ConvertData/=10;

				m_DisVlaue.Format(_T("%.2fMΩ"), ConvertData);
				m_DisplayCtrl.SetWindowText(m_DisVlaue);
			}
			break;
	}
}



