// SubListCtrl.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "SubListCtrl.h"
#include "InPlaceEdit.h"
#include "resource.h"


// CSubListCtrl

IMPLEMENT_DYNAMIC(CSubListCtrl, CListCtrl)

CSubListCtrl::CSubListCtrl()
{

}

CSubListCtrl::~CSubListCtrl()
{
}


BEGIN_MESSAGE_MAP(CSubListCtrl, CListCtrl)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, &CSubListCtrl::OnLvnEndlabeledit)
	ON_WM_LBUTTONDOWN()
//	ON_WM_PAINT()
END_MESSAGE_MAP()


int CSubListCtrl::AddColumn(LPCTSTR sColHeading,int nWidth /*=-1*/,int nFormat /*=LVCFMT_LEFT*/)
{
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if( nWidth == -1 )          {
		HD_ITEM hd_item;
		hd_item.mask = HDI_WIDTH;   //넓이지정을 한다는 것을 알려줍니다.
		pHeader->GetItem( nColumnCount - 1, &hd_item );        
		nWidth = hd_item.cxy;
	}
	return InsertColumn(nColumnCount, sColHeading, nFormat, nWidth, nColumnCount );
}

int CSubListCtrl::HitTestEx(CPoint &point, int *col) const
{
	int colnum = 0;
	int row = GetTopIndex();
	int bottom = row + GetCountPerPage();
	if(bottom > GetItemCount()) bottom = GetItemCount();

	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	
	for( ;row <=bottom;row++) {
		CRect rect;
		GetItemRect( row, &rect, LVIR_BOUNDS );
		if(rect.PtInRect(point)) {       // 컬럼 찾기
			for( colnum = 0; colnum < nColumnCount; colnum++ ) {
				int colwidth = GetColumnWidth(colnum);
				if(point.x >= rect.left && point.x <= (rect.left + colwidth )) {
					if( col ) *col = colnum;
					return row;
				}
				rect.left += colwidth;
			}
		}
	}
	return -1;
}

CEdit* CSubListCtrl::EditSubLabel( int nItem, int nCol )
{       // 리턴된 포인터는 저장되면 안됨(Save)

	if( !EnsureVisible( nItem, TRUE ) ) return NULL;

	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if( nCol >= nColumnCount || GetColumnWidth(nCol) < 5 )  return NULL;

	int offset = 0;
	for(int i = 0; i < nCol; i++) offset += GetColumnWidth(i);

	CRect rect;
	GetItemRect( nItem, &rect, LVIR_BOUNDS );

	CRect rcClient;
	GetClientRect( &rcClient );
	if(offset + rect.left < 0 || offset + rect.left > rcClient.right) {
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll( size );
		rect.left -= size.cx;
	}

	// 컬럼의 정렬방식 알아내기
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetColumn( nCol, &lvcol );
	DWORD dwStyle ;
	if ((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT) dwStyle = ES_LEFT;
	else if ((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dwStyle = ES_RIGHT;
	else
		dwStyle = ES_CENTER;

	rect.left += offset+4;
	rect.right = rect.left + GetColumnWidth( nCol ) - 3 ;    
	if( rect.right > rcClient.right) rect.right = rcClient.right;
	dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL;

	CString Coloumstring = GetItemText( nItem, nCol );
 	CEdit *pEdit = new CInPlaceEdit(nItem, nCol, Coloumstring);
 	pEdit->Create( dwStyle, rect, this, IDC_IPEDIT );
	pEdit->SetWindowText(Coloumstring);
	pEdit->SetSel(0, Coloumstring.GetAllocLength());
	return pEdit;
}


void CSubListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(GetFocus() != this) SetFocus();
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSubListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(GetFocus() != this) SetFocus();
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CSubListCtrl::OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LV_DISPINFO *plvDispInfo = (LV_DISPINFO*)pNMHDR;    
	LV_ITEM *plvItem = &plvDispInfo->item;

	if (plvItem->pszText != NULL)       {
		SetItemText(plvItem->iItem, plvItem->iSubItem, plvItem->pszText);
	}
	*pResult = FALSE;
}

void CSubListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	int index;
	CListCtrl::OnLButtonDown(nFlags, point);

	int colnum;
	if( ( index = HitTestEx( point, &colnum )) != -1  && colnum == 1)    {
			if( GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS )
				EditSubLabel( index, colnum );
	}
}

BOOL CSubListCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (((NMHDR*)lParam)->code) {
		case HDN_BEGINTRACKW    :
		case HDN_BEGINTRACKA    :   
			*pResult = TRUE;      // 마우스의 트래킹 방지(실제 크기 조정)
			return TRUE;           // 메시지를 처리했으므로    
	}
	return CListCtrl::OnNotify(wParam, lParam, pResult);
}

