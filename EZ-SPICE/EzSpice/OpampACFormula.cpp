#include "StdAfx.h"
#include "OpampACFormula.h"
#include "CircultSimulation.h"
#include "MainFrm.h"
#include "ElementFactory.h"
#include "LinearACFormula.h"
#include "NonLinearDCFormula.h"
#include "NonLinearACFormula.h"
#include "NodePretreatment.h"
#include <algorithm>

OpampACFormula::OpampACFormula(void)
{
}

OpampACFormula::~OpampACFormula(void)
{
}

void OpampACFormula::AheadProcessing( list<IPCData> &Datalist )
{

}

void OpampACFormula::RearProcessing( list<IPCData> &Datalist )
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

	//��������� ���ؼ� �ѹ��� ���������� ����

	CCircultSimulation::m_CircultCase = linearDC;

	list<Node> NodeList;
	NodePretreatment NewPretreatment;
	if (NewPretreatment.Processing(CCircultSimulation::m_CircuitList, NodeList) == FALSE) {
		AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
	}

	//���� ������ ��� ���� ����
	list<CBaseElement*>::iterator deletepos = m_TempList.begin();
	while (deletepos != m_TempList.end()) {
		delete (*deletepos);
		++deletepos;
	}
	m_TempList.clear();
}

void OpampACFormula::Initialize()
{
	//���� �� ���� ������ ����
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

BOOL OpampACFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist )
{
	//Op-amp�� ��� Op-amp�� �߽����� �¿� ��带 ���������� �����ϰ� �ȴ�.
	//�ٸ� Circuit�� ��� ��带 �����ϰ� ������ �� ��� �¿� ȸ�ο� ���� �����Ͽ� �� �������� �����Ѵ�,
	//������ ȸ���� ��� Op-amp������ ������ ���ϴ� ��Ȱ�� �ϰ� �Ǹ�
	//������ ȸ���� ��� ���� ������ ������ Op-amp������ ���� �������� �ٽ� ȸ�ΰ� ���۵ȴ�.
	//�� �������� �������� ���������� üũ�Ͽ� ��ư ���չ��� �����ؾ� ���� �����ؾ� �Ѵ�.

	//copy circuit
	Initialize();

	//left circuit, right circult  (between opamp)
	
	list<IPCData> LeftCircuitDataList;
	list<IPCData> LeftRealDataList;
	if (DivideLeftCircuit(LeftCircuitDataList, LeftRealDataList, Provlist) == FALSE) {
		RearProcessing(Datalist);
		return FALSE;
	}
	
	if (DivideRightCircuit(LeftCircuitDataList, Datalist, Provlist) == FALSE) {
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

int OpampACFormula::CheckCircuit( list<CBaseElement*> &CurrentList )
{
	list<CBaseElement*>::iterator pos = CurrentList.begin();
	while (pos != CurrentList.end()) {
		if ((*pos)->m_Flag == transistor || (*pos)->m_Flag == diode) {
			return nonlinearAC;
		}
		++pos;
	}
	return linearAC;
}

BOOL OpampACFormula::DivideRightCircuit(list<IPCData> LeftCircuitDataList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	m_TempCircuitList.clear();

	//op-amp Ž��
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		if ((*pos)->m_Flag == InvertingAmplifier || (*pos)->m_Flag == NonInvertingAmplifier) {
			break;
		}
		++pos;
	}

	if (pos != m_CircuitList.end()) {

		//���� 1. opamp�� ��� ȸ���� �̷�� ���� �ʱ� ������ ù��° ConnectPoint�� ����Ȱ��� �������̴�.

		list<CPoint>::iterator LeftPoint = (*pos)->m_ConnectPoint.begin();

		//������ ó�� ���� �˻�
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

			//���޵� inputvolatage�� ȸ�� ����
			//opamp��ġ�� ����+����+�׶��� ����

			list<IPCData>::iterator inputdatapos = LeftCircuitDataList.begin();
			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			CBaseElement* pPower = GetElementGroup(acpowercollection, (*inputdatapos).GrapehValue[0], (*pos));

			(*elementpos)->DeleteElement((*pos));
			(*elementpos)->m_ConnectElement.push_back(pPower);
			pPower->m_ConnectElement.push_back((*elementpos));

			int TempCae = CCircultSimulation::m_CircultCase;
			if (CheckCircuit(m_TempCircuitList) == linearAC) {

				CCircultSimulation::m_CircultCase = linearAC;

				list<Node> NodeList;
				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
					return FALSE;
				}

				LinearACFormula Converter;
				if (Converter.OPAmpProcessing(LeftCircuitDataList, NodeList, Datalist, Provlist) == FALSE) {
					AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
					return FALSE;
				}
			}
			else { //����
				if (CCircultSimulation::m_RLCCase == rr) {
					CCircultSimulation::m_CircultCase = nonlinearAC;

					list<Node> NodeList;
					NodePretreatment NewPretreatment;
					if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
						AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
						return FALSE;
					}

					NonLinearACFormula Converter;
					if (Converter.OPAmpProcessing(LeftCircuitDataList, NodeList, Datalist, Provlist) == FALSE) {
						AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
						return FALSE;
					}
				}
				else {
					AfxMessageBox(_T("���� AC �ؼ��� OP-Amp, ���̿���, ���׸� �����մϴ�."));
					return FALSE;
				}
			}
			CCircultSimulation::m_CircultCase = TempCae;
		}
	}
	return TRUE;
}



BOOL OpampACFormula::DivideLeftCircuit(list<IPCData> &LeftCircuitDataList, list<IPCData> &LeftRealDatalist, list<CBaseElement*> &Provlist)	//���� �߿�
{
	m_TempCircuitList.clear();

	//op-amp Ž��
	list<CBaseElement*>::iterator pos = m_CircuitList.begin();
	while (pos != m_CircuitList.end()) {
		if ((*pos)->m_Flag == InvertingAmplifier || (*pos)->m_Flag == NonInvertingAmplifier) {
			break;
		}
		++pos;
	}

	if (pos != m_CircuitList.end()) {

		//���� 1. opamp�� ��� ȸ���� �̷�� ���� �ʱ� ������ �ι�° ConnectPoint�� ����Ȱ��� �����̴�.

		list<CPoint>::iterator LeftPoint = (*pos)->m_ConnectPoint.begin();
		LeftPoint++;

		//���� ó�� ���� �˻�
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
			RecursiveCircuitSearch((*elementpos));	//ȸ�� Ž���Ͽ� m_TempCircuitList�� ����		

			//(*elementpos�� opamp������ ���� ���� (*pos)�� opamp
			//opamp ���� ���� (���� + ���� + �׶���)
			CBaseElement* pResistance = GetElementGroup(resistancecollection, -1, (*pos));
			if (pResistance == NULL) {
				return FALSE;
			}

			//opamp���� �� ���� ���� üũ (���� ���ο� ���κ긦 �ޱ� ����)
			(*elementpos)->DeleteElement((*pos));
			(*elementpos)->m_ConnectElement.push_back(pResistance);
			pResistance->m_ConnectElement.push_back((*elementpos));

			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			list<Node> NodeList;
			int TempCae = CCircultSimulation::m_CircultCase;

			//���� ȸ�ΰ� �������� ���� ���� üũ
			if (CheckCircuit(m_TempCircuitList) == linearAC) {

				CCircultSimulation::m_CircultCase = linearAC;

				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
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

				LinearACFormula Converter;
				if (Converter.Processing(NodeList, LeftCircuitDataList, NewProvList) == FALSE) {
					AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
					return FALSE;
				}

				list<IPCData>::iterator datapos = LeftCircuitDataList.begin();
				while (datapos != LeftCircuitDataList.end()) {
					if ((*datapos).ProvNumber != NewProv->m_ElementNumber) {
						LeftRealDatalist.push_back((*datapos));
					}
					++datapos;
				}
			}
			else {	//����
				

				//�����϶��� CCircultSimulation::m_CircuitList �� ���� ä���� nonlinearAC�� �����Ѵ�.
				if (CCircultSimulation::m_RLCCase == rr) {

					CCircultSimulation::m_CircultCase = nonlinearAC;

					NodePretreatment NewPretreatment;
					if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
						AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
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

					list<CBaseElement*> BackupList;
					BackupList.clear();
					BackupList.resize(CCircultSimulation::m_CircuitList.size());
					copy(CCircultSimulation::m_CircuitList.begin(), CCircultSimulation::m_CircuitList.end(), BackupList.begin());

					CCircultSimulation::m_CircuitList.clear();
					CCircultSimulation::m_CircuitList.resize(m_TempCircuitList.size());
					copy(m_TempCircuitList.begin(), m_TempCircuitList.end(), CCircultSimulation::m_CircuitList.begin());
					
					NonLinearACFormula Converter;
					if (Converter.Processing(NodeList, LeftCircuitDataList, NewProvList) == FALSE) {
						AfxMessageBox(_T("ȸ�ΰ� ���������� �ʽ��ϴ�"));	
						return FALSE;
					}

					list<IPCData>::iterator datapos = LeftCircuitDataList.begin();
					while (datapos != LeftCircuitDataList.end()) {
						if ((*datapos).ProvNumber != NewProv->m_ElementNumber) {
							LeftRealDatalist.push_back((*datapos));
						}
						++datapos;
					}

					CCircultSimulation::m_CircuitList.clear();
					CCircultSimulation::m_CircuitList.resize(BackupList.size());
					copy(BackupList.begin(), BackupList.end(), CCircultSimulation::m_CircuitList.begin());
				}
				else {
					AfxMessageBox(_T("���� AC �ؼ��� OP-Amp, ���̿���, ���׸� �����մϴ�."));
					return FALSE;
				}
			}

			CCircultSimulation::m_CircultCase = TempCae;
			//�����, ���� ������ Ư������ ���
			list<ElementData> GetList;
			(*pos)->GetProperties(GetList);
			list<IPCData>::iterator resultpos = LeftCircuitDataList.begin();

			double GainVoltagerate = 0;
			double MaxVolatage = 0, MinVoltage = 0;
			list<ElementData>::iterator getpos = GetList.begin();
			while (getpos != GetList.end()) {
				if ((*getpos).Text == CString(_T("GainVoltagerate"))) {
					GainVoltagerate = (*getpos).Value;
				}
				else if((*getpos).Text == CString(_T("MaxVoltage"))) {
					MaxVolatage = (*getpos).Value;
				}
				else if((*getpos).Text == CString(_T("MinVoltage"))) {
					MinVoltage = (*getpos).Value;
				}
				++getpos;
			}

			for (int iCount = 0; iCount < DivideSector; ++iCount) {
				(*resultpos).GrapehValue[iCount] *= GainVoltagerate;
				if ((*resultpos).GrapehValue[iCount] > MaxVolatage) (*resultpos).GrapehValue[iCount] = MaxVolatage;
				if ((*resultpos).GrapehValue[iCount] < MinVoltage) (*resultpos).GrapehValue[iCount] = MinVoltage;
			}
		}
	}
	return TRUE;
}

CBaseElement* OpampACFormula::GetElementGroup(int Collectiontype, double inputVoltage, CBaseElement *pOPampElement)
{
	CElementFactory *pFactory = CMainFrame::CreateElementFactory();
	switch (Collectiontype) {
	case acpowercollection:
		{	
			CBaseElement *NewPower = pFactory->CreateElement(acpower);
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

			//position�̵� ���������� ��������ϱ� ������ 
			//opamp�� ������ �������� ������ ������ �������� ��ġ�ؾ� 

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


			//opamp leftresistance ���װ� ȹ��
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

void OpampACFormula::RecursiveCircuitSearch(CBaseElement* pElement)
{
	list<CBaseElement*> RecusiveList;
	list<CBaseElement*>::iterator pos = pElement->m_ConnectElement.begin();
	while (pos != pElement->m_ConnectElement.end()) {

		if ((*pos)->m_Flag != InvertingAmplifier && (*pos)->m_Flag != NonInvertingAmplifier) {
			//�ߺ� �ƴҶ� ����
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


