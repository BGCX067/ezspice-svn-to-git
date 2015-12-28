#include "StdAfx.h"
#include "Plotter.h"
#include "StructInformation.h"
#include "Ociloscope.h"
#include "OciloscopeDoc.h"
#include "OciloscopeView.h"

CRect CPlotter::m_ClientRect = CRect(0,0,0,0);

CPlotter::CPlotter(void)
{
	m_XAxis = 100;
	m_YAxis = 40;
}

CPlotter::~CPlotter(void)
{
}


BOOL CPlotter::IsObject(CPoint point)
{
	if (point.x > m_XAxis -10 && point.x < m_XAxis + 10) {
		return TRUE;
	}
	return FALSE;
}

void CPlotter::Draw( Gdiplus::Graphics* pDC, COciloscopeView *pView)
{
	Gdiplus::Font font(_T("Arial"), 20, FontStyleRegular, UnitPixel);
	SolidBrush sbrush(Color::Black);
	StringFormat format;
	format.SetHotkeyPrefix(HotkeyPrefixShow);

	Pen PlotterPen(Color(255, 171, 40, 38), 1.0f);
	Pen NewPen(Color(255, 171, 40, 38), 2.0f);
	sbrush.SetColor(Color(255, 171, 40, 38));

	switch (m_PlotterColor) {
		case darkred:
			NewPen.SetColor(Color(255, 171, 40, 38));
			sbrush.SetColor(Color(255, 171, 40, 38));
			PlotterPen.SetColor(Color(255, 171, 40, 38));
			break;

		case darkgreen:
			NewPen.SetColor(Color(255, 74, 186, 26));
			sbrush.SetColor(Color(255, 74, 186, 26));
			PlotterPen.SetColor(Color(255, 74, 186, 26));
			break;

		case darkyellow:
			NewPen.SetColor(Color(255, 200, 130, 35));
			sbrush.SetColor(Color(255, 200, 130, 35));
			PlotterPen.SetColor(Color(255,  200, 130, 35));
			break;

		case darkblue:
			NewPen.SetColor(Color(255, 35, 99, 200));
			sbrush.SetColor(Color(255, 35, 99, 200));
			PlotterPen.SetColor(Color(255, 35, 99, 200));
			break;
	}

	pDC->DrawString(m_FlotterName, -1, &font, RectF((float)m_XAxis -9, (float)m_YAxis -35, (float)m_XAxis +10, (float)m_YAxis-15), &format, &sbrush);

	GraphicsPath path;
	path.AddLine(m_XAxis -10, m_YAxis -40, m_XAxis -10, m_YAxis -10);
	path.AddLine(m_XAxis -10, m_YAxis -10, m_XAxis, m_YAxis);
	path.AddLine(m_XAxis, m_YAxis, m_XAxis +10, m_YAxis -10);
	path.AddLine(m_XAxis +10, m_YAxis -10, m_XAxis +10, m_YAxis -40);
	pDC->DrawPath(&PlotterPen, &path);

	int real_width = m_ClientRect.Width();
	int real_height = m_ClientRect.Height();

	pDC->DrawLine(&NewPen, m_XAxis, m_YAxis, m_XAxis, m_YAxis +(real_height-60));

	//y축
	real_height = m_ClientRect.Height()-40;
	double VirtualXpos = m_XAxis-100;
	double TotalVolateExtent = fabs(pView->m_MaxVoltage) + fabs(pView->m_MinVoltage);
	double OnePos = (double)(real_height-40)/TotalVolateExtent;

	if (VirtualXpos != 0) {
		
		double RepetitionCount = pView->m_Hertz/pView->m_Anniversary;
		if (pView->m_SectorList[m_SectorNumber].Circitcase == linearDC || pView->m_SectorList[m_SectorNumber].Circitcase == nonlinearDC) {
			RepetitionCount = 1;
		}

		if (RepetitionCount <= 1) { //한주기보다 작을때
			int MaxCount = (int)(DivideSector*RepetitionCount) -1;
			double xstep = (real_width - 140)/double(MaxCount);
			int XCount = (int)(VirtualXpos/xstep);
			m_RealXaxis = VirtualXpos /((real_width - 140)/pView->m_Hertz);
			m_RealYaxis = m_GrapehValue[XCount];

			if (pView->m_MaxVoltage >= m_GrapehValue[XCount] && pView->m_MinVoltage <= m_GrapehValue[XCount]) {
				pDC->DrawLine(&NewPen, PointF((float)80, (float)(real_height -((m_GrapehValue[XCount] + fabs(pView->m_MinVoltage))*OnePos)))
					, PointF((float)real_width -40, (float)(real_height -((m_GrapehValue[XCount] + fabs(pView->m_MinVoltage))*OnePos))));
			}
		}
		else {
			int MaxCount = (int)(DivideSector*RepetitionCount) -1;
			double xstep = (real_width - 140)/double(MaxCount);
			int XCount = (int)(VirtualXpos/xstep);

			int ConvertCount = 0;
			if (XCount != 0 && XCount/(DivideSector-1) >= 1) {
				ConvertCount = (DivideSector-1)*(XCount/(DivideSector-1));
				XCount -= ConvertCount;
			}

			m_RealXaxis = VirtualXpos /((real_width - 140)/pView->m_Hertz);
			m_RealYaxis = m_GrapehValue[XCount];

			if (pView->m_MaxVoltage >= m_GrapehValue[XCount] && pView->m_MinVoltage <= m_GrapehValue[XCount]) {
				pDC->DrawLine(&NewPen, PointF((float)80, (float)(real_height -((m_GrapehValue[XCount] + fabs(pView->m_MinVoltage))*OnePos)))
					, PointF((float)real_width -40, (float)(real_height -((m_GrapehValue[XCount] + fabs(pView->m_MinVoltage))*OnePos))));
			}
		}
	}
	else {
		pDC->DrawLine(&NewPen, PointF((float)80, (float)(real_height -((m_GrapehValue[0] + fabs(pView->m_MinVoltage))*OnePos)))
			, PointF((float)real_width -40, (float)(real_height -((m_GrapehValue[0] + fabs(pView->m_MinVoltage))*OnePos))));
	}
}



