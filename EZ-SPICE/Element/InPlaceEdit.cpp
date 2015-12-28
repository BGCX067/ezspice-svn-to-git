// InPlaceEdit.cpp : ���� �����Դϴ�.
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



// CInPlaceEdit �޽��� ó�����Դϴ�.



BOOL CInPlaceEdit::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN ) {
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || 
			pMsg->wParam == VK_ESCAPE || GetKeyState( VK_CONTROL) ) {
				::TranslateMessage(pMsg);
				::DispatchMessage(pMsg);
				return TRUE;            // ���̻� ó������ �ʴ´�.

		}
	}
	return CEdit::PreTranslateMessage(pMsg);

}

int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1) return -1;

	// ������ ��Ʈ�� ����
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

	 // ����Ʈ�� ��Ʈ���� �θ𿡰� ���� ������
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

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
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
	size.cx += 5;               // ������ ���۸� �߰��Ѵ�.

	// Ŭ���̾�Ʈ ����� ũ�⸦ �˾Ƴ���.
	CRect rect, parentrect;
	GetClientRect( &rect );
	GetParent()->GetClientRect( &parentrect );

	// �θ��� ��ǥ�� �簢���� ��ȯ�Ѵ�.
	ClientToScreen( &rect );
	GetParent()->ScreenToClient( &rect );

	// ��Ʈ���� ũ�⺯��� �ʿ䰡 �ִ��� üũ�ϰ�
	// �þ ������ �ִ��� üũ
	if( size.cx > rect.Width() )            {
		if( size.cx + rect.left < parentrect.right ) rect.right = rect.left + size.cx;
		else rect.right = parentrect.right;
		MoveWindow( &rect );
	}
}
