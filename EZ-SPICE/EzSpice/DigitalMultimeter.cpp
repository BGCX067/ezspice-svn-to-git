// DigitalMultimeter.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "DigitalMultimeter.h"

#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include <algorithm>
#include "StructInformation.h"


enum SelectButton
{
	dcvoltage, acvoltage, dcelectric, acelectric, Ohm, 
};

// CDigitalMultimeter 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDigitalMultimeter, CDialog)

CDigitalMultimeter::CDigitalMultimeter(CWnd* pParent /*=NULL*/)
	: CDialog(CDigitalMultimeter::IDD, pParent)
	, m_ViewValue(_T(""))
{
	
}

CDigitalMultimeter::~CDigitalMultimeter()
{
	m_BackBrush.DeleteObject();
}

void CDigitalMultimeter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MULTIMETERVIEW, m_ViewValue);
	DDX_Control(pDX, IDC_MULTIMETERVIEW, m_ViewCtrl);
	DDX_Control(pDX, IDC_MULTCOMBO, m_MultComboCtrl);
}


BEGIN_MESSAGE_MAP(CDigitalMultimeter, CDialog)
	ON_BN_CLICKED(IDC_DC_VOLTAGE, &CDigitalMultimeter::OnBnClickedDcVoltage)
	ON_BN_CLICKED(IDC_AC_VOLTAGE, &CDigitalMultimeter::OnBnClickedAcVoltage)
	ON_BN_CLICKED(IDC_DC_ELECTRIC, &CDigitalMultimeter::OnBnClickedDcElectric)
	ON_BN_CLICKED(IDC_AC_ELECTRIC, &CDigitalMultimeter::OnBnClickedAcElectric)
	ON_BN_CLICKED(IDC_OHM, &CDigitalMultimeter::OnBnClickedOhm)
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(IDC_MULTCOMBO, &CDigitalMultimeter::OnCbnSelchangeMultcombo)
END_MESSAGE_MAP()


// CDigitalMultimeter 메시지 처리기입니다.

void CDigitalMultimeter::OnBnClickedDcVoltage()
{
	m_SelectButton = dcvoltage;
	m_pView->MultimeterSimulStart();
}

void CDigitalMultimeter::OnBnClickedAcVoltage()
{
	m_SelectButton = acvoltage;
	m_pView->MultimeterSimulStart();
}

void CDigitalMultimeter::OnBnClickedDcElectric()
{
	m_SelectButton = dcelectric;
}

void CDigitalMultimeter::OnBnClickedAcElectric()
{
	m_SelectButton = dcelectric;
}

void CDigitalMultimeter::OnBnClickedOhm()
{
	m_SelectButton = Ohm;
}

BOOL CDigitalMultimeter::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pView = NULL;
	m_SelectProvNumber = -1;
	m_BackBrush.CreateSolidBrush(BLACK_BRUSH);

	CFont fnt;
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 50;
	lf.lfWeight = FW_NORMAL;
	::lstrcpy(lf.lfFaceName, _T("Tahoma"));
	fnt.CreateFontIndirect(&lf);

	m_ViewValue = _T("0.0000V");	
	m_ViewCtrl.SetFont(&fnt);
	fnt.Detach();
	UpdateData(FALSE);

	CRect ClientRect;
	GetClientRect(&ClientRect);	

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/2;
	int DlgY = (yWin-ClientRect.bottom + 30)/2;

	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;
	return TRUE;  // return TRUE unless you set the focus to a control
}

HBRUSH CDigitalMultimeter::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(nCtlColor) {
		case CTLCOLOR_STATIC:
			{
				if (pWnd->GetDlgCtrlID() == IDC_MULTIMETERVIEW) {
					pDC->SetTextColor(RGB(0,255,0));
					pDC->SetBkMode(TRANSPARENT);
					hbr = (HBRUSH)m_BackBrush;
				}
				
			}
			break;
	}
 	return hbr;
}

void CDigitalMultimeter::PostNcDestroy()
{
	if (m_pView != NULL) {
		m_pView->m_pMultimeterDlg = NULL;
		delete this;
	}
	CDialog::PostNcDestroy();
}

void CDigitalMultimeter::SetCEzSpiceViewHandle( CEzSpiceView *pView )
{
	m_pView = pView;
}

void CDigitalMultimeter::OnCancel()
{
	DestroyWindow();
}

void CDigitalMultimeter::InitializeProvList(list<IPCData> &List)
{
	m_IPCDataList.clear();
	m_IPCDataList.resize(List.size());
	copy(List.begin(), List.end(), m_IPCDataList.begin());

	m_MultComboCtrl.ResetContent();
	list<IPCData>::iterator pos = m_IPCDataList.begin();
	while (pos != m_IPCDataList.end()) {

		CString ProvName;
		ProvName.Format(_T("Prov%d"), (*pos).ProvNumber);
		m_MultComboCtrl.AddString(ProvName);
		++pos;
	}

	if (List.size() > 0) {
		m_MultComboCtrl.SetCurSel(0);
		m_SelectProvNumber = 0;
		ReloadIPCData();
	}
}

void CDigitalMultimeter::OnCbnSelchangeMultcombo()
{
	m_SelectProvNumber = m_MultComboCtrl.GetCurSel();
	ReloadIPCData();
}


void CDigitalMultimeter::ReloadIPCData()
{
	int iCount = 0;
	list<IPCData>::iterator pos = m_IPCDataList.begin();
	while (pos != m_IPCDataList.end()) {
		if (iCount == m_SelectProvNumber) {
			CString OutputData(_T("0.0000V"));

			if (m_SelectButton == acvoltage && (m_pView->m_SectorList[iCount].Circitcase == linearAC || m_pView->m_SectorList[iCount].Circitcase == nonlinearAC)) {
				//실효값 적용

				double MaxValue = 0;
				for (int iCount = 0; iCount < DivideSector; ++iCount) {
					if (MaxValue < (*pos).GrapehValue[iCount]) {
						MaxValue = (*pos).GrapehValue[iCount];
					}
				}
				MaxValue *= MaxAmplitude;
				OutputData.Format(_T("%0.4fV"), MaxValue);
			}
			else if (m_SelectButton == dcvoltage) {
				OutputData.Format(_T("%0.4fV"), (*pos).GrapehValue[0]);
			}
			m_ViewValue = OutputData;
			UpdateData(FALSE);
		}
		++iCount; ++pos;
	}
}