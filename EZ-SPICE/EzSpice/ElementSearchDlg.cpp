// ElementSearchDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "ElementSearchDlg.h"

#include "EzSpice.h"
#include "MainFrm.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include "DrawTool.h"


// enum ElementKind {
// 	lines, dcpower, acpower, resistance, ground, inductor, capacity, transistor, prov, diode, leq, geq,
// };

enum FilterType
{
	start, anywhere, end
};
// CElementSearchDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CElementSearchDlg, CDialog)

CElementSearchDlg::CElementSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CElementSearchDlg::IDD, pParent)
	, m_SearchString(_T(""))
	, m_FilterValue(0)
	, m_SelectElementNumber(-1)
{
	
}

CElementSearchDlg::~CElementSearchDlg()
{
}

void CElementSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEARCHEDIT, m_SearchEdit);
	DDX_Text(pDX, IDC_SEARCHEDIT, m_SearchString);
	DDX_Radio(pDX, IDC_FILTER1, m_FilterValue);
	DDX_Control(pDX, IDC_SEARCHLIST, m_SearchList);
	DDX_Control(pDX, IDC_TOTALCOUNT, m_TotalCtrl);
}


BEGIN_MESSAGE_MAP(CElementSearchDlg, CDialog)
	ON_BN_CLICKED(IDC_SEARCH, &CElementSearchDlg::OnBnClickedSearch)
	ON_BN_CLICKED(IDC_SEARCH_INSERT, &CElementSearchDlg::OnBnClickedSearchInsert)
	ON_BN_CLICKED(IDC_SEARCH_CLOSE, &CElementSearchDlg::OnBnClickedSearchClose)
//	ON_WM_CLOSE()
ON_LBN_SELCHANGE(IDC_SEARCHLIST, &CElementSearchDlg::OnLbnSelchangeSearchlist)
END_MESSAGE_MAP()


// CElementSearchDlg 메시지 처리기입니다.

void CElementSearchDlg::SetCEzSpiceViewHandle(CEzSpiceView *pView)	//삭제하기 위한 함수
{
	this->m_pView = pView;

}

void CElementSearchDlg::OnBnClickedSearch()
{
	UpdateData(TRUE);

	//Default Initialize

	for (int iCount = m_SearchList.GetCount() -1; iCount >= 0 ; --iCount) {
		m_SearchList.DeleteString(iCount);
	}
	
	m_SearchString = m_SearchString.MakeUpper();

	int TotalCount = 0;
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	list<ElementGroup>::iterator pos = pFrame->m_DefaultGroupList.begin();
	while (pos != pFrame->m_DefaultGroupList.end()) {
		 
		list<ElementInfo>::iterator elementpos = (*pos).ElementList.begin();
		while (elementpos != (*pos).ElementList.end()) {
			
			
			BOOL nFlag = TRUE;
			if (m_FilterValue == start) {
				for (int iCount = 0; iCount < m_SearchString.GetLength(); iCount++) {
					if (m_SearchString.GetAt(iCount) != (*elementpos).ElementName.GetAt(iCount)) {
						nFlag = FALSE;
					}
				}		
			}
			else if (m_FilterValue == anywhere) {
				if (_tcsstr((*elementpos).ElementName, m_SearchString) == NULL) {
					nFlag = FALSE;
				}
			}
			else if (m_FilterValue == end) {
				int jCount = 1;
				int DefaultLength =  (*elementpos).ElementName.GetLength();
				int SearchLength = m_SearchString.GetLength();
				for (int iCount = DefaultLength - 1; iCount >= DefaultLength - SearchLength; --iCount, ++jCount) {
					if (m_SearchString.GetAt(m_SearchString.GetLength() - jCount) != (*elementpos).ElementName.GetAt(iCount)) {
						nFlag = FALSE;
					}
				}		
			}

			if (nFlag) {
				TotalCount++;
				CString VisualString = (*elementpos).ElementName;
				VisualString += CString(_T(" | "));
				VisualString += (*elementpos).ElementGroupName;
				int Index = m_SearchList.AddString(VisualString);
				m_SearchList.SetItemData(Index, (*elementpos).ElementNumber);
			}
			++elementpos;
		}
		++pos;
	}

	if (TotalCount > 0) {
		CString TotalString;
		TotalString.Format(_T("1/%d"), TotalCount);
		m_TotalCtrl.SetWindowText(TotalString);
	}
}

void CElementSearchDlg::OnBnClickedSearchInsert()
{
	if (m_SelectElementNumber != -1) {
		m_pView->InsertElement(m_SelectElementNumber);
	}
	DestroyWindow();
}

void CElementSearchDlg::OnBnClickedSearchClose()
{
	//그냥 종료
	DestroyWindow();
}

void CElementSearchDlg::OnCancel()
{
	OnBnClickedSearchClose();
}

void CElementSearchDlg::PostNcDestroy()
{
	m_pView->m_pSearchDlg = NULL;	
	delete this;
	CDialog::PostNcDestroy();
}



BOOL CElementSearchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//Move Center Window
	CRect ClientRect;
	GetClientRect(&ClientRect);	

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/2;
	int DlgY = (yWin-ClientRect.bottom + 30)/2;

	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;
	return TRUE; 
}

void CElementSearchDlg::OnLbnSelchangeSearchlist()
{
	int Index = m_SearchList.GetCurSel();
	m_SelectElementNumber = (int)m_SearchList.GetItemData(Index);
}
