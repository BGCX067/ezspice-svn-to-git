// EditPropertyDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EditPropertyDlg.h"
#include "resource.h"
#include "InPlaceEdit.h"
#include "StructInformation.h"

enum Wavemodel
{
	sign, square, triangular
};


// CEditPropertyDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CEditPropertyDlg, CDialog)

CEditPropertyDlg::CEditPropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PROPERTY, pParent)
	, m_strElementName(_T(""))
{
	m_SineWave = NULL;
	m_SquareWave = NULL;
	m_TriangularWave = NULL;
	m_Flag = Fnormal;
	m_PictureCtrl = NULL;
}

CEditPropertyDlg::~CEditPropertyDlg()
{
	if (m_PictureCtrl != NULL) {
		delete m_PictureCtrl;
	}

	if (m_SineWave != NULL) {
		delete m_SineWave;
	}

	if (m_SquareWave != NULL) {
		delete m_SquareWave;
	}

	if (m_TriangularWave != NULL) {
		delete m_TriangularWave;
	}
}

void CEditPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_NAME, m_ElementName);
	DDX_Text(pDX, IDC_STATIC_NAME, m_strElementName);
	DDX_Control(pDX, IDC_STATICNAMETEXT, m_NameStatic);
	DDX_Control(pDX, IDC_PEOPERTY, m_PropertyListCtrl);
}


BEGIN_MESSAGE_MAP(CEditPropertyDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CEditPropertyDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CEditPropertyDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_HLEP, &CEditPropertyDlg::OnBnClickedHlep)
END_MESSAGE_MAP()


// CEditPropertyDlg 메시지 처리기입니다.

void CEditPropertyDlg::OnBnClickedOk()
{
	OnOK();
}

void CEditPropertyDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnCancel();
}

void CEditPropertyDlg::OnBnClickedHlep()
{
	AfxMessageBox(_T("도움말을 제공하지 않습니다."));
}

void CEditPropertyDlg::InitializeProperty(vector<Property> &PropertyList)
{
	this->m_PropertyList.clear();
	this->m_PropertyList.resize(PropertyList.size());
	copy(PropertyList.begin(), PropertyList.end(), this->m_PropertyList.begin());
}


void CEditPropertyDlg::SetElementFlag(int Flag)
{
	m_Flag = Flag;
}

BOOL CEditPropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFont fnt;
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 30;
	lf.lfWeight = FW_BOLD;
	::lstrcpy(lf.lfFaceName, _T("Tahoma"));
	fnt.CreateFontIndirect(&lf);
	m_ElementName.SetFont(&fnt);
	m_NameStatic.SetFont(&fnt);
 	fnt.Detach();
	UpdateData(FALSE);

	TCHAR NameBuffer[MAX_STRING] = {0,};
	this->GetWindowText(NameBuffer, MAX_STRING);
	CString DlgName;
	DlgName.Format(_T("%s - "), NameBuffer);
	DlgName += m_GroupName;
	this->SetWindowText(DlgName);

	//listCtrl Initialize
	{
		COLORREF crBkColor = ::GetSysColor(COLOR_3DFACE);
		m_PropertyListCtrl.SetBkColor(crBkColor);
		m_PropertyListCtrl.SetTextBkColor(crBkColor);

		m_PropertyListCtrl.AddColumn(_T("Name"), 150, LVCFMT_LEFT);
		m_PropertyListCtrl.AddColumn(_T("Value"), 80, LVCFMT_LEFT);
		m_PropertyListCtrl.AddColumn(_T("Unit"), 68, LVCFMT_LEFT);

		m_PropertyListCtrl.SetExtendedStyle(LVS_EDITLABELS | LVS_EX_GRIDLINES);
		
		int iCount = 0;
		vector<Property>::iterator pos = m_PropertyList.begin();
		while (pos != m_PropertyList.end()) {

			if ((*pos).PropertyName == CString(_T("WaveModel"))) {

				CRect ListCtrlRect;
				m_PropertyListCtrl.GetWindowRect(&ListCtrlRect);
				ScreenToClient(&ListCtrlRect);
				ListCtrlRect.bottom -= 50;
				m_PropertyListCtrl.MoveWindow(ListCtrlRect);

				CStatic *PropertyStatic = (CStatic*)GetDlgItem(IDC_PROPERTYSTATIC);

				CRect PropertyStaticRect;
				PropertyStatic->GetWindowRect(&PropertyStaticRect);
				ScreenToClient(&PropertyStaticRect);
				PropertyStaticRect.bottom -= 50;
				PropertyStatic->MoveWindow(PropertyStaticRect);

				m_SineWave = new CBitmapButton;
				m_SineWave->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW |
					BS_PUSHBUTTON, CRect(14, 205, 0, 0), 
					this, 25000); 

				(*pos).PropertyValue == 0 ? m_SineWave->LoadBitmaps(IDB_SIGNDOWN) : m_SineWave->LoadBitmaps(IDB_SIGN);
				m_SineWave->SizeToContent();

				m_SquareWave = new CBitmapButton;
				m_SquareWave->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW |
					BS_PUSHBUTTON, CRect(121, 205, 0, 0), 
					this, 25001); 

				(*pos).PropertyValue == 1 ? m_SquareWave->LoadBitmaps(IDB_SQUAREDOWN) : m_SquareWave->LoadBitmaps(IDB_SQUARE);
				m_SquareWave->SizeToContent();

				m_TriangularWave = new CBitmapButton;
				m_TriangularWave->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW |
					BS_PUSHBUTTON, CRect(228, 205, 0, 0), 
					this, 25002); 

				(*pos).PropertyValue == 2 ? m_TriangularWave->LoadBitmaps(IDB_TRIANGULRDOWN) : m_TriangularWave->LoadBitmaps(IDB_TRIANGULR);
				m_TriangularWave->SizeToContent();
			}
			else {
				int InsertIndex = m_PropertyListCtrl.InsertItem(iCount, (*pos).PropertyName);
				CString PropertyValue;

				switch (m_Flag) {
					case Fnormal:
						PropertyValue.Format(_T("%.1f"),(*pos).PropertyValue);
						break;

					case Fdiode:
						{
							PropertyValue.Format(_T("%.6f"),(*pos).PropertyValue);
						}
						break;

					default:
						break;
				}

				m_PropertyListCtrl.SetItemText(InsertIndex, 1, PropertyValue);
				m_PropertyListCtrl.SetItemText(InsertIndex, 2, (*pos).Unit);
			}
			iCount++;
			++pos;
		}

		m_PictureCtrl = new CBitmapButton;
		m_PictureCtrl->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW |
			BS_PUSHBUTTON, CRect(352, 70, 0, 0), 
			this, 25003); 
		
		BOOL nFlag = FALSE;
		switch (m_ElementFlag) {
			case dcpower: m_PictureCtrl->LoadBitmaps(IDB_DC); break;
			case acpower: m_PictureCtrl->LoadBitmaps(IDB_AC); break;
			case resistance: m_PictureCtrl->LoadBitmaps(IDB_RESIS);	break;
			case inductor: m_PictureCtrl->LoadBitmaps(IDB_INDUCTOR); break;
			case capacity: m_PictureCtrl->LoadBitmaps(IDB_CAPA); break;
			case diode: m_PictureCtrl->LoadBitmaps(IDB_DIODE); break;
			case transistor: m_PictureCtrl->LoadBitmaps(IDB_TR); break;
			case InvertingAmplifier: m_PictureCtrl->LoadBitmaps(IDB_AMP); break;
			case NonInvertingAmplifier: m_PictureCtrl->LoadBitmaps(IDB_NAMP); break;

			default: nFlag = TRUE; break;
		}
		if (!nFlag) {
			m_PictureCtrl->SizeToContent();
		}
		
	}

	return TRUE; 
}

BOOL CEditPropertyDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	int UID = (int)wParam;
	switch (UID) {
	case IDC_IPEDIT:	//CInPlaceEdit 값 변동시 호출
		{
			LV_DISPINFO *dispinfo = (LV_DISPINFO*)lParam;	
			m_PropertyList[dispinfo->item.iItem].PropertyValue = _tstof(dispinfo->item.pszText);
		}
		break;
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

BOOL CEditPropertyDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int ChoiceValue = -1;
	if (wParam == 25000) {
		ChoiceValue = sign;

		m_SineWave->LoadBitmaps(IDB_SIGNDOWN);
		m_SineWave->SizeToContent();

		m_SquareWave->LoadBitmaps(IDB_SQUARE);
		m_SquareWave->SizeToContent();

		m_TriangularWave->LoadBitmaps(IDB_TRIANGULR);
		m_TriangularWave->SizeToContent();

		m_SquareWave->RedrawWindow();
		m_SineWave->RedrawWindow();
		m_TriangularWave->RedrawWindow();
	}
	else if (wParam == 25001) {
		ChoiceValue = square;

		m_SquareWave->LoadBitmaps(IDB_SQUAREDOWN);
		m_SquareWave->SizeToContent();

		m_SineWave->LoadBitmaps(IDB_SIGN);
		m_SineWave->SizeToContent();

		m_TriangularWave->LoadBitmaps(IDB_TRIANGULR);
		m_TriangularWave->SizeToContent();

		m_SquareWave->RedrawWindow();
		m_SineWave->RedrawWindow();
		m_TriangularWave->RedrawWindow();
	}
	else if (wParam == 25002) {
		ChoiceValue = triangular;

		m_TriangularWave->LoadBitmaps(IDB_TRIANGULRDOWN);
		m_TriangularWave->SizeToContent();

		m_SquareWave->LoadBitmaps(IDB_SQUARE);
		m_SquareWave->SizeToContent();

		m_SineWave->LoadBitmaps(IDB_SIGN);
		m_SineWave->SizeToContent();

		m_SquareWave->RedrawWindow();
		m_SineWave->RedrawWindow();
		m_TriangularWave->RedrawWindow();
	}

	if (ChoiceValue != -1) {
		vector<Property>::iterator pos = m_PropertyList.begin();
		while (pos != m_PropertyList.end()) {
			if ((*pos).PropertyName == CString(_T("WaveModel"))) {
				(*pos).PropertyValue = ChoiceValue;
			}
			++pos;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}
