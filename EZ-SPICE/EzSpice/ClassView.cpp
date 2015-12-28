// �� MFC ���� �ҽ� �ڵ�� MFC Microsoft Office Fluent ����� �������̽�("Fluent UI")�� 
// ����ϴ� ����� ���� �ָ�, MFC C++ ���̺귯�� ����Ʈ��� ���Ե� 
// Microsoft Foundation Classes Reference �� ���� ���� ������ ���� 
// �߰������� �����Ǵ� �����Դϴ�.  
// Fluent UI�� ����, ��� �Ǵ� �����ϴ� �� ���� ��� ����� ������ �����˴ϴ�.  
// Fluent UI ���̼��� ���α׷��� ���� �ڼ��� ������ 
// http://msdn.microsoft.com/officeui�� �����Ͻʽÿ�.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"
#include "MainFrm.h"
#include "ClassView.h"
#include "Resource.h"
#include "EzSpice.h"
#include "EzSpiceDoc.h"
#include "EzSpiceView.h"
#include "BaseElement.h"


//////////////////////////////////////////////////////////////////////
// ����/�Ҹ�
//////////////////////////////////////////////////////////////////////

CClassView::CClassView()
{
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}

CClassView::~CClassView()
{
}

BEGIN_MESSAGE_MAP(CClassView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassView �޽��� ó����

int CClassView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// �並 ����ϴ�.
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("Ŭ���� �並 ������ ���߽��ϴ�.\n");
		return -1;      // ������ ���߽��ϴ�.
	}


	// �̹����� �ε��մϴ�.
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);
	// ��� ����� �θ� �������� �ƴ� �� ��Ʈ���� ���� ����õ˴ϴ�.
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);
	m_CircultCount = 0;
	m_ChildCount = 0;
	m_LastDoc = NULL;

	return 0;
}

void CClassView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CClassView::ReloadElementData(CEzSpiceDoc *pDoc, BOOL ClearFlag)
{
	if (m_LastDoc != pDoc || ClearFlag == TRUE) {
		m_wndClassView.DeleteAllItems();
		m_LastDoc = pDoc;
		m_CircultCount = 0;

		CString Title = pDoc->GetTitle();
		HTREEITEM hRoot = m_wndClassView.InsertItem(Title, 0, 0);
		m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
	}
	
	if (m_CircultCount != pDoc->m_CircuitList.size()) {
		m_CircultCount = pDoc->m_CircuitList.size();

		HTREEITEM hRoot = m_wndClassView.GetRootItem();
		list<CBaseElement*>::iterator pos = pDoc->m_CircuitList.begin();
		while (pos != pDoc->m_CircuitList.end()) {
			
			if ((*pos)->m_Name != CString(_T("Line"))) {
				Reconstruction(hRoot, pos);
			}
			++pos;
		}
		m_wndClassView.Expand(hRoot, TVE_EXPAND);
	}
}

void CClassView::Reconstruction(HTREEITEM parent, list<CBaseElement*>::iterator &pos)
{
	HTREEITEM SearchItem = m_wndClassView.GetChildItem(parent);
	while (SearchItem != NULL) {
		CString ItemText = m_wndClassView.GetItemText(SearchItem);
		if ((*pos)->m_GroupName == ItemText) {
			
			{
				BOOL nFlag = TRUE;
				HTREEITEM ChildItem = m_wndClassView.GetChildItem(SearchItem);
				while (ChildItem != NULL) {
					CString ElementString = m_wndClassView.GetItemText(ChildItem);
					if ((*pos)->m_Name == ElementString) {
						nFlag = FALSE;
						break;		
					}
					ChildItem = m_wndClassView.GetNextItem(ChildItem, TVGN_NEXT);
				}

				if (nFlag) {
					m_wndClassView.InsertItem((*pos)->m_Name, 3, 3, SearchItem);
					m_wndClassView.Expand(SearchItem, TVE_EXPAND);
				}
			}
			return;
		}
		SearchItem = m_wndClassView.GetNextItem(SearchItem, TVGN_NEXT);
	}
	HTREEITEM hClass = m_wndClassView.InsertItem((*pos)->m_GroupName, 1, 1, parent);
	m_wndClassView.InsertItem((*pos)->m_Name, 3, 3, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);
}

void CClassView::DeleteTreeItem(CBaseElement *pElement)
{
	HTREEITEM hRoot = m_wndClassView.GetRootItem();
	HTREEITEM SearchItem = m_wndClassView.GetChildItem(hRoot);
	while (SearchItem != NULL) {
		CString ItemText = m_wndClassView.GetItemText(SearchItem);
		if (pElement->m_GroupName == ItemText) {

			{
				BOOL nFlag = TRUE;
				HTREEITEM ChildItem = m_wndClassView.GetChildItem(SearchItem);
				while (ChildItem != NULL) {
					CString ElementString = m_wndClassView.GetItemText(ChildItem);
					if (pElement->m_Name == ElementString) {
						m_wndClassView.DeleteItem(ChildItem);
						break;		
					}
					ChildItem = m_wndClassView.GetNextItem(ChildItem, TVGN_NEXT);
				}
				
				ChildItem = m_wndClassView.GetChildItem(SearchItem);
				if (ChildItem == NULL) {
					m_wndClassView.DeleteItem(SearchItem);
				}
			}
			return;
		}
		SearchItem = m_wndClassView.GetNextItem(SearchItem, TVGN_NEXT);
	}
}


void CClassView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER); //���̾ƿ� ����
	m_wndClassView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CClassView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CClassView::OnPaint()
{
	CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

	CRect rectTree;
	m_wndClassView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CClassView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndClassView.SetFocus();
}

void CClassView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("��Ʈ���� �ε��� �� �����ϴ�. %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndClassView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);
}

BOOL CClassView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pnmh = (NMHDR*) lParam;
	switch (pnmh->code) {
		case TVN_SELCHANGED:
			{
				LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pnmh);
				CString SelectString = m_wndClassView.GetItemText(pNMTreeView->itemNew.hItem);

				CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
				CEzSpiceDoc *pDoc = (CEzSpiceDoc*)pFrame->MDIGetActive()->GetActiveView()->GetDocument();

				list<CBaseElement*>::iterator pos = pDoc->m_CircuitList.begin();
				while (pos != pDoc->m_CircuitList.end()) {
					if ((*pos)->m_Name == SelectString) {
						(*pos)->OnEditProperties();
					}
					++pos;
				}
				m_wndClassView.SelectItem(NULL);
			}
			break;

		default:
			break;
	}
	return CDockablePane::OnNotify(wParam, lParam, pResult);
}
