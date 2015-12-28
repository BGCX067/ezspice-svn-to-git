#include "StdAfx.h"
#include "OpampDCFormula.h"
#include "CircultSimulation.h"
#include "MainFrm.h"
#include "ElementFactory.h"
#include "LinearDCFormula.h"
#include "NonLinearDCFormula.h"
#include "NodePretreatment.h"
#include <algorithm>


OpampDCFormula::OpampDCFormula(void)
{
}

OpampDCFormula::~OpampDCFormula(void)
{
}

void OpampDCFormula::Initialize()
{
	//원본 및 연결 정보도 복사
	m_CircuitList.clear();
	list<CBaseElement*>::iterator pos = CCircultSimulation::m_CircuitList.begin();
	while (pos != CCircultSimulation::m_CircuitList.end()) {
	
		m_CircuitList.push_back((*pos));

		RecoveryConnectElment NewProgress;
		NewProgress.pElement = (*pos);
		NewProgress.ConnectElement.clear();
		NewProgress.ConnectElement.resize((*pos)->m_ConnectElement.size());
		copy((*pos)->m_ConnectElement.begin(), (*pos)->m_ConnectElement.end(), NewProgress.ConnectElement.begin());

		m_CircuitRecoveryList.push_back(NewProgress);
		++pos;
	}
}

int OpampDCFormula::CheckCircuit(list<CBaseElement*> &CurrentList)
{
	list<CBaseElement*>::iterator pos = CurrentList.begin();
	while (pos != CurrentList.end()) {
		if ((*pos)->m_Flag == transistor || (*pos)->m_Flag == diode) {
			return nonlinearDC;
		}
		++pos;
	}
	return linearDC;
}

void OpampDCFormula::AheadProcessing( list<IPCData> &Datalist )
{

}

void OpampDCFormula::RearProcessing( list<IPCData> &Datalist )
{
	list<RecoveryConnectElment>::iterator pos = m_CircuitRecoveryList.begin();
	while (pos != m_CircuitRecoveryList.end()) {

		list<CBaseElement*>::iterator searchpos = find(m_CircuitList.begin(), m_CircuitList.end(), (*pos).pElement);
		if (searchpos != m_CircuitList.end()) {

			(*searchpos)->m_ConnectElement.clear();
			(*searchpos)->m_ConnectElement.resize((*pos).ConnectElement.size());
			copy((*pos).ConnectElement.begin(), (*pos).ConnectElement.end(), (*searchpos)->m_ConnectElement.begin());

			(*searchpos)->m_InputElement.clear();
			(*searchpos)->m_OutputElement.clear();
		}
		++pos;
	}

	//진행방향을 위해서 한번더 진행방향결정 수행

	CCircultSimulation::m_CircultCase = linearDC;

	list<Node> NodeList;
	NodePretreatment NewPretreatment;
	if (NewPretreatment.Processing(CCircultSimulation::m_CircuitList, NodeList) == FALSE) {
		AfxMessageBox(_T("회로가 정상적이지 않습니다"));	
	}

	//새로 생성된 모든 소자 삭제
	list<CBaseElement*>::iterator deletepos = m_TempList.begin();
	while (deletepos != m_TempList.end()) {
		delete (*deletepos);
		++deletepos;
	}
	m_TempList.clear();
}

BOOL OpampDCFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist )
{
	//Op-amp의 경우 Op-amp를 중심으로 좌우 노드를 개별적으로 수행하게 된다.
	//다른 Circuit의 경우 노드를 결정하고 오지만 이 경우 좌우 회로에 대해 구분하여 이 구문에서 수행한다,
	//왼쪽의 회로의 경우 Op-amp시점의 전압을 구하는 역활을 하게 되며
	//오른쪽 회로의 경우 구한 전압을 가지고 Op-amp증폭에 의한 전압으로 다시 회로가 동작된다.
	//이 구문에서 선형인지 비선형인지를 체크하여 뉴튼 랩손법을 적용해야 할지 경정해야 한다.

	//copy circuit
	Initialize();

	//left circuit, right circult  (between opamp)
	double InputVoltage = 0;
	list<IPCData> LeftRealDataList;
	if (DivideLeftCircuit(InputVoltage, LeftRealDataList, Provlist) == FALSE) {
		RearProcessing(Datalist);
		return FALSE;
	}

	if (DivideRightCircuit(InputVoltage, Datalist, Provlist) == FALSE) {
		RearProcessing(Datalist);
		return FALSE;
	}

	list<IPCData>::iterator datapos = LeftRealDataList.begin();
	while (datapos != LeftRealDataList.end()) {
		Datalist.push_back((*datapos));
		++datapos;
	}
	
	RearProcessing(Datalist);
	return TRUE;
}

BOOL OpampDCFormula::DivideRightCircuit(double InputVoltage, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	m_TempCircuitList.clear();

	//op-amp 탐색
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		if ((*pos)->m_Flag == InvertingAmplifier || (*pos)->m_Flag == NonInvertingAmplifier) {
			break;
		}
		++pos;
	}

	if (pos != m_CircuitList.end()) {

		//가정 1. opamp의 경우 회전이 이루어 지지 않기 때문에 첫번째 ConnectPoint에 연결된것이 오른쪽이다.

		list<CPoint>::iterator LeftPoint = (*pos)->m_ConnectPoint.begin();

		//오른쪽 처음 라인 검색
		list<CBaseElement*>::iterator elementpos = (*pos)->m_ConnectElement.begin();
		while (elementpos != (*pos)->m_ConnectElement.end()) {

			if ((*elementpos)->m_Flag == lines) {
				if ((*elementpos)->m_LineTo == (*pos)->m_Position + (*LeftPoint) || (*elementpos)->m_MoveTo == (*pos)->m_Position + (*LeftPoint)) {
					break;
				}
			}
			++elementpos;
		}

		if (elementpos != (*pos)->m_ConnectElement.end()) {
			m_TempCircuitList.push_back((*elementpos));
			RecursiveCircuitSearch((*elementpos));

			//전달된 inputvolatage로 회로 수행
			//opamp위치에 전압+라인+그라운드 삽입
			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			CBaseElement* pPower = GetElementGroup(dcpowercollection, InputVoltage, (*pos));

			(*elementpos)->DeleteElement((*pos));
			(*elementpos)->m_ConnectElement.push_back(pPower);
			pPower->m_ConnectElement.push_back((*elementpos));

			int TempCae = CCircultSimulation::m_CircultCase;
			if (CheckCircuit(m_TempCircuitList) == linearDC) {
				
				CCircultSimulation::m_CircultCase = linearDC;
				
				list<Node> NodeList;
				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}

				LinearDCFormula Converter;
				if (Converter.Processing(NodeList, Datalist, Provlist) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}
			}
			else { //비선형

				CCircultSimulation::m_CircultCase = nonlinearDC;

				list<Node> NodeList;
				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}

				NonLinearDCFormula Converter;
				if (Converter.Processing(NodeList, Datalist, Provlist) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}
			}
			CCircultSimulation::m_CircultCase = TempCae;
		}
	}
	return TRUE;
}



BOOL OpampDCFormula::DivideLeftCircuit(double &InputVoltage, list<IPCData> &LeftRealDatalist, list<CBaseElement*> &Provlist)	//가장 중요
{
	m_TempCircuitList.clear();

	//op-amp 탐색
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		if ((*pos)->m_Flag == InvertingAmplifier || (*pos)->m_Flag == NonInvertingAmplifier) {
			break;
		}
		++pos;
	}

	if (pos != m_CircuitList.end()) {
		
		//가정 1. opamp의 경우 회전이 이루어 지지 않기 때문에 두번째 ConnectPoint에 연결된것이 왼쪽이다.

		list<CPoint>::iterator LeftPoint = (*pos)->m_ConnectPoint.begin();
		LeftPoint++;

		//왼쪽 처음 라인 검색
		list<CBaseElement*>::iterator elementpos = (*pos)->m_ConnectElement.begin();
		while (elementpos != (*pos)->m_ConnectElement.end()) {

			if ((*elementpos)->m_Flag == lines) {
				if ((*elementpos)->m_LineTo == (*pos)->m_Position + (*LeftPoint) || (*elementpos)->m_MoveTo == (*pos)->m_Position + (*LeftPoint)) {
					break;
				}
			}
			++elementpos;
		}

		if (elementpos != (*pos)->m_ConnectElement.end()) {
			m_TempCircuitList.push_back((*elementpos));
			RecursiveCircuitSearch((*elementpos));	//회로 탐색하여 m_TempCircuitList에 저장		

			//(*elementpos에 opamp제거후 저항 삽입 (*pos)가 opamp
			//opamp 저항 삽입 (저항 + 라인 + 그라운드)
			CBaseElement* pResistance = GetElementGroup(resistancecollection, InputVoltage, (*pos));
			if (pResistance == NULL) {
				return FALSE;
			}

			//opamp제거 및 왼쪽 라인 체크 (왼쪽 라인에 프로브를 달기 위해)
			(*elementpos)->DeleteElement((*pos));
			(*elementpos)->m_ConnectElement.push_back(pResistance);
			pResistance->m_ConnectElement.push_back((*elementpos));

			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			list<Node> NodeList;
			list<IPCData> DataList;
			int TempCae = CCircultSimulation::m_CircultCase;

			//현재 회로가 선형인지 비선형 인지 체크
			if (CheckCircuit(m_TempCircuitList) == linearDC) {
				
				CCircultSimulation::m_CircultCase = linearDC;

				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}
		
				list<CBaseElement*> NewProvList;
				CBaseElement *NewProv = pFactory->CreateElement(prov);
				NewProv->m_InputElement.push_back((*elementpos));
				NewProvList.push_back(NewProv);
				m_TempList.push_back(NewProv);

				list<CBaseElement*>::iterator provpos = Provlist.begin();
				while (provpos != Provlist.end()) {
					NewProvList.push_back((*provpos));
					++provpos;
				}
				
				LinearDCFormula Converter;
				if (Converter.Processing(NodeList, DataList, NewProvList) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}

				list<IPCData>::iterator datapos = DataList.begin();
				while (datapos != DataList.end()) {
					if ((*datapos).ProvNumber != NewProv->m_ElementNumber) {
						LeftRealDatalist.push_back((*datapos));
					}
					++datapos;
				}
				
			}
			else {	//비선형
				CCircultSimulation::m_CircultCase = nonlinearDC;

				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}

				list<CBaseElement*> NewProvList;
				CBaseElement *NewProv = pFactory->CreateElement(prov);
				NewProv->m_InputElement.push_back((*elementpos));
				NewProvList.push_back(NewProv);
				m_TempList.push_back(NewProv);

				list<CBaseElement*>::iterator provpos = Provlist.begin();
				while (provpos != Provlist.end()) {
					NewProvList.push_back((*provpos));
					++provpos;
				}

				NonLinearDCFormula Converter;
				if (Converter.Processing(NodeList, DataList, NewProvList) == FALSE) {
					AfxMessageBox(_T("변환회로가 정상적이지 않습니다"));	
					return FALSE;
				}

				list<IPCData>::iterator datapos = DataList.begin();
				while (datapos != DataList.end()) {
					if ((*datapos).ProvNumber != NewProv->m_ElementNumber) {
						LeftRealDatalist.push_back((*datapos));
					}
					++datapos;
				}
			}

			CCircultSimulation::m_CircultCase = TempCae;
			//비반전, 반전 증폭기 특성으로 계산
			list<ElementData> GetList;
			(*pos)->GetProperties(GetList);
			list<IPCData>::iterator resultpos = DataList.begin();

			double MaxVolatage = 0, MinVoltage = 0;
			list<ElementData>::iterator getpos = GetList.begin();
			while (getpos != GetList.end()) {
				if ((*getpos).Text == CString(_T("GainVoltagerate"))) {
					InputVoltage = (*resultpos).GrapehValue[0] * (*getpos).Value;
				}
				else if((*getpos).Text == CString(_T("MaxVoltage"))) {
					MaxVolatage = (*getpos).Value;
				}
				else if((*getpos).Text == CString(_T("MinVoltage"))) {
					MinVoltage = (*getpos).Value;
				}
				++getpos;
			}

			if (InputVoltage > MaxVolatage) InputVoltage = MaxVolatage;
			if (InputVoltage < MinVoltage) InputVoltage = MinVoltage;
		}
	}

	return TRUE;
}

CBaseElement* OpampDCFormula::GetElementGroup(int Collectiontype, double inputVoltage, CBaseElement *pOPampElement)
{
	CElementFactory *pFactory = CMainFrame::CreateElementFactory();
	switch (Collectiontype) {
	case dcpowercollection:
		{	
			CBaseElement *NewPower = pFactory->CreateElement(dcpower);
			CBaseElement *NewLine = pFactory->CreateElement(lines);
			CBaseElement *NewGround = pFactory->CreateElement(ground);

			NewPower->m_ConnectElement.push_back(NewLine);
			NewLine->m_ConnectElement.push_back(NewPower);
			NewLine->m_ConnectElement.push_back(NewGround);
			NewGround->m_ConnectElement.push_back(NewLine);

			m_TempList.push_back(NewPower);
			m_TempList.push_back(NewLine);
			m_TempList.push_back(NewGround);

			m_TempCircuitList.push_back(NewPower);
			m_TempCircuitList.push_back(NewLine);
			m_TempCircuitList.push_back(NewGround);

			list<ElementData> SendList;
			ElementData NewData;
			NewData.Text = CString(_T("Voltage"));
			NewData.Value = inputVoltage;
			SendList.push_back(NewData);
			NewPower->OnEditProperties(&SendList);

			//position이동 시작점으로 진행방향하기 때문에 
			//opamp의 오른쪽 시작점에 생성된 전압의 시작점이 위치해야 

			list<CPoint>::iterator connectpos = pOPampElement->m_ConnectPoint.begin();
			CPoint ConvertPosition = pOPampElement->m_Position + (*connectpos);

			list<CPoint>::iterator PowerConnectpos = NewPower->m_ConnectPoint.begin();
			ConvertPosition -= (*PowerConnectpos);
			NewPower->m_Position = ConvertPosition;

			return NewPower;
		}
		break;

	case resistancecollection:
		{
			CBaseElement *NewResistance = pFactory->CreateElement(resistance);
			CBaseElement *NewLine = pFactory->CreateElement(lines);
			CBaseElement *NewGround = pFactory->CreateElement(ground);

			NewResistance->m_ConnectElement.push_back(NewLine);
			NewLine->m_ConnectElement.push_back(NewResistance);
			NewLine->m_ConnectElement.push_back(NewGround);
			NewGround->m_ConnectElement.push_back(NewLine);

			m_TempList.push_back(NewResistance);
			m_TempList.push_back(NewLine);
			m_TempList.push_back(NewGround);

			m_TempCircuitList.push_back(NewResistance);
			m_TempCircuitList.push_back(NewLine);
			m_TempCircuitList.push_back(NewGround);


			//opamp leftresistance 저항값 획득
			list<ElementData> GetList;
			pOPampElement->GetProperties(GetList);

			double LeftResistanceVlaue = 0;
			list<ElementData>::iterator getpos = GetList.begin();
			while (getpos != GetList.end()) {
				if ((*getpos).Text == CString(_T("LeftResistance"))) {
					LeftResistanceVlaue = (*getpos).Value;
					break;
				}
				++getpos;
			}

			ElementData NewData;
			NewData.Text = CString(_T("Resistance"));
			NewData.Value = LeftResistanceVlaue;
			list<ElementData> SendList;
			SendList.push_back(NewData);
			NewResistance->OnEditProperties(&SendList);

			return NewResistance;
		}
		break;

	default:
		break;
	}

	return NULL;
}

void OpampDCFormula::RecursiveCircuitSearch(CBaseElement* pElement)
{
	list<CBaseElement*> RecusiveList;
	list<CBaseElement*>::iterator pos = pElement->m_ConnectElement.begin();
	while (pos != pElement->m_ConnectElement.end()) {
	
		if ((*pos)->m_Flag != InvertingAmplifier && (*pos)->m_Flag != NonInvertingAmplifier) {
			//중복 아닐때 수행
			list<CBaseElement*>::iterator Duplicatepos = find(m_TempCircuitList.begin(), m_TempCircuitList.end(), (*pos));
			if (Duplicatepos == m_TempCircuitList.end()) {
				RecusiveList.push_back((*pos));
				m_TempCircuitList.push_back((*pos));
			}
		}
		++pos;
	}

	list<CBaseElement*>::iterator Recusivepos = RecusiveList.begin();
	while (Recusivepos != RecusiveList.end()) {
		RecursiveCircuitSearch((*Recusivepos));
		++Recusivepos;
	}
}

