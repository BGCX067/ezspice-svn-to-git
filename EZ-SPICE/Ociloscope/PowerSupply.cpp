// PowerSupply.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Ociloscope.h"
#include "PowerSupply.h"
#include "MainFrm.h"


// CPowerSupply 대화 상자입니다.

enum Operator
{
	off, on
};

enum IncreaseVoltage
{
	nonincrease, plus_one, plus_fraction_eone, minus_one, minus_fraction_one		//0, 1, 0.1, -1, -0.1 	
};

IMPLEMENT_DYNAMIC(CPowerSupply, CDialog)

CPowerSupply::CPowerSupply(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_POWER, pParent)
{

}

CPowerSupply::~CPowerSupply()
{
	m_BackBrush.DeleteObject();

	if (m_OperationFlag) {
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->USB_HID_Sendreport(POWER_REPOERID, off, 0, 0);
	}
}

void CPowerSupply::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VOLTAGE, m_DisplayCtrl);
	DDX_Control(pDX, IDC_SPIN, m_SpinCtrl);
	DDX_Control(pDX, IDC_ON, m_On);
	DDX_Control(pDX, IDC_OFF, m_Off);
}


BEGIN_MESSAGE_MAP(CPowerSupply, CDialog)
	ON_BN_CLICKED(IDC_1V, &CPowerSupply::OnBnClicked1v)
	ON_BN_CLICKED(IDC_FLOAT1V, &CPowerSupply::OnBnClickedFloat1v)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, &CPowerSupply::OnDeltaposSpin)
	ON_WM_CTLCOLOR()

	ON_BN_CLICKED(IDC_ON, &CPowerSupply::OnBnClickedON)
	ON_BN_CLICKED(IDC_OFF, &CPowerSupply::OnBnClickedOFF)
END_MESSAGE_MAP()


// CPowerSupply 메시지 처리기입니다.

void CPowerSupply::OnBnClicked1v()
{
	m_Unit = 1;
}

void CPowerSupply::OnBnClickedFloat1v()
{
	m_Unit = 0.1;
}

void CPowerSupply::OnBnClickedON()
{
	m_On.SetWindowText(_T("ON\n(select)"));
	m_Off.SetWindowText(_T("OFF"));

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->USB_HID_Sendreport(POWER_REPOERID, on, 0, 0);
	m_OperationFlag = TRUE;
}

void CPowerSupply::OnBnClickedOFF()
{
	m_Off.SetWindowText(_T("OFF\n(Select)"));
	m_On.SetWindowText(_T("ON"));

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->USB_HID_Sendreport(POWER_REPOERID, off, 0, 0);
	m_OperationFlag = FALSE;
}

void CPowerSupply::OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	int Increase = nonincrease;
	if (pNMUpDown->iDelta == -1) { //up
		if (m_VoltageInt + m_Unit >= 10.0) {
			if (m_VoltageInt != 10.0f) {
				m_Unit == 1? Increase = plus_one : Increase = plus_fraction_eone;
			}
			m_VoltageInt = 10.0f;
			AfxMessageBox(_T("범위를 초과합니다."));
		}
		else {
			m_VoltageInt += m_Unit;
			m_Unit == 1? Increase = plus_one : Increase = plus_fraction_eone;
		}
	}
	else { //down
		if (m_VoltageInt - m_Unit < 0.1) {
			if (m_VoltageInt != 0.1f) {
				m_Unit == 1? Increase = minus_one : Increase = minus_fraction_one;
			}
			m_VoltageInt = 0.1f;
			AfxMessageBox(_T("범위를 초과합니다."));
		}
		else {
			m_VoltageInt -= m_Unit;
			m_Unit == 1? Increase = minus_one : Increase = minus_fraction_one;
		}
	}

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	int ConvertData = (int)(m_VoltageInt*10);

	int OperationData = off;
	m_OperationFlag == TRUE ? OperationData = on : OperationData = off;
	pFrame->USB_HID_Sendreport(POWER_REPOERID, OperationData, ConvertData, Increase);

	m_VoltageValue.Format(_T("%.1fV"), m_VoltageInt);
	m_DisplayCtrl.SetWindowText(m_VoltageValue);
	*pResult = 0;
}

void CPowerSupply::PostNcDestroy()
{
	((CMainFrame*)AfxGetMainWnd())->m_pPowerSupplyDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CPowerSupply::OnCancel()
{
	DestroyWindow();
}

BOOL CPowerSupply::OnInitDialog()
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
	m_VoltageInt = 0;
	m_Unit = 1;
	m_VoltageValue = _T("0V");
	m_OperationFlag = FALSE;

	CFont fnt;
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 40;
	lf.lfWeight = FW_NORMAL;
	::lstrcpy(lf.lfFaceName, _T("Tahoma"));
	fnt.CreateFontIndirect(&lf);
	m_DisplayCtrl.SetFont(&fnt);
	fnt.Detach();

	m_DisplayCtrl.SetWindowText(m_VoltageValue);

	return TRUE;  // return TRUE unless you set the focus to a control
}

HBRUSH CPowerSupply::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch(nCtlColor) {
		case CTLCOLOR_STATIC:
			{
				if (pWnd->GetDlgCtrlID() == IDC_VOLTAGE) {
					pDC->SetTextColor(RGB(0,255,0));
					pDC->SetBkMode(TRANSPARENT);
					hbr = (HBRUSH)m_BackBrush;
				}

			}
			break;
	}
	return hbr;
}


