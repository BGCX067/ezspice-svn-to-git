#pragma once

class COciloscopeView;

class CPlotter
{
public:
	CPlotter(void);
	~CPlotter(void);

public:
	CString		m_FlotterName;
	int			m_PlotterColor;
	int			m_SectorNumber;

public:
	int			m_XAxis;
	int			m_YAxis;

	double		m_RealXaxis;
	double		m_RealYaxis;
	double		*m_GrapehValue;

public:
	static CRect m_ClientRect;

public:
	void Draw(Gdiplus::Graphics* pDC, COciloscopeView *pView);
	BOOL IsObject(CPoint point);
};
