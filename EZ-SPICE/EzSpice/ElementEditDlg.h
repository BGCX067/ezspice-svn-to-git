#pragma once
#include "afxwin.h"
#include "afxcmn.h"

typedef struct _ElementInfo
{
public:
	CString     ElementGroupName;
	CString		ElementName;
	UINT		ElementNumber;
	UINT		ElementImageNumber;
	CString		ElementIntroduceString;

	bool operator()(struct _ElementInfo &rs) const {
		return ElementName == rs.ElementName;
	}
}ElementInfo;


typedef struct _ElementGroup
{
public:
	CString				GroupName;	
	list<ElementInfo>	ElementList;

	bool operator()(struct _ElementGroup &rs) const {
		return GroupName == rs.GroupName;
	}
}ElementGroup;


class CElementEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CElementEditDlg)

public:
	CElementEditDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CElementEditDlg();

// 대화 상자 데이터입니다.
//	enum { IDD = IDD_ELEMENTEDITDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	CComboBox			m_UserGroupCombo;
	CComboBox			m_DefaultCombo;
	CListCtrl			m_DefaultListCtrl;
	CListCtrl			m_UserGroupListCtrl;

	

	int					m_UserGroupIndex;
	int					m_DefaultIndex;
	CImageList			m_ImageList;

	//CopyList
	list<ElementGroup>	m_UserGroup;
	list<ElementGroup>	m_DefaultGroup;

	//Drag&Drap
	CImageList			*m_pImageListDrag;
	int					m_nIndexLeftSel;
	BOOL				m_DeleteDragFlag;

protected:
	virtual void OnCancel();
	virtual void PostNcDestroy();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedUserOk();
	afx_msg void OnBnClickedUserCancel();
	afx_msg void OnBnClickedUserHlep();
	afx_msg void OnCbnSelchangeUsergroup();
	afx_msg void OnCbnSelchangeElementgroup();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLvnBegindragElementlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBegindragGrouplistdisplay(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickElementlist(NMHDR *pNMHDR, LRESULT *pResult);

public:
	void InitializeData();
	void DrawDefaultElement();
	void DrawUserElement();
	void UpdateElementList();
	
	
};
