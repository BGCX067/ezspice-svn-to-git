// InPlaceEdit.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "InPlaceEdit.h"
#include "resource.h"


// CInPlaceEdit

IMPLEMENT_DYNAMIC(CInPlaceEdit, CEdit)


CInPlaceEdit::CInPlaceEdit( int iItem, int iSubItem, CString sInitText )
{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_bESC = FALSE;
}

CInPlaceEdit::~CInPlaceEdit()
{
}


BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CInPlaceEdit 메시지 처리기입니다.



BOOL CInPlaceEdit::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN ) {
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || 
			pMsg->wParam == VK_ESCAPE || GetKeyState( VK_CONTROL) ) {
				::TranslateMessage(pMsg);
				::DispatchMessage(pMsg);
				return TRUE;            // 더이상 처리하지 않는다.

		}
	}
	return CEdit::PreTranslateMessage(pMsg);

}

int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1) return -1;

	// 적당한 폰트를 지정
	CFont* font = GetParent()->GetFont();
	SetFont(font);
	SetWindowText( m_sInitText );
	SetFocus();
	SetSel( 0, -1 );
	return 0;
}

void CInPlaceEdit::OnKillFocus(CWnd* pNewWnd)
{
	 CEdit::OnKillFocus(pNewWnd);

	 CString str;
	 GetWindowText(str);

	 // 리스트뷰 컨트롤의 부모에게 통지 보내기
	 LV_DISPINFO     dispinfo;
	 dispinfo.hdr.hwndFrom   = GetParent()->m_hWnd;
	 dispinfo.hdr.idFrom                 = GetDlgCtrlID();
	 dispinfo.hdr.code                   = LVN_ENDLABELEDIT;

	 dispinfo.item.mask              = LVIF_TEXT;
	 dispinfo.item.iItem             = m_iItem;
	 dispinfo.item.iSubItem          = m_iSubItem;
	 dispinfo.item.pszText           = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
	 dispinfo.item.cchTextMax        = str.GetLength();

	 GetParent()->GetParent()->SendMessage( WM_NOTIFY, IDC_IPEDIT, (LPARAM)&dispinfo);
	 DestroyWindow();
}

void CInPlaceEdit::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

void CInPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( nChar == VK_RETURN || nChar == VK_ESCAPE)           {
		GetParent()->SetFocus();
		return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);

	CString str;
	GetWindowText( str );
	CWindowDC dc(this);
	CFont *pFont = GetParent()->GetFont();
	CFont *pFontDC = dc.SelectObject( pFont );
	CSize size = dc.GetTextExtent( str );
	dc.SelectObject( pFontDC );
	size.cx += 5;               // 여분의 버퍼를 추가한다.

	// 클라이언트 사격형 크기를 알아낸다.
	CRect rect, parentrect;
	GetClientRect( &rect );
	GetParent()->GetClientRect( &parentrect );

	// 부모의 좌표로 사각형을 변환한다.
	ClientToScreen( &rect );
	GetParent()->ScreenToClient( &rect );

	// 컨트롤이 크기변경될 필요가 있느지 체크하고
	// 늘어날 공간이 있는지 체크
	if( size.cx > rect.Width() )            {
		if( size.cx + rect.left < parentrect.right ) rect.right = rect.left + size.cx;
		else rect.right = parentrect.right;
		MoveWindow( &rect );
	}
}
