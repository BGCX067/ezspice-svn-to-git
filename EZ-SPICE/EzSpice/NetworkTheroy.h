#pragma once
#include "afxwin.h"


// CNetworkTheroy ��ȭ �����Դϴ�.

class CNetworkTheroy : public CDialog
{
	DECLARE_DYNAMIC(CNetworkTheroy)

public:
	CNetworkTheroy(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CNetworkTheroy();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()

protected:
	virtual void PostNcDestroy();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

public:
	CListBox m_ContentCtrl;
	CStatic m_PictureCtrl;

public:
	void InitializeTheroy();
	void UpdataPicture(int picturenumber);
	afx_msg void OnLbnSelchangeContent();
};
