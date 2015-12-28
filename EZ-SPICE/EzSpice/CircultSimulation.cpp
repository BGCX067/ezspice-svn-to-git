#include "StdAfx.h"
#include "CircultSimulation.h"
#include "NodePretreatment.h"
#include "LinearDCFormula.h"
#include "LinearACFormula.h"
#include "NonLinearDCFormula.h"
#include "NonLinearACFormula.h"
#include "OpampACFormula.h"
#include "OpampDCFormula.h"
#include "TransistorFormula.h"
#include "BaseElement.h"
#include <algorithm>

int	CCircultSimulation::m_CircultCase = 0;	
int CCircultSimulation::m_RLCCase = 0;		
ACPowerData	CCircultSimulation::m_ACData;
list<CBaseElement*> CCircultSimulation::m_CircuitList;

CCircultSimulation::CCircultSimulation(void)
{
}

CCircultSimulation::~CCircultSimulation(void)
{
	list<CBaseElement*> DeleteList;

	//동적 생성된 Geq, Leq 해제
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		list<CBaseElement*>::iterator lowpos = (*pos).AllElement.begin();
		while (lowpos != (*pos).AllElement.end()) {
	
			if ((*lowpos)->m_Flag == geq || (*lowpos)->m_Flag == leq) {

				list<CBaseElement*>::iterator Duplicatepos = find(DeleteList.begin(), DeleteList.end(), (*lowpos));
				if (Duplicatepos == DeleteList.end()) {
					DeleteList.push_back((*lowpos));
				}
			}
			++lowpos;
		}
		++pos;
	}

	list<CBaseElement*>::iterator deletepos = DeleteList.begin();
	while (deletepos != DeleteList.end()) {
		delete (*deletepos);
		++deletepos;
	}
}

BOOL CCircultSimulation::Initialize( list<CBaseElement*> &CircultList )
{
	m_CircuitList.clear();
	m_CircuitList.resize(CircultList.size());
	copy(CircultList.begin(), CircultList.end(), m_CircuitList.begin());

	m_NodeList.clear();
	CircultDistinction(CircultList);	//circult case distinction

	RLCCaseDitinction(CircultList);		//rcl case distinction

	FrequencyDitinction(CircultList);	//Frequency distinction		

	if (m_CircultCase != DCOpamp && m_CircultCase != ACOpamp) {
		NodePretreatment NewPretreatment;
		if (NewPretreatment.Processing(CircultList, m_NodeList) == FALSE) {
			AfxMessageBox(_T("회로가 정상적이지 않습니다"));	
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CCircultSimulation::FormulaConversion(vector<IPCSectorHeader> &SectorList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	//Prov Check;
	list<CBaseElement*>::iterator pos = Provlist.begin();
	while (pos != Provlist.end()) {
		if ((*pos)->m_InputElement.size() != 1) {
			AfxMessageBox(_T("프로브가 정상적이지 않습니다"));	
			return FALSE;
		}
		++pos;
	}

	//SectorList Initialize;
	for (UINT iCount = 0; iCount < Provlist.size(); iCount++) {
		IPCSectorHeader Header;
		Header.Circitcase = m_CircultCase;
		Header.Frequency = m_ACData.Frequency;
		SectorList.push_back(Header);
	}

	//Exception
	if (m_RLCCase == rc && m_ACData.WaveModel == square && SectorList.empty()) {
		IPCSectorHeader Header;
		Header.Circitcase = m_CircultCase;
		Header.Frequency = m_ACData.Frequency;
		SectorList.push_back(Header);	
	}

	if (m_NodeList.size() >= 1) {

		switch (m_CircultCase) {
			case linearDC:
				{
					LinearDCFormula Converter;
					if (Converter.Processing(m_NodeList, Datalist, Provlist) == FALSE) {
						AfxMessageBox(_T("회로가 정상적이지 않습니다"));	
						return FALSE;
					}
				}
				break;

			case nonlinearDC:
				{
					NonLinearDCFormula Converter;
					if (Converter.Processing(m_NodeList, Datalist, Provlist) == FALSE) {
						AfxMessageBox(_T("회로가 정상적이지 않습니다"));	
						return FALSE;
					}
				}
				break;

			case linearAC:
				{	
					LinearACFormula Converter;
					if (Converter.Processing(m_NodeList, Datalist, Provlist) == FALSE) {
						AfxMessageBox(_T("회로가 정상적이지 않습니다"));	
						return FALSE;
					}
				}
				break;

			case nonlinearAC:
				{
					if (m_RLCCase == rr) {
						NonLinearACFormula Converter;
						if (Converter.Processing(m_NodeList, Datalist, Provlist) == FALSE) {
							AfxMessageBox(_T("회로가 정상적이지 않습니다"));	
							return FALSE;
						}
					}
					else {
						AfxMessageBox(_T("비선형 AC의 경우 다이오드와 저항만 가능합니다."));
						return FALSE;
					}
				}
				break;

			case transistorFormula:
				{
					if (m_RLCCase == rr) {
						TransistorFormula Converter;
						if (Converter.Processing(m_NodeList, Datalist, Provlist) == FALSE) {
							AfxMessageBox(_T("회로가 정상적이지 않습니다"));	
							return FALSE;
						}

						if (m_ACData.Amplitude == -1) {
							for (UINT iCount = 0; iCount < SectorList.size(); iCount++) {
								SectorList[iCount].Circitcase = linearDC;
							}
						}
					}
					else {
						AfxMessageBox(_T("이상적인 트랜지스터의 경우 저항만 가능합니다."));
						return FALSE;
					}
				}
				break;

			default:
				break;
		}
	}
	else {
		switch (m_CircultCase) {
			case DCOpamp:
				{
					OpampDCFormula Converter;
					if (Converter.Processing(m_NodeList, Datalist, Provlist) == FALSE) {
						return FALSE;
					}

					for (UINT iCount = 0; iCount < SectorList.size(); iCount++) {
						SectorList[iCount].Circitcase = linearDC;
					}
				}
				break;

			case ACOpamp:
				{
					OpampACFormula Converter;
					if (Converter.Processing(m_NodeList, Datalist, Provlist) == FALSE) {
						return FALSE;
					}
				}
				break;
			
			default:
				break;
		}
	}

	return TRUE;
}

void CCircultSimulation::CircultDistinction(list<CBaseElement*> &CircultList)
{
	BOOL CaseFlag[5] = {FALSE,};

	list<CBaseElement*>::iterator pos = CircultList.begin();
	while (pos != CircultList.end()) {

		switch((*pos)->m_Flag) {
			case dcpower: 
				CaseFlag[0] = TRUE; 
				break;

			case acpower: 
				CaseFlag[1] = TRUE; 
				break;

			case transistor: 
				CaseFlag[4] = TRUE;
				break;

			case diode:
				CaseFlag[2] = TRUE; 
				break;

			case InvertingAmplifier:
			case NonInvertingAmplifier:
				CaseFlag[3] = TRUE;
				break;

			default:
				break;
		}
		++pos;
	}

	if (CaseFlag[4]) {
		m_CircultCase = transistorFormula;
	}
	else {
		if (CaseFlag[2]) {	//transistor 비선형
			if (CaseFlag[1]) {	//acpower
				m_CircultCase = nonlinearAC;

				if (CaseFlag[3]) m_CircultCase = ACOpamp;
			}
			else if(CaseFlag[0]) { //dcpower
				m_CircultCase = nonlinearDC;

				if (CaseFlag[3]) m_CircultCase = DCOpamp;
			}		
		}
		else { //default 선형
			if (CaseFlag[1]) { //acpower
				m_CircultCase = linearAC;

				if (CaseFlag[3]) m_CircultCase = ACOpamp;
			}
			else if(CaseFlag[0]) { //dcpower
				m_CircultCase = linearDC;

				if (CaseFlag[3]) m_CircultCase = DCOpamp;
			}
		}
	}
}

void CCircultSimulation::FrequencyDitinction( list<CBaseElement*> &CircultList )
{
	//If AC Circuit than Frequency check
	ACPowerData NewData;	
	list<CBaseElement*>::iterator pos = CircultList.begin();
	while (pos != CircultList.end()) {
		if ((*pos)->m_Flag == acpower) {

			list<ElementData> GetList;
			(*pos)->GetProperties(GetList);

			list<ElementData>::iterator Datapos = GetList.begin();
			while (Datapos != GetList.end()) {
				if ((*Datapos).Text == CString(_T("Frequency"))) {
					NewData.Frequency = (*Datapos).Value;
				}
				else if ((*Datapos).Text == CString(_T("Amplitude"))) {
					NewData.Amplitude = (*Datapos).Value;
				}
				else if ((*Datapos).Text == CString(_T("WaveModel"))) {
					NewData.WaveModel = (int)(*Datapos).Value;
				}
				++Datapos;	
			}
		}
		++pos;
	}
	m_ACData = NewData;
}

void CCircultSimulation::RLCCaseDitinction( list<CBaseElement*> &CircultList )
{
	BOOL CapacityFlag = FALSE;
	BOOL InductorFlag = FALSE;

	list<CBaseElement*>::iterator pos = CircultList.begin();
	while (pos != CircultList.end()) {

		if ((*pos)->m_Flag == capacity) CapacityFlag = TRUE;
		if ((*pos)->m_Flag == inductor) InductorFlag = TRUE;
		++pos;
	}

	if (CapacityFlag && InductorFlag) m_RLCCase = rlc;
	else if (CapacityFlag && !InductorFlag) m_RLCCase = rc;
	else if (!CapacityFlag && InductorFlag) m_RLCCase = rl;
	else m_RLCCase = rr;
}
