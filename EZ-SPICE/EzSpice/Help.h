#pragma once
#include "afxwin.h"


// CHelp ��ȭ �����Դϴ�.

class CEzSpiceView;

class CHelp : public CDialog
{
	DECLARE_DYNAMIC(CHelp)

public:
	CHelp(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CHelp();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();
	void InitializeTree();
public:
	CTreeCtrl		m_TreeCtrl;
	CEzSpiceView	*m_pView;
	CImageList		ImgList;

	afx_msg void OnTvnSelchangedExaple(NMHDR *pNMHDR, LRESULT *pResult);
	void SetEzView(CEzSpiceView *pView);
	afx_msg void OnNMDblclkExaple(NMHDR *pNMHDR, LRESULT *pResult);
};
