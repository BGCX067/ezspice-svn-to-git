// dllmain.cpp : DLL의 초기화 루틴을 정의합니다.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>
#include "BaseElement.h"
#include "ElementInformation.h"
#include "StructInformation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE ElementDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);
	if (dwReason == DLL_PROCESS_ATTACH) {
		TRACE0("Element.DLL을 초기화하고 있습니다.\n");
		
		if (!AfxInitExtensionModule(ElementDLL, hInstance))
			return 0;

		InitializeElement();
		new CDynLinkLibrary(ElementDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		TRACE0("Element.DLL을 종료하고 있습니다.\n");
		AfxTermExtensionModule(ElementDLL);
	}
	return 1; 
}
__declspec(dllexport)
BOOL GetElementInfoListToDll(list<ElementGroup> &CopyList)
{
	if (g_ElementList.size() > 0) {
		CopyList.clear();
		CopyList.resize(g_ElementList.size());
		copy(g_ElementList.begin(), g_ElementList.end(), CopyList.begin());
		return TRUE;
	}
	return FALSE;
}

__declspec(dllexport)
BOOL ElementToInformation(CBaseElement* pElement, ElementInformation &NewInfo)
{
	//default Save attribute

	NewInfo.m_Flag = pElement->m_Flag;
	NewInfo.m_ElementNumber = pElement->m_ElementNumber;

	NewInfo.m_Position = pElement->m_Position;
	NewInfo.m_LineTo = pElement->m_LineTo;
	NewInfo.m_MoveTo = pElement->m_MoveTo;
	NewInfo.m_Axis = pElement->m_Axis;
	NewInfo.m_LineMoveDirection = pElement->m_LineMoveDirection;
	NewInfo.m_ConnnectCount = pElement->m_ConnnectCount;

	NewInfo.m_CheckFlag = pElement->m_CheckFlag;
	NewInfo.m_GroundFlag = pElement->m_GroundFlag;
	NewInfo.m_RotationFlag = pElement->m_RotationFlag;
	NewInfo.m_ProvActivityFlag = pElement->m_ProvActivityFlag;
	NewInfo.m_UpdataFlag = pElement->m_UpdataFlag;

	switch (NewInfo.m_Flag) {
	case dcpower:
		{
			CPowerElementDC *ConvertElement = (CPowerElementDC*)pElement;
			NewInfo.m_Voltage = ConvertElement->m_Voltage;
			NewInfo.m_InternalResistance = ConvertElement->m_InternalResistance;
		}
		break;

	case resistance:
		{
			CResistanceElement *ConvertElement = (CResistanceElement*)pElement;
			NewInfo.m_Resistance = ConvertElement->m_Resistance;
			NewInfo.m_Power = ConvertElement->m_Power;
			NewInfo.m_Temperture = ConvertElement->m_Temperture;
			NewInfo.m_MaximumVoltage = ConvertElement->m_MaximumVoltage;
		}
		break;

	case inductor:
		{
			CInductorElement *ConvertElement = (CInductorElement*)pElement;
			NewInfo.m_Inductance = ConvertElement->m_Inductance;
			NewInfo.m_Temperture = ConvertElement->m_Temperture;
			NewInfo.m_MaximumVoltage = ConvertElement->m_MaximumVoltage;
		}
		break;

	case capacity:
		{
			CCapacityElement *ConvertElement = (CCapacityElement*)pElement;
			NewInfo.m_Capacitance = ConvertElement->m_Capacitance;
			NewInfo.m_Temperture = ConvertElement->m_Temperture;
			NewInfo.m_MaximumVoltage = ConvertElement->m_MaximumVoltage;
		}
		break;

	case  prov:
		{
			CProvElement *ConvertElement = (CProvElement*)pElement;
			NewInfo.m_ProvColor = ConvertElement->m_ProvColor;
		}
		break;

	case diode:
		{
			CDiodeElement *ConvertElement = (CDiodeElement*)pElement;
			NewInfo.m_OperatingPoint = ConvertElement->m_OperatingPoint;
			NewInfo.m_ThermalVoltage = ConvertElement->m_ThermalVoltage;
			NewInfo.m_SaturationCurrent = ConvertElement->m_SaturationCurrent;
			NewInfo.m_EmissonConefficient = ConvertElement->m_EmissonConefficient;
		}
		break;

	case acpower:
		{
			CPowerElementAC *ConvertElement = (CPowerElementAC*)pElement;
			NewInfo.m_Frequency = ConvertElement->m_Frequency;
			NewInfo.m_Amplitude = ConvertElement->m_Amplitude;
			NewInfo.m_WaveModel = ConvertElement->m_WaveModel;

		}
		break;

	case transistor:
		{
			CTransistorElement *ConvertElement = (CTransistorElement*)pElement;
			NewInfo.m_TROperatingPoint = ConvertElement->m_OperatingPoint;
			NewInfo.m_SaturatingPoint = ConvertElement->m_SaturatingPoint;
			NewInfo.m_Beta = ConvertElement->m_Beta;
		}
		break;

	case InvertingAmplifier:
		{
			CInvertAmpElement *ConvertElement = (CInvertAmpElement*)pElement;
			NewInfo.m_Firstresistance = ConvertElement->m_LeftResistance;
			NewInfo.m_Secondresistance = ConvertElement->m_UpResistance;
			NewInfo.m_MaxVoltage = ConvertElement->m_MaxVoltage;
			NewInfo.m_MinVoltage = ConvertElement->m_MinVoltage;
		}
		break;

	case NonInvertingAmplifier:
		{
			CNonInverAmpElement *ConvertElement = (CNonInverAmpElement*)pElement;
			NewInfo.m_Firstresistance = ConvertElement->m_LeftDownResistance;
			NewInfo.m_Secondresistance = ConvertElement->m_BottomResistance;
			NewInfo.m_MaxVoltage = ConvertElement->m_MaxVoltage;
			NewInfo.m_MinVoltage = ConvertElement->m_MinVoltage;
		}
		break;

	default:
		break;
	}

	return TRUE;
}

__declspec(dllexport)
CBaseElement* CreateElementToDll(int ElementNumber)
{
	CBaseElement *NewElement = NULL;
	

	//연결 구간 설정시(connectpoint)
	//첫번째꺼만 출구로 인식
	//즉 3개면 1번이 출구고 나머지 2,3번은 입구
	switch (ElementNumber) {
		case lines:
			{	
				static int LineCount = 0;
				NewElement = new CConnectLine;
				NewElement->m_ElementNumber = LineCount++;
			}
			break;

		case dcpower:
			{
				static int PowerCount = 1;
				NewElement = new CPowerElementDC;	
				NewElement->m_GroupName = CString(_T("DCPower"));
				NewElement->m_Name.Format(_T("V%d"), PowerCount);
				NewElement->m_ElementNumber = PowerCount++;
						
				NewElement->m_PositionArea = CRect(30, 30, 30, 30);
				NewElement->m_ConnectPoint.push_back(CPoint(0, -20));
				NewElement->m_ConnectPoint.push_back(CPoint(0, 20));
			}
			break;

		case resistance:
			{
				static int ResistanceCount = 1;
				NewElement = new CResistanceElement;
				NewElement->m_GroupName = CString(_T("Resistance"));
				NewElement->m_Name.Format(_T("R%d"), ResistanceCount);
				NewElement->m_ElementNumber = ResistanceCount++;
				
				NewElement->m_PositionArea = CRect(30, 30, 30, 30);
				NewElement->m_ConnectPoint.push_back(CPoint(20, 0));
				NewElement->m_ConnectPoint.push_back(CPoint(-20, 0));
			}
			break;

		case ground:
			{
				static int GroundCount = 1;
				NewElement = new CGroundElement;
				NewElement->m_GroupName = CString(_T("Ground"));
				NewElement->m_Name.Format(_T("G%d"), GroundCount);
				NewElement->m_ElementNumber = GroundCount++;

				NewElement->m_PositionArea = CRect(14, 8, 14, 8);
				NewElement->m_ConnectPoint.push_back(CPoint(0, 0));
			}
			break;

		case inductor:
			{
				static int InductorCount = 1;
				NewElement = new CInductorElement;
				NewElement->m_GroupName = CString(_T("Inductor"));
				NewElement->m_Name.Format(_T("L%d"), InductorCount);
				NewElement->m_ElementNumber = InductorCount++;

				NewElement->m_PositionArea = CRect(30, 30, 30, 30);
				NewElement->m_ConnectPoint.push_back(CPoint(20, 0));
				NewElement->m_ConnectPoint.push_back(CPoint(-20, 0));
			}
			break;

		case capacity:
			{
				static int CapacityCount = 1;
				NewElement = new CCapacityElement;
				NewElement->m_GroupName = CString(_T("Capacity"));
				NewElement->m_Name.Format(_T("C%d"), CapacityCount);
				NewElement->m_ElementNumber = CapacityCount++;

				NewElement->m_PositionArea = CRect(30, 30, 30, 30);
				NewElement->m_ConnectPoint.push_back(CPoint(20, 0));
				NewElement->m_ConnectPoint.push_back(CPoint(-20, 0));
			}
			break;

		case prov:
			{
				static int ProvCount = 1;
				NewElement = new CProvElement;
				NewElement->m_GroupName = CString(_T("Prov"));
				NewElement->m_Name.Format(_T("P%d"), ProvCount);
				NewElement->m_ElementNumber = ProvCount++;

				NewElement->m_PositionArea = CRect(5, 45, 35, 5);
			}
			break;

		case diode:
			{
				static int DiodeCount = 1;
				NewElement = new CDiodeElement;
				NewElement->m_GroupName = CString(_T("Diode"));
				NewElement->m_Name.Format(_T("D%d 1N4001"), DiodeCount);
				NewElement->m_ElementNumber = DiodeCount++;

				NewElement->m_PositionArea = CRect(30, 30, 30, 30);;
				NewElement->m_ConnectPoint.push_back(CPoint(20, 0));
				NewElement->m_ConnectPoint.push_back(CPoint(-20, 0));
			}
			break;

		case leq:
			{
				NewElement = new CLeqElement;
			}
			break;

		case geq:
			{
				NewElement = new CGeqElement;
			}
			break;

		case acpower:
			{
				static int ACPowerCount = 1;
				NewElement = new CPowerElementAC;	
				NewElement->m_GroupName = CString(_T("ACPower"));
				NewElement->m_Name.Format(_T("VG%d"), ACPowerCount);
				NewElement->m_ElementNumber = ACPowerCount++;

				NewElement->m_PositionArea = CRect(30, 30, 30, 30);
				NewElement->m_ConnectPoint.push_back(CPoint(0, -20));
				NewElement->m_ConnectPoint.push_back(CPoint(0, 20));
			}
			break;

		case transistor:
			{
				static int TransistorCount = 1;
				NewElement = new CTransistorElement;	
				NewElement->m_GroupName = CString(_T("Transistor"));
				NewElement->m_Name.Format(_T("T%d"), TransistorCount);
				NewElement->m_ElementNumber = TransistorCount++;

				NewElement->m_PositionArea = CRect(35, 30, 15, 30);
				NewElement->m_ConnectPoint.push_back(CPoint(0, 20));
				NewElement->m_ConnectPoint.push_back(CPoint(0, -20));
				NewElement->m_ConnectPoint.push_back(CPoint(-30, 0));
			}
			break;

		case composition:
			{
				NewElement = new CCompositionElement;
			}
			break;

		case InvertingAmplifier:
			{
				static int InvertingAmplifierCount = 1;
				NewElement = new CInvertAmpElement;	
				NewElement->m_GroupName = CString(_T("InvertingAmplifier"));
				NewElement->m_Name.Format(_T("OP%d"), InvertingAmplifierCount);
				NewElement->m_ElementNumber = InvertingAmplifierCount++;

				NewElement->m_PositionArea = CRect(130, 120, 90, 60);
				NewElement->m_ConnectPoint.push_back(CPoint(80, 0));
				NewElement->m_ConnectPoint.push_back(CPoint(-120, 0));
			}
			break;

		case NonInvertingAmplifier:
			{
				static int NonInvertingAmplifierCount = 1;
				NewElement = new CNonInverAmpElement;	
				NewElement->m_GroupName = CString(_T("NonInvertingAmplifier"));
				NewElement->m_Name.Format(_T("NOP%d"), NonInvertingAmplifierCount);
				NewElement->m_ElementNumber = NonInvertingAmplifierCount++;

				NewElement->m_PositionArea = CRect(110, 55, 110, 170);
				NewElement->m_ConnectPoint.push_back(CPoint(100, 0));
				NewElement->m_ConnectPoint.push_back(CPoint(-100, 0));
			}
			break;

		default:
			break;
	}
	return NewElement;
}

__declspec(dllexport)
CBaseElement* InformationToElement(ElementInformation &CurrentInfo)
{
	CBaseElement* NewElement = CreateElementToDll(CurrentInfo.m_Flag);

	NewElement->m_RotationFlag = Angle0;
	for (int iCount = 0; iCount < CurrentInfo.m_RotationFlag; iCount++) {
		NewElement->RotationElement();
	}
		
	switch (CurrentInfo.m_Flag) {
	case dcpower:
		{
			CPowerElementDC *ConvertElement = (CPowerElementDC*)NewElement;
			ConvertElement->m_Voltage = CurrentInfo.m_Voltage;
			ConvertElement->m_InternalResistance = CurrentInfo.m_InternalResistance;
		}
		break;

	case resistance:
		{
			CResistanceElement *ConvertElement = (CResistanceElement*)NewElement;
			ConvertElement->m_Resistance = CurrentInfo.m_Resistance;
			ConvertElement->m_Power = CurrentInfo.m_Power;
			ConvertElement->m_Temperture = CurrentInfo.m_Temperture;
			ConvertElement->m_MaximumVoltage = CurrentInfo.m_MaximumVoltage;
		}
		break;

	case inductor:
		{
			CInductorElement *ConvertElement = (CInductorElement*)NewElement;
			ConvertElement->m_Inductance = CurrentInfo.m_Inductance;
			ConvertElement->m_Temperture = CurrentInfo.m_Temperture;
			ConvertElement->m_MaximumVoltage = CurrentInfo.m_MaximumVoltage;
		}
		break;

	case capacity:
		{
			CCapacityElement *ConvertElement = (CCapacityElement*)NewElement;
			ConvertElement->m_Capacitance = CurrentInfo.m_Capacitance;
			ConvertElement->m_Temperture = CurrentInfo.m_Temperture;
			ConvertElement->m_MaximumVoltage = CurrentInfo.m_MaximumVoltage;
		}
		break;

	case  prov:
		{
			CProvElement *ConvertElement = (CProvElement*)NewElement;
			ConvertElement->m_ProvColor = CurrentInfo.m_ProvColor;
		}
		break;

	case diode:
		{
			CDiodeElement *ConvertElement = (CDiodeElement*)NewElement;
			ConvertElement->m_OperatingPoint = CurrentInfo.m_OperatingPoint;
			ConvertElement->m_ThermalVoltage = CurrentInfo.m_ThermalVoltage;
			ConvertElement->m_SaturationCurrent = CurrentInfo.m_SaturationCurrent;
			ConvertElement->m_EmissonConefficient = CurrentInfo.m_EmissonConefficient;
		}
		break;

	case acpower:
		{
			CPowerElementAC *ConvertElement = (CPowerElementAC*)NewElement;
			ConvertElement->m_Frequency = CurrentInfo.m_Frequency;
			ConvertElement->m_Amplitude = CurrentInfo.m_Amplitude;
			ConvertElement->m_WaveModel = CurrentInfo.m_WaveModel;
		}
		break;

	case transistor:
		{
			CTransistorElement *ConvertElement = (CTransistorElement*)NewElement;
			ConvertElement->m_OperatingPoint = CurrentInfo.m_TROperatingPoint;
			ConvertElement->m_SaturatingPoint = CurrentInfo.m_SaturatingPoint;
			ConvertElement->m_Beta = CurrentInfo.m_Beta;
		}
		break;

	case InvertingAmplifier:
		{
			CInvertAmpElement *ConvertElement = (CInvertAmpElement*)NewElement;
			ConvertElement->m_LeftResistance = CurrentInfo.m_Firstresistance;
			ConvertElement->m_UpResistance = CurrentInfo.m_Secondresistance;
			ConvertElement->m_MaxVoltage = CurrentInfo.m_MaxVoltage;
			ConvertElement->m_MinVoltage = CurrentInfo.m_MinVoltage;
		}
		break;

	case NonInvertingAmplifier:
		{
			CNonInverAmpElement *ConvertElement = (CNonInverAmpElement*)NewElement;
			ConvertElement->m_LeftDownResistance = CurrentInfo.m_Firstresistance;
			ConvertElement->m_BottomResistance = CurrentInfo.m_Secondresistance;
			ConvertElement->m_MaxVoltage = CurrentInfo.m_MaxVoltage;
			ConvertElement->m_MinVoltage = CurrentInfo.m_MinVoltage;
		}
		break;

	default:
		break;
	}

	//default Save attribute

	NewElement->m_Flag = CurrentInfo.m_Flag;
	NewElement->m_ElementNumber = CurrentInfo.m_ElementNumber;

	NewElement->m_Position = CurrentInfo.m_Position;
	NewElement->m_LineTo = CurrentInfo.m_LineTo;
	NewElement->m_MoveTo = CurrentInfo.m_MoveTo;
	NewElement->m_Axis =CurrentInfo.m_Axis;
	NewElement->m_LineMoveDirection = CurrentInfo.m_LineMoveDirection;
	NewElement->m_ConnnectCount = CurrentInfo.m_ConnnectCount;

	NewElement->m_CheckFlag = CurrentInfo.m_CheckFlag;
	NewElement->m_GroundFlag = CurrentInfo.m_GroundFlag;
	NewElement->m_RotationFlag = CurrentInfo.m_RotationFlag;
	NewElement->m_ProvActivityFlag = CurrentInfo.m_ProvActivityFlag;
	NewElement->m_UpdataFlag = CurrentInfo.m_UpdataFlag;

	return NewElement;
}