// ElementEditDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "ElementEditDlg.h"
#include "ElementGroupInsertDlg.h"
#include "MainFrm.h"
#include <algorithm>

// CElementEditDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CElementEditDlg, CDialog)

CElementEditDlg::CElementEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ELEMENTEDITDLG, pParent)
{

}

CElementEditDlg::~CElementEditDlg()
{

}

void CElementEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USERGROUP, m_UserGroupCombo);
	DDX_Control(pDX, IDC_ELEMENTGROUP, m_DefaultCombo);
	DDX_Control(pDX, IDC_ELEMENTLIST, m_DefaultListCtrl);
	DDX_Control(pDX, IDC_GROUPLISTDISPLAY, m_UserGroupListCtrl);
//	DDX_Control(pDX, IDC_ELEMENTINFO, m_DefaultElementString);
}


BEGIN_MESSAGE_MAP(CElementEditDlg, CDialog)
	ON_BN_CLICKED(IDC_INSERT, &CElementEditDlg::OnBnClickedInsert)
	ON_BN_CLICKED(IDC_DELETE, &CElementEditDlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_USER_OK, &CElementEditDlg::OnBnClickedUserOk)
	ON_BN_CLICKED(IDC_USER_CANCEL, &CElementEditDlg::OnBnClickedUserCancel)
	ON_BN_CLICKED(IDC_USER_HLEP, &CElementEditDlg::OnBnClickedUserHlep)
	ON_CBN_SELCHANGE(IDC_USERGROUP, &CElementEditDlg::OnCbnSelchangeUsergroup)
	ON_CBN_SELCHANGE(IDC_ELEMENTGROUP, &CElementEditDlg::OnCbnSelchangeElementgroup)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_ELEMENTLIST, &CElementEditDlg::OnLvnBegindragElementlist)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_GROUPLISTDISPLAY, &CElementEditDlg::OnLvnBegindragGrouplistdisplay)
	ON_NOTIFY(NM_CLICK, IDC_ELEMENTLIST, &CElementEditDlg::OnNMClickElementlist)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CElementEditDlg 메시지 처리기입니다.


void CElementEditDlg::OnBnClickedUserOk()
{
	UpdateElementList();
	DestroyWindow();
}

void CElementEditDlg::OnBnClickedUserCancel()
{
	DestroyWindow();
}

void CElementEditDlg::OnBnClickedUserHlep()
{
	AfxMessageBox(_T("현재 도움말이 제공되지 않습니다."));
}


void CElementEditDlg::OnCancel()
{
	OnBnClickedUserCancel();
}

void CElementEditDlg::UpdateElementList()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->m_UserGroupList.clear();
	pFrame->m_UserGroupList.resize(m_UserGroup.size());
	copy(m_UserGroup.begin(), m_UserGroup.end(), pFrame->m_UserGroupList.begin());
	pFrame->UpdateElementView();
}

void CElementEditDlg::PostNcDestroy()
{
	((CMainFrame*)AfxGetMainWnd())->m_pElementEditDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

BOOL CElementEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//CopyElementData;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	m_UserGroup.clear();
	m_UserGroup.resize(pFrame->m_UserGroupList.size());
	copy(pFrame->m_UserGroupList.begin(), pFrame->m_UserGroupList.end(), m_UserGroup.begin());

	m_DefaultGroup.clear();
	m_DefaultGroup.resize(pFrame->m_DefaultGroupList.size());
	copy(pFrame->m_DefaultGroupList.begin(), pFrame->m_DefaultGroupList.end(), m_DefaultGroup.begin());
	
	m_pImageListDrag = NULL;
	m_nIndexLeftSel = -1;
	m_DeleteDragFlag = FALSE;

	//Drag&Drap
	CBitmap Bitmap;
	Bitmap.LoadBitmap(IDB_ELEMENT);
	
	m_ImageList.Create(32, 32, ILC_COLOR32 | ILC_MASK, 8, 0);
	m_ImageList.Add((&Bitmap), RGB(0,0,0));
	m_DefaultListCtrl.SetImageList(&m_ImageList, LVSIL_NORMAL);
	m_UserGroupListCtrl.SetImageList(&m_ImageList, LVSIL_NORMAL);

	DWORD dwExStyle = m_DefaultListCtrl.GetExtendedStyle();
	m_DefaultListCtrl.SetExtendedStyle(dwExStyle | LVS_EX_BORDERSELECT);
	dwExStyle = m_UserGroupListCtrl.GetExtendedStyle();
	m_UserGroupListCtrl.SetExtendedStyle(dwExStyle | LVS_EX_BORDERSELECT);

	//Default Initialize
	CRect ClientRect;
	GetClientRect(&ClientRect);	

	int xWin = GetSystemMetrics(SM_CXSCREEN);
	int yWin = GetSystemMetrics(SM_CYSCREEN);

	int DlgX = (xWin-ClientRect.right)/2;
	int DlgY = (yWin-ClientRect.bottom + 30)/2;

	MoveWindow(DlgX, DlgY, ClientRect.right, ClientRect.bottom + 30, TRUE) ;
	InitializeData();
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CElementEditDlg::InitializeData()
{
	m_DefaultIndex = m_UserGroupIndex = 0;

	//UserGroup
	int iCount = 0;
	list<ElementGroup>::iterator userpos = m_UserGroup.begin();
	while (userpos != m_UserGroup.end()) {
		m_UserGroupCombo.AddString((*userpos).GroupName);
		++userpos;
	}
	if (m_UserGroup.size() > 0) m_UserGroupCombo.SetCurSel(0);
	DrawUserElement();

	//Default
	iCount = 0;
	list<ElementGroup>::iterator defaultpos = m_DefaultGroup.begin();
	while (defaultpos != m_DefaultGroup.end()) {
		m_DefaultCombo.AddString((*defaultpos).GroupName);
		++defaultpos;
	}
	m_DefaultCombo.SetCurSel(0);
	DrawDefaultElement();
}

void CElementEditDlg::DrawDefaultElement()
{
	m_DefaultListCtrl.DeleteAllItems();
	int iCount = 0;
	if (m_DefaultGroup.size() > 0) {
		
		ElementGroup SearchGroup;
 		m_DefaultCombo.GetLBText(m_DefaultIndex, SearchGroup.GroupName);
		list<ElementGroup>::iterator defaultpos = find_if(m_DefaultGroup.begin(), m_DefaultGroup.end(), SearchGroup);
		
		if (defaultpos != m_DefaultGroup.end()) {
			list<ElementInfo>::iterator Lowpos = (*defaultpos).ElementList.begin();
			while (Lowpos != (*defaultpos).ElementList.end()) {
				m_DefaultListCtrl.InsertItem((*Lowpos).ElementImageNumber, (*Lowpos).ElementName, (*Lowpos).ElementImageNumber);
				++Lowpos;
			}
		}
	}
}

void CElementEditDlg::DrawUserElement()
{
	m_UserGroupListCtrl.DeleteAllItems();
	int iCount = 0;
	if (m_UserGroup.size() > 0) {

		ElementGroup SearchGroup;
		m_UserGroupCombo.GetLBText(m_UserGroupIndex, SearchGroup.GroupName);
		list<ElementGroup>::iterator userpos = find_if(m_UserGroup.begin(), m_UserGroup.end(), SearchGroup);

		if (userpos != m_UserGroup.end()) {
			list<ElementInfo>::iterator Lowpos = (*userpos).ElementList.begin();
			while (Lowpos != (*userpos).ElementList.end()) {
				m_UserGroupListCtrl.InsertItem((*Lowpos).ElementImageNumber, (*Lowpos).ElementName, (*Lowpos).ElementImageNumber);
				++Lowpos;
			}
		}
	}
}

void CElementEditDlg::OnBnClickedInsert()
{
	CElementGroupInsertDlg dlg;
	if (dlg.DoModal() == IDOK && dlg.m_strGroupName.GetAllocLength() > 0) {
		ElementGroup NewGroup;
		NewGroup.GroupName = dlg.m_strGroupName;

		m_UserGroup.push_back(NewGroup);
		m_UserGroupIndex = m_UserGroupCombo.AddString(NewGroup.GroupName);
		m_UserGroupCombo.SetCurSel(m_UserGroupIndex);
		DrawUserElement();
	}
}

void CElementEditDlg::OnBnClickedDelete()
{
	int DeleteIndex = m_UserGroupCombo.GetCurSel();

	ElementGroup SearchGroup;
	m_UserGroupCombo.GetLBText(m_UserGroupIndex, SearchGroup.GroupName);
	list<ElementGroup>::iterator userpos = find_if(m_UserGroup.begin(), m_UserGroup.end(), SearchGroup);
	
	if (userpos != m_UserGroup.end()) {
		m_UserGroup.erase(userpos);
	}
	m_UserGroupCombo.DeleteString(DeleteIndex);

	if (m_UserGroup.size() > 0) {
		m_UserGroupCombo.SetCurSel(0);
	}
	m_UserGroupIndex = 0;
	DrawUserElement();
}


void CElementEditDlg::OnCbnSelchangeUsergroup()
{
	m_UserGroupIndex = m_UserGroupCombo.GetCurSel();
	DrawUserElement();
}

void CElementEditDlg::OnCbnSelchangeElementgroup()
{
	m_DefaultIndex = m_DefaultCombo.GetCurSel();
	DrawDefaultElement();
}

void CElementEditDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CString strTemp = _T("");
	if (m_pImageListDrag != NULL) {
		ClientToScreen(&point);

		m_pImageListDrag->DragLeave(NULL);
		m_pImageListDrag->EndDrag();
		ReleaseCapture();

		if (m_DeleteDragFlag) {		//오른쪽 리스트에서 아이템 삭제
			CWnd *pWnd = CWnd::WindowFromPoint(point);					//오른쪽 리스트에 아이템 추가
			if (pWnd == &m_DefaultListCtrl && m_nIndexLeftSel >= 0) {
				LVITEM lvItem;
				TCHAR szBuffer[256];
				::ZeroMemory(&lvItem, sizeof(lvItem));
				::ZeroMemory(szBuffer, sizeof(256));

				lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
				lvItem.iItem = m_nIndexLeftSel;
				lvItem.pszText = szBuffer;
				lvItem.cchTextMax = 256;
				m_UserGroupListCtrl.GetItem(&lvItem);

				ElementGroup SearchGroup;
				m_UserGroupCombo.GetLBText(m_UserGroupIndex, SearchGroup.GroupName);
				list<ElementGroup>::iterator Userpos = find_if(m_UserGroup.begin(), m_UserGroup.end(), SearchGroup);

				if (Userpos != m_UserGroup.end()) {

					ElementInfo SearchElement;
					SearchElement.ElementName = CString(lvItem.pszText);
					list<ElementInfo>::iterator Elementpos = find_if((*Userpos).ElementList.begin(), (*Userpos).ElementList.end(), SearchElement);

					if (Elementpos != (*Userpos).ElementList.end()) {
						(*Userpos).ElementList.erase(Elementpos);
					}
				}
				DrawUserElement();
			}
			m_DeleteDragFlag = FALSE;
		}
		else if (m_UserGroup.size() > 0) {
			CWnd *pWnd = CWnd::WindowFromPoint(point);					//오른쪽 리스트에 아이템 추가
			if (pWnd == &m_UserGroupListCtrl && m_nIndexLeftSel >= 0) {

				LVITEM lvItem;
				TCHAR szBuffer[256];
				::ZeroMemory(&lvItem, sizeof(lvItem));
				::ZeroMemory(szBuffer, sizeof(256));

				lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
				lvItem.iItem = m_nIndexLeftSel;
				lvItem.pszText = szBuffer;
				lvItem.cchTextMax = 256;
				m_DefaultListCtrl.GetItem(&lvItem);
				
				//list에 값 복사
				ElementGroup SearchGroup;
				m_DefaultCombo.GetLBText(m_DefaultIndex, SearchGroup.GroupName);
				list<ElementGroup>::iterator defaultpos = find_if(m_DefaultGroup.begin(), m_DefaultGroup.end(), SearchGroup);

				if (defaultpos != m_DefaultGroup.end()) {
					ElementInfo SearchElement;
					SearchElement.ElementName = CString(lvItem.pszText);
					list<ElementInfo>::iterator Elementpos = find_if((*defaultpos).ElementList.begin(), (*defaultpos).ElementList.end(), SearchElement);

					if (Elementpos != (*defaultpos).ElementList.end()) {
						ElementInfo NewInfo;
						NewInfo.ElementImageNumber = (*Elementpos).ElementImageNumber;
						NewInfo.ElementIntroduceString = (*Elementpos).ElementIntroduceString;
						NewInfo.ElementName = (*Elementpos).ElementName;
						NewInfo.ElementNumber = (*Elementpos).ElementNumber;

						ElementGroup SearchGroup;
						m_UserGroupCombo.GetLBText(m_UserGroupIndex, SearchGroup.GroupName);
						list<ElementGroup>::iterator Userpos = find_if(m_UserGroup.begin(), m_UserGroup.end(), SearchGroup);

						if (Userpos != m_UserGroup.end()) {
							(*Userpos).ElementList.push_back(NewInfo);
						}
					}
				}
				m_UserGroupListCtrl.InsertItem(0, lvItem.pszText, lvItem.iImage);
			}
		}
		else {
			AfxMessageBox(_T("그룹이 설정되지 않았습니다."));
		}
	}

	delete m_pImageListDrag;
	m_pImageListDrag = NULL;
	m_nIndexLeftSel = -1;

	CDialog::OnLButtonUp(nFlags, point);
}

void CElementEditDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_pImageListDrag != NULL) {
		ClientToScreen(&point);

		CWnd *pWnd = CWnd::WindowFromPoint(point);
		if (pWnd != NULL) {
			if (this == pWnd || IsChild(pWnd)) {
				m_pImageListDrag->DragEnter(NULL, point);
				m_pImageListDrag->DragMove(point);
			}
			else {
				m_pImageListDrag->DragLeave(NULL);
			}
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CElementEditDlg::OnLvnBegindragElementlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	CPoint ptDrag, ptAction;
	m_nIndexLeftSel = pNMLV->iItem;
	m_pImageListDrag = m_DefaultListCtrl.CreateDragImage(pNMLV->iItem, &ptDrag);
	m_pImageListDrag->SetBkColor(RGB(0,0,0));
	ptAction = pNMLV->ptAction;

	SetCapture();
	m_pImageListDrag->BeginDrag(0, ptAction - ptDrag);
	m_DefaultListCtrl.ClientToScreen(&ptAction);
	m_pImageListDrag->DragEnter(NULL, ptAction);
	*pResult = 0;
}

void CElementEditDlg::OnLvnBegindragGrouplistdisplay(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	CPoint ptDrag, ptAction;
	m_nIndexLeftSel = pNMLV->iItem;
	m_pImageListDrag = m_UserGroupListCtrl.CreateDragImage(pNMLV->iItem, &ptDrag);
	m_pImageListDrag->SetBkColor(RGB(0,0,0));
	ptAction = pNMLV->ptAction;

	SetCapture();
	m_pImageListDrag->BeginDrag(0, ptAction - ptDrag);
	m_UserGroupListCtrl.ClientToScreen(&ptAction);
	m_pImageListDrag->DragEnter(NULL, ptAction);
	m_DeleteDragFlag = TRUE;
	*pResult = 0;
}



void CElementEditDlg::OnNMClickElementlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	LVITEM lvItem;
	TCHAR szBuffer[256];
	::ZeroMemory(&lvItem, sizeof(lvItem));
	::ZeroMemory(szBuffer, sizeof(256));

	lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
	lvItem.iItem = pNMItemActivate->iItem;
	lvItem.pszText = szBuffer;
	lvItem.cchTextMax = 256;
	m_DefaultListCtrl.GetItem(&lvItem);

	ElementGroup SearchGroup;
	m_DefaultCombo.GetLBText(m_DefaultIndex, SearchGroup.GroupName);
	list<ElementGroup>::iterator defaultpos = find_if(m_DefaultGroup.begin(), m_DefaultGroup.end(), SearchGroup);

	if (defaultpos != m_DefaultGroup.end()) {
		ElementInfo SearchElement;
		SearchElement.ElementName = CString(lvItem.pszText);
		list<ElementInfo>::iterator Elementpos = find_if((*defaultpos).ElementList.begin(), (*defaultpos).ElementList.end(), SearchElement);

		if (Elementpos != (*defaultpos).ElementList.end()) {
			
			GetDlgItem(IDC_SUBSTRING)->SetWindowText((*Elementpos).ElementIntroduceString);
// 			for(int iCount = 0; iCount < m_DefaultElementString.GetCount(); iCount++) {
// 				m_DefaultElementString.DeleteString(0);
// 			}
// 
// 			int Index = m_DefaultElementString.AddString((*Elementpos).ElementIntroduceString);
// 			m_DefaultElementString.SetTopIndex(Index);
		}
	}
	*pResult = 0;
}
