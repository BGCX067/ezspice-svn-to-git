#pragma once
#include "afxwin.h"


// CNetworkTheroy 대화 상자입니다.

class CNetworkTheroy : public CDialog
{
	DECLARE_DYNAMIC(CNetworkTheroy)

public:
	CNetworkTheroy(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CNetworkTheroy();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

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
