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

BOOL OpampDCFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist )
{
	//Op-amp�� ��� Op-amp�� �߽����� �¿� ��带 ���������� �����ϰ� �ȴ�.
	//�ٸ� Circuit�� ��� ��带 �����ϰ� ������ �� ��� �¿� ȸ�ο� ���� �����Ͽ� �� �������� �����Ѵ�,
	//������ ȸ���� ��� Op-amp������ ������ ���ϴ� ��Ȱ�� �ϰ� �Ǹ�
	//������ ȸ���� ��� ���� ������ ������ Op-amp������ ���� �������� �ٽ� ȸ�ΰ� ���۵ȴ�.
	//�� �������� �������� ���������� üũ�Ͽ� ��ư ���չ��� �����ؾ� ���� �����ؾ� �Ѵ�.

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
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
					return FALSE;
				}

				LinearDCFormula Converter;
				if (Converter.Processing(NodeList, Datalist, Provlist) == FALSE) {
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
					return FALSE;
				}
			}
			else { //����

				CCircultSimulation::m_CircultCase = nonlinearDC;

				list<Node> NodeList;
				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
					return FALSE;
				}

				NonLinearDCFormula Converter;
				if (Converter.Processing(NodeList, Datalist, Provlist) == FALSE) {
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
					return FALSE;
				}
			}
			CCircultSimulation::m_CircultCase = TempCae;
		}
	}
	return TRUE;
}



BOOL OpampDCFormula::DivideLeftCircuit(double &InputVoltage, list<IPCData> &LeftRealDatalist, list<CBaseElement*> &Provlist)	//���� �߿�
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
			CBaseElement* pResistance = GetElementGroup(resistancecollection, InputVoltage, (*pos));
			if (pResistance == NULL) {
				return FALSE;
			}

			//opamp���� �� ���� ���� üũ (���� ���ο� ���κ긦 �ޱ� ����)
			(*elementpos)->DeleteElement((*pos));
			(*elementpos)->m_ConnectElement.push_back(pResistance);
			pResistance->m_ConnectElement.push_back((*elementpos));

			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			list<Node> NodeList;
			list<IPCData> DataList;
			int TempCae = CCircultSimulation::m_CircultCase;

			//���� ȸ�ΰ� �������� ���� ���� üũ
			if (CheckCircuit(m_TempCircuitList) == linearDC) {
				
				CCircultSimulation::m_CircultCase = linearDC;

				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
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
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
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
			else {	//����
				CCircultSimulation::m_CircultCase = nonlinearDC;

				NodePretreatment NewPretreatment;
				if (NewPretreatment.Processing(m_TempCircuitList, NodeList) == FALSE) {
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
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
					AfxMessageBox(_T("��ȯȸ�ΰ� ���������� �ʽ��ϴ�"));	
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
			//�����, ���� ������ Ư������ ���
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

void OpampDCFormula::RecursiveCircuitSearch(CBaseElement* pElement)
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

