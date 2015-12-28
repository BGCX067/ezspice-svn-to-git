// FunctionGenerator.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Ociloscope.h"
#include "FunctionGenerator.h"
#include "MainFrm.h"
#include "StructInformation.h"


IMPLEMENT_DYNAMIC(CFunctionGenerator, CDialog)

CFunctionGenerator::CFunctionGenerator(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FUNC, pParent)
{

}

CFunctionGenerator::~CFunctionGenerator()
{
	m_BackBrush.DeleteObject();
}

void CFunctionGenerator::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HZ, m_HzCtrl);
	DDX_Control(pDX, IDC_SPIN, m_SpinCtrl);
	DDX_Control(pDX, IDC_SIGN, m_Signbutton);
	DDX_Control(pDX, IDC_SQUA, m_Squarbutton);
	DDX_Control(pDX, IDC_TRIA, m_Tributton);
}

BEGIN_MESSAGE_MAP(CFunctionGenerator, CDialog)
	ON_BN_CLICKED(IDC_10Khertz, &CFunctionGenerator::OnBnClicked10kHz)
	ON_BN_CLICKED(IDC_1Khertz, &CFunctionGenerator::OnBnClicked1kHz)
	ON_BN_CLICKED(IDC_100hertz, &CFunctionGenerator::OnBnClicked100Hz)
	ON_BN_CLICKED(IDC_10hertz, &CFunctionGenerator::OnBnClicked10Hz)
	ON_BN_CLICKED(IDC_EDIT, &CFunctionGenerator::OnBnClickedEdit)
	ON_BN_CLICKED(IDC_SEND, &CFunctionGenerator::OnBnClickedSend)
	ON_BN_CLICKED(IDC_SIGN, &CFunctionGenerator::OnBnClickedSign)
	ON_BN_CLICKED(IDC_SQUA, &CFunctionGenerator::OnBnClickedSqua)
	ON_BN_CLICKED(IDC_TRIA, &CFunctionGenerator::OnBnClickedTria)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, &CFunctionGenerator::OnDeltaposSpin)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CFunctionGenerator 메시지 처리기입니다.


void CFunctionGenerator::OnBnClickedEdit()
{
	m_EnableFlag = !m_EnableFlag;

	if (m_EnableFlag) {
		GetDlgItem(IDC_EDIT)->SetWindowText(_T("Fix"));
		m_Hzvalue.Format(_T("%d"), m_HzInt);
		m_HzCtrl.SetWindowText(m_Hzvalue);
	}
	else {
		GetDlgItem(IDC_EDIT)->SetWindowText(_T("Edit"));
		CString CurrentString;
		m_HzCtrl.GetWindowText(CurrentString);		
		m_HzInt = _ttoi(CurrentString);

		m_HzInt = (m_HzInt/10*10);

		CurrentString.Format(_T("%dHz"), m_HzInt);
		m_HzCtrl.SetWindowText(CurrentString);
	}
	m_HzCtrl.EnableWindow(m_EnableFlag);
}


void CFunctionGenerator::OnBnClickedSend()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->USB_HID_Sendreport(FUN_REPORTID, ((char*)&m_WaveModel)[0], ((char*)&m_HzInt)[1], ((char*)&m_HzInt)[0]);
}

void CFunctionGenerator::OnBnClicked10kHz()
{
	m_Unit = 10000;
}

void CFunctionGenerator::OnBnClicked1kHz()
{
	m_Unit = 1000;
}

void CFunctionGenerator::OnBnClicked100Hz()
{
	m_Unit = 100;
}

void CFunctionGenerator::OnBnClicked10Hz()
{
	m_Unit = 10;
}


void CFunctionGenerator::OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	if (pNMUpDown->iDelta == -1) { //up
		if (m_HzInt + m_Unit >= 10000) {
			m_HzInt = 10000;
			AfxMessageBox(_T("범위를 초과합니다."));
		}
		else {
			m_HzInt += m_Unit;
		}
	}
	else { //down
		if (m_HzInt - m_Unit < 0) {
			m_HzInt = 0;
			AfxMessageBox(_T("범위를 초과합니다."));
		}
		else {
			m_HzInt -= m_Unit;
		}
	}
	m_Hzvalue.Format(_T("%dHz"), m_HzInt);
	m_HzCtrl.SetWindowText(m_Hzvalue);
	*pResult = 0;
}

void CFunctionGenerator::PostNcDestroy()
{
	((CMainFrame*)AfxGetMainWnd())->m_pFunctionDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CFunctionGenerator::OnCancel()
{
	DestroyWindow();
}

BOOL CFunctionGenerator::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect ClientRect;
	GetClientRect(&ClientRect);

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/3*2;
	int DlgY = (yWin-ClientRect.bottom + 30)/3;
	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;

	m_EnableFlag = FALSE;
	m_HzCtrl.EnableWindow(m_EnableFlag);
	m_BackBrush.CreateSolidBrush(BLACK_BRUSH);

	CFont fnt;
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 40;
	lf.lfWeight = FW_NORMAL;
	::lstrcpy(lf.lfFaceName, _T("Tahoma"));
	fnt.CreateFontIndirect(&lf);
	m_HzCtrl.SetFont(&fnt);
	fnt.Detach();
	UpdateData(FALSE);

	m_HzInt = 0;
	m_Unit = 10;
	m_WaveModel = sign;
	m_Hzvalue = _T("00Hz");	
	m_HzCtrl.SetWindowText(m_Hzvalue);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

HBRUSH CFunctionGenerator::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(nCtlColor) {
		case CTLCOLOR_STATIC:
			{
				if (pWnd->GetDlgCtrlID() == IDC_HZ) {
					pDC->SetTextColor(RGB(0,255,0));
					pDC->SetBkMode(TRANSPARENT);
					hbr = (HBRUSH)m_BackBrush;
				}

			}
			break;
	}
	return hbr;
}

void CFunctionGenerator::OnBnClickedSign()
{
	m_WaveModel = sign;
	m_Signbutton.SetWindowText(_T("Sign\n(Select)"));
	m_Squarbutton.SetWindowText(_T("Squa"));
	m_Tributton.SetWindowText(_T("Tria"));
}

void CFunctionGenerator::OnBnClickedSqua()
{
	m_WaveModel = square;
	m_Squarbutton.SetWindowText(_T("Squa\n(Select)"));
	m_Signbutton.SetWindowText(_T("Sign"));
	m_Tributton.SetWindowText(_T("Tria"));
}

void CFunctionGenerator::OnBnClickedTria()
{
	m_WaveModel = triangular;
	m_Tributton.SetWindowText(_T("Tria\n(Select)"));
	m_Squarbutton.SetWindowText(_T("Squa"));
	m_Signbutton.SetWindowText(_T("Sign"));
}


