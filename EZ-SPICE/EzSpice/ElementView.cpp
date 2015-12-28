// ElementView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EzSpice.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include "ElementView.h"
#include "MainFrm.h"
#include <algorithm>


// CElementView

IMPLEMENT_DYNAMIC(CElementView, CDockablePane)

CElementView::CElementView()
{

}

CElementView::~CElementView()
{
}


BEGIN_MESSAGE_MAP(CElementView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// CElementView 메시지 처리기입니다.

int CElementView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_3D_VS2005, rectDummy, this, 1)) {
		TRACE0("출력 탭 창을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}

	m_wndTabs.SetLocation(CMFCTabCtrl::LOCATION_TOP);

	CBitmap Bitmap;
	Bitmap.LoadBitmap(IDB_ELEMENT);
	m_ElementImageList.Create(32, 32, ILC_COLOR32 | ILC_MASK, 8, 0);
	m_ElementImageList.Add((&Bitmap), RGB(0,0,0));

	return 0;
}

void CElementView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CElementView::RemoveAllPageList()
{
	m_wndTabs.RemoveAllTabs();
	list<CElementListCtrl*>::iterator pos = m_PageList.begin();
	while (pos != m_PageList.end()) {
		delete (*pos);
		++pos;
	}
	m_PageList.clear();
}

void CElementView::ReloadElementData()
{
 	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	if (pFrame != NULL) {
		RemoveAllPageList();

		const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;
		COLORREF crBkColor = ::GetSysColor(COLOR_3DFACE);
		CRect rectDummy;
		rectDummy.SetRectEmpty();

		int PageNumber = 1, PageCount = 0;
		list<ElementGroup>::iterator pos = pFrame->m_UserGroupList.begin();
		while (pos != pFrame->m_UserGroupList.end()) {
		
			CElementListCtrl *NewCtrl = new CElementListCtrl;
			NewCtrl->Create(dwStyle, rectDummy, &m_wndTabs, ++PageNumber);	
			NewCtrl->SetFont(&m_Font);
			NewCtrl->SetBkColor(crBkColor);
			NewCtrl->SetTextBkColor(crBkColor);
			NewCtrl->SetImageList(&m_ElementImageList, LVSIL_NORMAL);
			NewCtrl->SetGroupNumber(PageCount);

			m_wndTabs.AddTab(NewCtrl, (*pos).GroupName, (UINT)PageCount++);
			m_PageList.push_back(NewCtrl);

			list<ElementInfo>::iterator elementpos = (*pos).ElementList.begin();
			while (elementpos != (*pos).ElementList.end()) {
				NewCtrl->InsertItem((*elementpos).ElementImageNumber, (*elementpos).ElementName, (*elementpos).ElementImageNumber);
				++elementpos;
			}
			++pos;
		}
	}
}

void CElementView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}



//////////////////////////////////////////////////////////////////////////
// CElementListCtrl

CElementListCtrl::CElementListCtrl()
{
	
}

CElementListCtrl::~CElementListCtrl()
{

}

void CElementListCtrl::SetGroupNumber(int Number)
{
	m_GroupNumber = Number;
}

BEGIN_MESSAGE_MAP(CElementListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, &CElementListCtrl::OnNMClick)
//	ON_WM_KEYDOWN()
//ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CElementListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
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

	this->GetItem(&lvItem);
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	int iCount = 0;
	list<ElementGroup>::iterator userpos = pFrame->m_UserGroupList.begin();
	while (userpos != pFrame->m_UserGroupList.end()) {
		if (iCount++ == m_GroupNumber) {
			break;
		}
		++userpos;
	}

	if (userpos != pFrame->m_UserGroupList.end()) {
		ElementInfo SearchElement;
		SearchElement.ElementName = CString(lvItem.pszText);
		list<ElementInfo>::iterator Elementpos = find_if((*userpos).ElementList.begin(), (*userpos).ElementList.end(), SearchElement);

		if (Elementpos != (*userpos).ElementList.end()) {
			CEzSpiceView *pView = (CEzSpiceView*)pFrame->MDIGetActive()->GetActiveView();
			
			int ElementCount = pView->GetDocument()->GetElementCount((*Elementpos).ElementNumber);
			if ((*Elementpos).ElementNumber == acpower) {
				if (ElementCount == 1) {
					AfxMessageBox(_T("더이상 AC 전압원을 추가할 수 없습니다."));
					return;
				}
			}
			pView->InsertElement((*Elementpos).ElementNumber);
			pView->m_ActivityFlag = FALSE;
		}
	}

	*pResult = 0;
}



void CElementView::OnMouseMove(UINT nFlags, CPoint point)
{

	CDockablePane::OnMouseMove(nFlags, point);
}
