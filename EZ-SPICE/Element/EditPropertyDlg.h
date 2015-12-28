#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "SubListCtrl.h"


typedef struct _Property
{
	CString		PropertyName;
	double		PropertyValue;

	CString		Unit;				//����
	double		MaxValue;			//�ְ�
	double		MinValue;			//�ּҰ�

	_Property()
	{
		PropertyValue = 0.0f;
		MaxValue = 0.0f;
		MinValue = 0.0f;
	}
}Property;

enum PrintFlag
{
	Fnormal, Fdiode
};

class CEditPropertyDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditPropertyDlg)

public:
	CEditPropertyDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CEditPropertyDlg();

// ��ȭ ���� �������Դϴ�.
//	enum { IDD = IDD_PROPERTY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CStatic				m_ElementName;
	CString				m_strElementName;
	CStatic				m_NameStatic;
	CSubListCtrl		m_PropertyListCtrl;



	CString				m_GroupName;
	int					m_ElementFlag;
	vector<Property>	m_PropertyList;

	CBitmapButton		*m_PictureCtrl;
	CBitmapButton		*m_SineWave;
	CBitmapButton		*m_SquareWave;
	CBitmapButton		*m_TriangularWave;

	int					m_Flag;

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedHlep();
	
public:
	virtual BOOL OnInitDialog();
	void InitializeProperty(vector<Property> &PropertyList);	
	void SetElementFlag(int Flag);

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
};
