#include "StdAfx.h"
#include "NonLinearACFormula.h"
#include "BaseElement.h"
#include <algorithm>
#include <math.h>
#include "MainFrm.h"
#include "ElementFactory.h"
#include "NodePretreatment.h"
#include "LinearDCFormula.h"
#include "NonLinearDCFormula.h"
#include "CircultSimulation.h"



NonLinearACFormula::NonLinearACFormula(void)
{
	m_StencilBuffer = NULL;
	m_Result = NULL;
	m_TempACPower = NULL;
	m_TempResistance = NULL;
}

NonLinearACFormula::~NonLinearACFormula(void)
{
	if (m_StencilBuffer != NULL) {
		delete m_StencilBuffer[0];
		delete m_StencilBuffer;
	}

	if (m_Result != NULL) {
		delete m_Result;
	}

	if (m_TempResistance != NULL) {
		delete m_TempResistance;
	}

	//TempList ����
	list<CBaseElement*>::iterator pos = m_TempList.begin();
	while (pos != m_TempList.end()) {
		delete (*pos);
		++pos;
	}
}


void NonLinearACFormula::RearProcessing(list<IPCData> &Datalist)	//��ó��
{
	//��ó�� (���� RLC ���� �̵����� ���� MaxVolate ������ ��ȯ)
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	double MaxVoltage = pFrame->m_VoltMax;

	list<IPCData>::iterator pos = Datalist.begin();
	while (pos != Datalist.end()) {

		for (int iCount = 0; iCount < DivideSector; iCount++) {
			if ((*pos).GrapehValue[iCount] > MaxVoltage) {
				MaxVoltage = (*pos).GrapehValue[iCount];
			}
		}
		++pos;
	}

	if (MaxVoltage > (int)MaxVoltage) {
		pFrame->m_VoltMax = (int)MaxVoltage + 1;
	}
	else pFrame->m_VoltMax = (int)MaxVoltage;


	//�ﰢ�� ���ý� (������ -> �ﰢ��)
	if (CCircultSimulation::m_ACData.WaveModel == triangular) {
		//�����ʹ� 3600�� 
		list<IPCData>::iterator pos = Datalist.begin();
		while (pos != Datalist.end()) {

			//ù������ ���� ���� ���� ���� ����
			BOOL UpDownFlag = FALSE;
			(*pos).GrapehValue[0] < (*pos).GrapehValue[1] ? UpDownFlag = TRUE : UpDownFlag = FALSE;

			int LineToCount = 0;
			double LineTopos = (*pos).GrapehValue[0];
			double MoveTopos = (*pos).GrapehValue[1];
			for (int iCount = 1; iCount < DivideSector; ++iCount) {

				MoveTopos = (*pos).GrapehValue[iCount];
				if (UpDownFlag) {	//������ ����
					if (LineTopos > MoveTopos || iCount == DivideSector -1) {	//�ѹ� ���̴� ����
						//����
						double Slope = ((*pos).GrapehValue[iCount] - (*pos).GrapehValue[LineToCount])/(iCount - LineToCount);
						double PlusValue = (*pos).GrapehValue[iCount] - Slope*iCount;

						for (int jCount = LineToCount; jCount < iCount; jCount++) {
							(*pos).GrapehValue[jCount] = Slope*jCount + PlusValue;
						}
						LineToCount = iCount;
						UpDownFlag = FALSE;
					}
				}		
				else {	//������ ����
					if (LineTopos < MoveTopos || iCount == DivideSector -1) {	//�ѹ� ���̴� ����

						double Slope = ((*pos).GrapehValue[iCount] - (*pos).GrapehValue[LineToCount])/(iCount - LineToCount);
						double PlusValue = (*pos).GrapehValue[iCount] - Slope*iCount;

						for (int jCount = LineToCount; jCount < iCount; jCount++) {
							(*pos).GrapehValue[jCount] = Slope*jCount + PlusValue;
						}

						LineToCount = iCount;
						UpDownFlag = TRUE;
					}
				}
				LineTopos = MoveTopos;
			}
			++pos;
		}
	}
}

BOOL NonLinearACFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{

	//copy
	m_NodeList.clear();
	m_NodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_NodeList.begin());

	m_ProvList.clear();
	m_ProvList.resize(Provlist.size());
	copy(Provlist.begin(), Provlist.end(), m_ProvList.begin());

	//Initialize and New IPCData
	Initialize(Datalist, m_ProvList);


	//���� �������� ����
	list<CBaseElement*>::iterator pos = CCircultSimulation::m_CircuitList.begin();
	while (pos != CCircultSimulation::m_CircuitList.end()) {

		RecoveryConnectElment NewConnect;
		NewConnect.pElement = (*pos);

		NewConnect.ConnectElement.clear();
		NewConnect.ConnectElement.resize((*pos)->m_ConnectElement.size());
		copy((*pos)->m_ConnectElement.begin(), (*pos)->m_ConnectElement.end(), NewConnect.ConnectElement.begin());

		m_CircuitRecoveryList.push_back(NewConnect);
		m_ReconstructionList.push_back((*pos));
		++pos;
	}

	//AC->DC�������� ��ü
	list<CBaseElement*>::iterator Reconpos = m_ReconstructionList.begin();
	while (Reconpos != m_ReconstructionList.end()) {
		if ((*Reconpos)->m_Flag == acpower) {
			break;
		}
		++Reconpos;
	}

	CBaseElement *NewElement = NULL;
	if (Reconpos != m_ReconstructionList.end()) {

		CElementFactory *pFactory = CMainFrame::CreateElementFactory();
		NewElement = pFactory->CreateElement(dcpower);
		NewElement->m_Position = (*Reconpos)->m_Position;		//��ġ�� ����

		list<CBaseElement*>::iterator pos = (*Reconpos)->m_ConnectElement.begin();
		while (pos != (*Reconpos)->m_ConnectElement.end()) {	
			(*pos)->DeleteElement((*Reconpos));
			(*pos)->m_ConnectElement.push_back(NewElement);
			NewElement->m_ConnectElement.push_back((*pos));
			++pos;
		}
		m_ReconstructionList.erase(Reconpos);
		m_ReconstructionList.push_back(NewElement);
	}
	
	//��� ����

	int TempCase = CCircultSimulation::m_CircultCase;
	CCircultSimulation::m_CircultCase = nonlinearDC;

	NodePretreatment NodeConverter;
	list<Node> ReconNodeList;
	if (NodeConverter.Processing(m_ReconstructionList, ReconNodeList) == FALSE) {
		return FALSE;
	}

	if (CCircultSimulation::m_ACData.Amplitude != -1) {	//���� ������ �´� -1�� ���´�.

		double fluidFrequency = (1/CCircultSimulation::m_ACData.Frequency)/DivideSector;
		for (double iCount = 0; iCount < DivideSector; iCount++) {

			double fluidVoltage = CCircultSimulation::m_ACData.Amplitude*sin(2*PIE*CCircultSimulation::m_ACData.Frequency*(fluidFrequency*iCount));		

			if (fluidVoltage < 0.01 && fluidVoltage > -0.01) {
				list<IPCData>::iterator ipcpos = Datalist.begin();
				while (ipcpos != Datalist.end()) {

					(*ipcpos).GrapehValue[(int)iCount] = 0;
					++ipcpos;
				}
			}
			else {
				list<ElementData> SendList;
				ElementData NewVoltage;
				NewVoltage.Text = CString(_T("Voltage"));
				NewVoltage.Value = fluidVoltage;
				SendList.push_back(NewVoltage);
				NewElement->OnEditProperties(&SendList);

				NonLinearDCFormula NewFormula;
				list<IPCData> NewDataList;
				if (NewFormula.Processing(ReconNodeList, NewDataList, Provlist) == FALSE) {
					break;
				}

				list<IPCData>::iterator ipcpos = Datalist.begin();
				while (ipcpos != Datalist.end()) {

					SearchIPCData NewSearch;
					NewSearch.ProvNumber = (*ipcpos).ProvNumber;

					list<IPCData>::iterator searchpos = find_if(NewDataList.begin(), NewDataList.end(), NewSearch);
					if (searchpos != NewDataList.end()) {
						(*ipcpos).GrapehValue[(int)iCount] = (*searchpos).GrapehValue[0];
					}
					++ipcpos;
				}
			}
			//��� Operatorpoint ����
			//RecoveryNode(ReconNodeList);
		}
	}

	CCircultSimulation::m_CircultCase = TempCase;
	//�ﰢ�� ���ý� (������ -> �ﰢ��)
	RearProcessing(Datalist);


	//ȸ�� ����
	list<CBaseElement*>::iterator deletepos = NewElement->m_ConnectElement.begin();
	while (deletepos != NewElement->m_ConnectElement.end()) {
		(*deletepos)->DeleteElement(NewElement);
		++deletepos;
	}

	list<RecoveryConnectElment>::iterator recoverpos = m_CircuitRecoveryList.begin();
	while (recoverpos != m_CircuitRecoveryList.end()) {
		list<CBaseElement*>::iterator searchpos = find(CCircultSimulation::m_CircuitList.begin(), CCircultSimulation::m_CircuitList.end(), (*recoverpos).pElement);
		if (searchpos != CCircultSimulation::m_CircuitList.end()) {

			(*searchpos)->m_ConnectElement.clear();
			(*searchpos)->m_ConnectElement.resize((*recoverpos).ConnectElement.size());
			copy((*recoverpos).ConnectElement.begin(), (*recoverpos).ConnectElement.end(), (*searchpos)->m_ConnectElement.begin());

			(*searchpos)->m_InputElement.clear();
			(*searchpos)->m_OutputElement.clear();
		}
		++recoverpos;
	}

	//��������� ���� �ѹ��� ����
	{
		CCircultSimulation::m_CircultCase = linearAC;
		NodePretreatment NodeConverter;
		list<Node> ReconNodeList;
		if (NodeConverter.Processing(CCircultSimulation::m_CircuitList, ReconNodeList) == FALSE) {
			return FALSE;
		}
	}
	return TRUE;
}

void NonLinearACFormula::RecoveryNode(list<Node> &CurrentList)
{
	list<ElementData> SendList;
	ElementData ResetData;
	ResetData.Text = _T("OperatingPoint");
	ResetData.Value = 0.9;
	SendList.push_back(ResetData);

	list<Node>::iterator pos = CurrentList.begin();
	while (pos != CurrentList.end()) {
	
		list<CBaseElement*>::iterator lowpos = (*pos).AllElement.begin();
		while (lowpos != (*pos).AllElement.end()) {
		
			if ((*lowpos)->m_Flag == geq || (*lowpos)->m_Flag == leq) {
				(*lowpos)->OnEditProperties(&SendList);
			}
			++lowpos;
		}
		++pos;
	}
	
}

double NonLinearACFormula::VoltageDropping()
{
	//AC���п��� DC���п����� ��ü �� ���� DC �˰��� �����Ͽ� ĳ�н��� ����ġ�� ����üũ
	//ĳ�н��� ���� ���ο� ���κ� ���� 

	//������ ��� ���� ���� ���� (���������� �޶����Ƿ� - ���� DC �ѹ� ������ �Ʒ�����)
	list<RecoveryProgress> CircuitRecoveryList;
	list<CBaseElement*>::iterator circuitpos = CCircultSimulation::m_CircuitList.begin();
	while (circuitpos != CCircultSimulation::m_CircuitList.end()) {

		RecoveryProgress NewProgress;
		NewProgress.pElement = (*circuitpos);

		NewProgress.InputElement.clear();
		NewProgress.InputElement.resize((*circuitpos)->m_InputElement.size());
		copy((*circuitpos)->m_InputElement.begin(), (*circuitpos)->m_InputElement.end(), NewProgress.InputElement.begin());

		NewProgress.OutputElement.clear();
		NewProgress.OutputElement.resize((*circuitpos)->m_OutputElement.size());
		copy((*circuitpos)->m_OutputElement.begin(), (*circuitpos)->m_OutputElement.end(), NewProgress.OutputElement.begin());

		CircuitRecoveryList.push_back(NewProgress);
		++circuitpos;
	}

	list<CBaseElement*> NewCircuit;
	list<CBaseElement*> NewProvList;

	//AC���� -> DC ���� �� ȸ�� �ʱ� ���� ��ȯ
	NewCircuit.clear();
	NewCircuit.resize(CCircultSimulation::m_CircuitList.size());
	copy(CCircultSimulation::m_CircuitList.begin(), CCircultSimulation::m_CircuitList.end(), NewCircuit.begin());

	CBaseElement *pACPowerElement = NULL;
	CBaseElement *pDCPowerElement = NULL;
	list<CBaseElement*>::iterator elementpos = NewCircuit.begin();
	while (elementpos != NewCircuit.end()) {

		if ((*elementpos)->m_Flag == acpower) {	//AC->DC

			pACPowerElement = (*elementpos);
			double Amplitude = GetElementProperty(pACPowerElement, CString(_T("Amplitude")));

			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			CBaseElement *NewElement = pFactory->CreateElement(dcpower);

			pDCPowerElement = NewElement;
			NewElement->m_Position = pACPowerElement->m_Position;
			list<ElementData> SendList;
			ElementData NewVoltage;
			NewVoltage.Text = CString(_T("Voltage"));
			NewVoltage.Value = Amplitude;

			SendList.push_back(NewVoltage);
			NewElement->OnEditProperties(&SendList);

			NewElement->m_ConnectElement.clear();
			NewElement->m_ConnectElement.resize((*elementpos)->m_ConnectElement.size());
			copy((*elementpos)->m_ConnectElement.begin(), (*elementpos)->m_ConnectElement.end(), NewElement->m_ConnectElement.begin());

			list<CBaseElement*>::iterator insertpos = NewElement->m_ConnectElement.begin();
			while (insertpos != NewElement->m_ConnectElement.end()) {
				(*insertpos)->m_ConnectElement.push_back(NewElement);
				++insertpos;
			}

			NewCircuit.push_back(NewElement);
		}
		else if ((*elementpos)->m_Flag == capacity) {//���ο� ���κ� ����(ĳ�н��� �յ�)

			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			list<CBaseElement*>::iterator capacitypos = (*elementpos)->m_OutputElement.begin();
			while (capacitypos != (*elementpos)->m_OutputElement.end()) {

				CBaseElement *NewElement = pFactory->CreateElement(prov);
				NewElement->m_InputElement.push_back((*capacitypos));
				NewProvList.push_back(NewElement);
				++capacitypos;
			}
		}
		++elementpos;
	}

	list<CBaseElement*> RecoveryList;
	if (pACPowerElement != NULL) {

		list<CBaseElement*>::iterator deletepos = pACPowerElement->m_ConnectElement.begin();
		while (deletepos != pACPowerElement->m_ConnectElement.end()) {

			RecoveryList.push_back((*deletepos));
			(*deletepos)->m_ConnectElement.remove(pACPowerElement);
			++deletepos;
		}

		NewCircuit.remove(pACPowerElement);		
	}

	CCircultSimulation::m_CircultCase = linearDC;

	list<Node> NewNodeList;
	NodePretreatment NewPretratment;
	if (NewPretratment.Processing(NewCircuit, NewNodeList) == FALSE) {
		AfxMessageBox(_T("���������� ��ȭ���ּ���!!!"));
		return -1;
	}


	list<IPCData> NewDataList;
	LinearDCFormula NewFormula;
	if (NewFormula.Processing(NewNodeList, NewDataList, NewProvList) == FALSE) {
		AfxMessageBox(_T("���������� ��ȭ���ּ���!!!"));
		return -1;
	}

	// acpower �����ϸ鼭 ������ ���ο� �ٽ� ����
	CCircultSimulation::m_CircultCase = linearAC;
	list<CBaseElement*>::iterator recoverypos = RecoveryList.begin();
	while (recoverypos != RecoveryList.end()) {
		(*recoverypos)->m_ConnectElement.push_back(pACPowerElement);
		(*recoverypos)->m_ConnectElement.remove(pDCPowerElement);
		++recoverypos;
	}

	if (pDCPowerElement != NULL) {
		delete pDCPowerElement;
	}

	//���� ����������� ����
	list<RecoveryProgress>::iterator progresspos = CircuitRecoveryList.begin();
	while (progresspos != CircuitRecoveryList.end()) {

		list<CBaseElement*>::iterator searchpos = find(CCircultSimulation::m_CircuitList.begin(), CCircultSimulation::m_CircuitList.end(), (*progresspos).pElement);
		if (searchpos != CCircultSimulation::m_CircuitList.end()) {

			(*searchpos)->m_InputElement.clear();
			(*searchpos)->m_InputElement.resize((*progresspos).InputElement.size());
			copy((*progresspos).InputElement.begin(), (*progresspos).InputElement.end(), (*searchpos)->m_InputElement.begin());

			(*searchpos)->m_OutputElement.clear();
			(*searchpos)->m_OutputElement.resize((*progresspos).OutputElement.size());
			copy((*progresspos).OutputElement.begin(), (*progresspos).OutputElement.end(), (*searchpos)->m_OutputElement.begin());
		}
		++progresspos;
	}

	//�� ��ȯ
	list<IPCData>::iterator ipcpos = NewDataList.begin();
	while (ipcpos != NewDataList.end()) {

		if ((*ipcpos).GrapehValue[0] != 0) {
			return (*ipcpos).GrapehValue[0];
		}
		++ipcpos;
	}

	return -1;
}

double NonLinearACFormula::GetCapacitance()
{
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		list<CBaseElement*>::iterator lowpos = (*pos).AllElement.begin();
		while (lowpos != (*pos).AllElement.end()) {

			if ((*lowpos)->m_Flag == capacity) {

				list<ElementData> GetList;
				(*lowpos)->GetProperties(GetList);

				list<ElementData>::iterator datapos = GetList.begin();
				while (datapos != GetList.end()) {

					if ((*datapos).Text == CString(_T("Capacitance"))) {
						return (*datapos).Value;
					}
					++datapos;
				}
			}
			++lowpos;
		}
		++pos;
	}
	return -1;
}

void NonLinearACFormula::RestitutionList()
{
	m_NodeList.clear();
	m_NodeList.resize(m_TempNodeList.size());
	copy(m_TempNodeList.begin(), m_TempNodeList.end(), m_NodeList.begin());

	m_ProvList.clear();
	m_ProvList.resize(m_TempProvList.size());
	copy(m_TempProvList.begin(), m_TempProvList.end(), m_ProvList.begin());
}

void NonLinearACFormula::SectionCompositonImpedance(list<IPCData> &Datalist)
{
	//CompositionResistance ���κ� ���� ������ �ռ����� ���ϰ� ����
	UINT RepetitionCount = m_ProvList.size();
	for (UINT iCount = 0; iCount < RepetitionCount; iCount++) {

		//�ش� �� �ʱ�ȭ
		m_NodeList.clear();
		m_NodeList.resize(m_TempNodeList.size());
		copy(m_TempNodeList.begin(), m_TempNodeList.end(), m_NodeList.begin());

		int ProvCount = 0;
		list<CBaseElement*>::iterator provpos = m_TempProvList.begin();
		while (provpos != m_TempProvList.end()) {
			if (ProvCount == iCount) {
				m_ProvList.clear();
				m_ProvList.push_back((*provpos));
				break;
			}
			++ProvCount;
			++provpos;
		}

		//�׺�� ������ ���� � ��ȯ
		Thevenintheorem();

		//���κ� ��ġ �ռ� ���Ǵ��� 
		CompostionProvPostion(Datalist);
	}

	//��ü �ռ� ���Ǵ���
	//������ ���ϱ� ���ؼ��� �������� �ռ� ���Ǵ��� �������� ��ü �ռ� ���Ǵ��� ������ ������
	{
		//�ش� �� �ʱ�ȭ
		m_ProvList.clear();
		m_NodeList.clear();
		m_NodeList.resize(m_TempNodeList.size());
		copy(m_TempNodeList.begin(), m_TempNodeList.end(), m_NodeList.begin());

		IPCData NewData;
		NewData.ComplexCheckFlag = FALSE;

		//ù ���� üũ
		list<Node>::iterator pos = m_NodeList.begin();
		while (pos != m_NodeList.end()) {
			if ((*pos).FirstFlag) {
				NewData.NodeNumber = (*pos).NodeNumber;
				break;
			}
			++pos;
		}
		m_TempDataList.push_back(NewData);

		//�׺�� ������ ���� � ��ȯ
		Thevenintheorem();

		//���κ� ��ġ �ռ� ���Ǵ��� 
		CompostionProvPostion(m_TempDataList, TRUE);
	}
}

void NonLinearACFormula::Phasemovement(list<IPCData> &Datalist)
{
	list<IPCData>::iterator FirstNodepos = m_TempDataList.begin();
	double FirstAngle = atan((*FirstNodepos).ComplexNumber.imag()/ (*FirstNodepos).ComplexNumber.real());
	FirstAngle *= RADTOANGLE;

	TRACE("FirstNode Angle : %f\n", FirstAngle);

	list<IPCData>::iterator pos = Datalist.begin();
	while (pos != Datalist.end()) {

		if ((*pos).FirstFlag == FALSE) {	//���� ��带 ������ ������ ���� �̵�	

			double CurrentAngle = atan((*pos).ComplexNumber.imag()/ (*pos).ComplexNumber.real());
			CurrentAngle *= RADTOANGLE;

			if ((*pos).ComplexNumber.real() == 0) { //R ������ 0�϶� ����ó��
				(*pos).ComplexNumber.imag() > 0 ? CurrentAngle = 90 : CurrentAngle = -90;
			}
			TRACE("Current Angle : %f\n", CurrentAngle);

			CurrentAngle -= FirstAngle;		

			TRACE("CurrentAngle - FirstAngle : %f\n", CurrentAngle);

			//Shift
			if (CurrentAngle < 0) {
				CurrentAngle *= MultSector;
				int ShiftCount = abs((int)CurrentAngle);

				double *ShiftData = new double[ShiftCount];
				memset(ShiftData, 0, sizeof(double)*ShiftCount);
				memcpy_s((char*)ShiftData, sizeof(double)*ShiftCount, (char*)(*pos).GrapehValue + sizeof(double)*(DivideSector - ShiftCount), sizeof(double)*ShiftCount);
				memcpy_s((char*)(*pos).GrapehValue + sizeof(double)*ShiftCount, sizeof(double)*(DivideSector - ShiftCount), (char*)(*pos).GrapehValue, sizeof(double)*(DivideSector - ShiftCount));
				memcpy_s((char*)(*pos).GrapehValue, sizeof(double)*ShiftCount, (char*)ShiftData, sizeof(double)*ShiftCount);

				//delete [] ShiftData;
			}
			else if (CurrentAngle > 0) {
				CurrentAngle *= MultSector;
				int ShiftCount = abs((int)CurrentAngle);

				double *ShiftData = new double[ShiftCount];
				memset(ShiftData, 0, sizeof(double)*ShiftCount);
				memcpy_s((char*)ShiftData, sizeof(double)*ShiftCount, (char*)(*pos).GrapehValue, sizeof(double)*ShiftCount);
				memcpy_s((char*)(*pos).GrapehValue, sizeof(double)*(DivideSector - ShiftCount), (char*)(*pos).GrapehValue + sizeof(double)*ShiftCount, sizeof(double)*(DivideSector - ShiftCount));
				memcpy_s((char*)(*pos).GrapehValue + sizeof(double)*(DivideSector - ShiftCount), sizeof(double)*ShiftCount, (char*)ShiftData, sizeof(double)*ShiftCount);

				//delete [] ShiftData;
			}
		}
		++pos;
	}
}


void NonLinearACFormula::CompostionProvPostion(list<IPCData> &Datalist, BOOL nFlag/* = FALSE*/)
{
	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		if (IsProvArea((*nodepos)) || nFlag == TRUE) {
			list<IPCData>::iterator ipcpos = Datalist.begin();
			while (ipcpos != Datalist.end()) {

				if ((*nodepos).NodeNumber == (*ipcpos).NodeNumber && (*ipcpos).ComplexCheckFlag == FALSE) {
					(*ipcpos).ComplexCheckFlag = TRUE;

					if ((*nodepos).OutputElement.size() == 1) {	//���� ����(����)

						list<CBaseElement*>::iterator Elementpos = (*nodepos).OutputElement.begin();
						(*ipcpos).ComplexNumber = GetElementInpedance((*Elementpos));
					}
					else {	//���� ����(����)

						double_complex ComplexNumber(0,0);
						list<CBaseElement*>::iterator Elementpos = (*nodepos).OutputElement.begin();
						while (Elementpos != (*nodepos).OutputElement.end()) {

							if (ComplexNumber.real() != 0 || ComplexNumber.imag() != 0) {
								double_complex AnotheromplexNumber = GetElementInpedance((*Elementpos));
								ComplexNumber = (ComplexNumber*AnotheromplexNumber)/(ComplexNumber+AnotheromplexNumber);
							}
							else {
								ComplexNumber += GetElementInpedance((*Elementpos));
							}
							++Elementpos;
						}
						(*ipcpos).ComplexNumber = ComplexNumber;
					}
				}
				++ipcpos;
			}
		}
		++nodepos;
	}
}

void NonLinearACFormula::CircultBackUpList()
{
	m_TempNodeList.clear();
	m_TempNodeList.resize(m_NodeList.size());
	copy(m_NodeList.begin(), m_NodeList.end(), m_TempNodeList.begin());

	m_TempProvList.clear();
	m_TempProvList.resize(m_ProvList.size());
	copy(m_ProvList.begin(), m_ProvList.end(), m_TempProvList.begin());
}

void NonLinearACFormula::Thevenintheorem()
{
	BOOL OperationFlag = FALSE;
	do 
	{
		OperationFlag = FALSE;
		SeriesMerger(OperationFlag);	//���� ����
		RarallelMerger(OperationFlag);	//���� ����(������)
		DuplicateMerger(OperationFlag);	//���� ����(�ΰ��� ������ ��,�Ա� ����)
	} while (OperationFlag);
}

BOOL NonLinearACFormula::IsProvArea(Node &CurrentNode)
{
	list<CBaseElement*>::iterator pos = m_ProvList.begin();
	while (pos != m_ProvList.end()) {

		list<CBaseElement*>::iterator Comparepos = (*pos)->m_InputElement.begin();
		list<CBaseElement*>::iterator Searchpos = find(CurrentNode.LineElement.begin(), CurrentNode.LineElement.end(), (*Comparepos));

		if (Searchpos != CurrentNode.LineElement.end()) {
			return TRUE;
		}
		++pos;
	}

	return FALSE;
}

complex<double> NonLinearACFormula::GetElementInpedance(CBaseElement *pElement)
{
	double Resistance = 0;
	double Impedance = 0;

	switch (pElement->m_Flag) {
		case composition:
			{
				Resistance = GetElementProperty(pElement, CString(_T("Resistance")));
				Impedance = GetElementProperty(pElement, CString(_T("Complexnumber")));
			}
			break;
		case resistance:
			Resistance = GetElementProperty(pElement, CString(_T("Resistance")));
			break;

		case inductor:
			Impedance = GetElementProperty(pElement, CString(_T("Impedance")));
			break;

		case capacity:
			Impedance -= GetElementProperty(pElement, CString(_T("Impedance")));
			break;

		default:
			AfxMessageBox(_T("LinearACFormula : GetElementInpedance() Exception"));
			break;
	}
	return double_complex(Resistance, Impedance);
}

void NonLinearACFormula::GetFamilyNode(Node &CurrentNode, CBaseElement *CompareElement, Node **ResultNode)
{
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		if ((*pos).NodeNumber != CurrentNode.NodeNumber) {	//�ߺ� �ƴҶ�

			list<CBaseElement*>::iterator Searchpos = find((*pos).AllElement.begin(), (*pos).AllElement.end(), CompareElement);
			if (Searchpos != (*pos).AllElement.end()) {
				(*ResultNode) = &(*pos);
				return;
			}
		}
		++pos;
	}
}

int NonLinearACFormula::DeleteElementInNode(Node &CurrentNode, CBaseElement *pElement)
{
	CurrentNode.AllElement.remove(pElement);

	list<CBaseElement*>::iterator inputpos = CurrentNode.InputElement.begin();
	while (inputpos != CurrentNode.InputElement.end()) {
		if ((*inputpos) == pElement) {
			break;
		}
		++inputpos;
	}

	if (inputpos != CurrentNode.InputElement.end()) {
		CurrentNode.InputElement.erase(inputpos);
		return inputlist;
	}

	list<CBaseElement*>::iterator outputpos = CurrentNode.OutputElement.begin();
	while (outputpos != CurrentNode.OutputElement.end()) {
		if ((*outputpos) == pElement) {
			break;
		}
		++outputpos;
	}

	if (outputpos != CurrentNode.OutputElement.end()) {
		CurrentNode.OutputElement.erase(outputpos);
		return outputlist;
	}

	return non;
}

void NonLinearACFormula::SeriesMerger(BOOL &OperationFlag)
{
	//�⺻������ ���κ갡 ���Ե� ������ �����Ѵ�.
	//�������׽� �ΰ��� ������ �ϳ��� ��ġ�� �ϳ��� ���ڸ� �����Ѵ�.
	//�ߺ�ó���� ���� ���� �ѹ� ���� �� ����
	//���� ����(���п� ���� ���) ���� ���� ����

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		if ((*nodepos).FirstFlag == FALSE && IsProvArea((*nodepos)) == FALSE) {		//���κ� ������ �ƴҶ�
			if ((*nodepos).InputElement.size() == 1 && (*nodepos).OutputElement.size() == 1) {	//���� �����϶�

				//���Ǵ��� ����
				double_complex CompositionResistance(0,0);
				list<CBaseElement*>::iterator FirstElement = (*nodepos).InputElement.begin();
				CompositionResistance += GetElementInpedance((*FirstElement));

				list<CBaseElement*>::iterator SecondElement = (*nodepos).OutputElement.begin();
				CompositionResistance += GetElementInpedance((*SecondElement));


				//���� ��带 ������ �¿� ��� Ž���� �ش� ���ڵ� ����
				Node *FirstFamilyNode = NULL;
				int FirstConnectListPos = non;
				GetFamilyNode((*nodepos), (*FirstElement), &FirstFamilyNode);
				if (FirstFamilyNode != NULL) {
					FirstConnectListPos = DeleteElementInNode(*FirstFamilyNode, (*FirstElement));
				}

				Node *SecondFamilyNode = NULL;
				int SecondConnectListPos = non;
				GetFamilyNode((*nodepos), (*SecondElement), &SecondFamilyNode);
				if (SecondFamilyNode != NULL) {
					SecondConnectListPos = DeleteElementInNode(*SecondFamilyNode, (*SecondElement));
				}

				//���ο� ���� ���� �� ���� ��� �ΰ��� ����
				CElementFactory *pFactory = CMainFrame::CreateElementFactory();
				CBaseElement *NewElement = pFactory->CreateElement(composition);

				ElementData NewResistance;
				NewResistance.Text = CString(_T("Resistance"));
				double Unit = 1e-3;
				NewResistance.Value = CompositionResistance.real()*Unit;

				ElementData NewImpedance;
				NewImpedance.Text = CString(_T("Complexnumber"));
				NewImpedance.Value = CompositionResistance.imag();

				list<ElementData> SendList;
				SendList.push_back(NewResistance);
				SendList.push_back(NewImpedance);
				NewElement->OnEditProperties(&SendList);

				FirstConnectListPos == non ? NULL :
					FirstConnectListPos == inputlist ? FirstFamilyNode->InputElement.push_back(NewElement) : FirstFamilyNode->OutputElement.push_back(NewElement);
				FirstConnectListPos != non ? FirstFamilyNode->AllElement.push_back(NewElement) : NULL;

				SecondConnectListPos == non ? NULL :
					SecondConnectListPos == inputlist ? SecondFamilyNode->InputElement.push_back(NewElement) : SecondFamilyNode->OutputElement.push_back(NewElement);
				SecondConnectListPos != non ? SecondFamilyNode->AllElement.push_back(NewElement) : NULL;

				//�ش� ��� ����
				m_NodeList.erase(nodepos);

				//�ӽ� ����Ʈ�� ����, ���� ������ ���� �ϱ� ����
				m_TempList.push_back(NewElement);

				OperationFlag = TRUE;
				break;
			}
		}
		++nodepos;
	}
}

BOOL NonLinearACFormula::IsAnotherNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode)
{
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		if ((*pos).NodeNumber != CurrentNode.NodeNumber) {

			list<CBaseElement*>::iterator Searchpos = find((*pos).AllElement.begin(), (*pos).AllElement.end(), pElement);
			if (Searchpos != (*pos).AllElement.end()) {
				(*AnotherNode) = &(*pos);
				return TRUE;
			}
		}
		++pos;
	}
	return FALSE;
}

void NonLinearACFormula::RarallelMerger(BOOL &OperationFlag)
{
	//�⺻������ ���κ갡 ���Ե� ������ �����Ѵ�.
	//������ ���ı����� �����Ѵ�. �� ������ ������ ���ڵ��� ����� �ٸ� ��尡 ����� �Ѵ�.

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		if (/*IsProvArea((*nodepos)) == FALSE*/TRUE) {	//���κ� ������ �ƴҶ�
			if ((*nodepos).OutputElement.size() >= 2) {	//���� �����϶�

				DuplicateNode fflushNode;
				list<CBaseElement*> Mergerlist;
				list<CBaseElement*>::iterator outputpos = (*nodepos).OutputElement.begin();
				while (outputpos != (*nodepos).OutputElement.end()) {
					if (IsAnotherNode((*nodepos), (*outputpos), &fflushNode.AnotherNode) == FALSE) {
						Mergerlist.push_back((*outputpos));
					}
					++outputpos;
				}

				if (Mergerlist.size() >= 2) {	//���� ���ڰ� ���� ������ �������

					//���Ǵ��� ����
					double_complex CompositionResistance(0,0);
					list<CBaseElement*>::iterator FirstElement = Mergerlist.begin();
					double_complex FirstInpedance(0,0);
					FirstInpedance = GetElementInpedance((*FirstElement));

					list<CBaseElement*>::iterator SecondElement = Mergerlist.begin();
					++SecondElement;
					double_complex SecondInpedance(0,0);
					SecondInpedance = GetElementInpedance((*SecondElement));

					CompositionResistance = (FirstInpedance*SecondInpedance)/(FirstInpedance+SecondInpedance);

					//���� ��忡�� �ΰ��� ���� ����
					DeleteElementInNode((*nodepos), (*FirstElement));
					DeleteElementInNode((*nodepos), (*SecondElement));

					//���ο� ���� ������ �ռ� ���Ǵ��� ����
					CElementFactory *pFactory = CMainFrame::CreateElementFactory();
					CBaseElement *NewElement = pFactory->CreateElement(composition);

					ElementData NewResistance;
					NewResistance.Text = CString(_T("Resistance"));
					double Unit = 1e-3;
					NewResistance.Value = CompositionResistance.real()*Unit;

					ElementData NewImpedance;
					NewImpedance.Text = CString(_T("Complexnumber"));
					NewImpedance.Value = CompositionResistance.imag();

					list<ElementData> SendList;
					SendList.push_back(NewResistance);
					SendList.push_back(NewImpedance);
					NewElement->OnEditProperties(&SendList);

					//���� ��忡 ���ο� ���� ����
					(*nodepos).AllElement.push_back(NewElement);
					(*nodepos).OutputElement.push_back(NewElement);

					m_TempList.push_back(NewElement);
					OperationFlag = TRUE;
					break;
				}
			}
		}
		++nodepos;
	}
}

void NonLinearACFormula::GetDuplicateNode(Node &CurrentNode, list<CBaseElement*> &MergerList)
{
	//outputElement���� ���� ��带 ������ �ٸ� ��带 �����˻�
	list<CBaseElement*>::iterator outputpos = CurrentNode.OutputElement.begin();
	while (outputpos != CurrentNode.OutputElement.end()) {

		DuplicateNode FirstNode;
		if (IsAnotherNode(CurrentNode, (*outputpos), &FirstNode.AnotherNode)) {

			list<CBaseElement*>::iterator lowpos = CurrentNode.OutputElement.begin();
			while (lowpos != CurrentNode.OutputElement.end()) {

				if ((*outputpos) != (*lowpos)) {
					DuplicateNode SecondNode;
					if (IsAnotherNode(CurrentNode, (*lowpos), &SecondNode.AnotherNode)) {

						if (FirstNode.AnotherNode->NodeNumber == SecondNode.AnotherNode->NodeNumber) {
							MergerList.push_back((*outputpos));
							MergerList.push_back((*lowpos));
							return;
						}
					}
				}
				++lowpos;
			}
		}
		++outputpos;
	}
}

void NonLinearACFormula::DuplicateMerger(BOOL &OperationFlag)
{
	//�⺻������ ���κ갡 ���Ե� ������ �����Ѵ�.
	//��尡 �����϶� �� ���ڵ��� ����� �ٸ� ��尡 �ߺ��϶� �ߺ�����
	//RarallelMerger()�� ����ϳ�, ��� ������ ���� ����

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		if (/*IsProvArea((*nodepos)) == FALSE*/TRUE) {	//���κ� ������ �ƴҶ�
			if ((*nodepos).OutputElement.size() >= 2) {	//���� �����϶�

				list<CBaseElement*> Mergerlist;
				GetDuplicateNode((*nodepos), Mergerlist);

				if (Mergerlist.size() >= 2) {

					//���Ǵ��� ����
					double_complex CompositionResistance(0,0);
					list<CBaseElement*>::iterator FirstElement = Mergerlist.begin();
					double_complex FirstInpedance(0,0);
					FirstInpedance = GetElementInpedance((*FirstElement));

					list<CBaseElement*>::iterator SecondElement = Mergerlist.begin();
					++SecondElement;
					double_complex SecondInpedance(0,0);
					SecondInpedance = GetElementInpedance((*SecondElement));

					CompositionResistance = (FirstInpedance*SecondInpedance)/(FirstInpedance+SecondInpedance);

					//���� ��带 ������ �¿� ��� Ž���� �ش� ���ڵ� ����
					Node *FirstFamilyNode = NULL;
					int FirstConnectListPos = non;
					GetFamilyNode((*nodepos), (*FirstElement), &FirstFamilyNode);
					if (FirstFamilyNode != NULL) {
						FirstConnectListPos = DeleteElementInNode(*FirstFamilyNode, (*FirstElement));
					}

					Node *SecondFamilyNode = NULL;
					int SecondConnectListPos = non;
					GetFamilyNode((*nodepos), (*SecondElement), &SecondFamilyNode);
					if (SecondFamilyNode != NULL) {
						SecondConnectListPos = DeleteElementInNode(*SecondFamilyNode, (*SecondElement));
					}

					//���� ��忡�� �ΰ��� ���� ����
					DeleteElementInNode((*nodepos), (*FirstElement));
					DeleteElementInNode((*nodepos), (*SecondElement));

					//���ο� ���� ������ �ռ� ���Ǵ��� ����
					CElementFactory *pFactory = CMainFrame::CreateElementFactory();
					CBaseElement *NewElement = pFactory->CreateElement(composition);

					ElementData NewResistance;
					NewResistance.Text = CString(_T("Resistance"));
					double Unit = 1e-3;
					NewResistance.Value = CompositionResistance.real()*Unit;

					ElementData NewImpedance;
					NewImpedance.Text = CString(_T("Complexnumber"));
					NewImpedance.Value = CompositionResistance.imag();

					list<ElementData> SendList;
					SendList.push_back(NewResistance);
					SendList.push_back(NewImpedance);
					NewElement->OnEditProperties(&SendList);

					(*nodepos).AllElement.push_back(NewElement);
					(*nodepos).OutputElement.push_back(NewElement);

					//FirstFamilyNode�� SecondFamilyNode�� 100%��ġ �ؾ� �Ѵ�.
					if (FirstFamilyNode == SecondFamilyNode) {	//�ߺ��� �ٸ� ��忡�� ���ο� ���� ����
						FirstFamilyNode->AllElement.push_back(NewElement);

						FirstConnectListPos == non ? NULL :
							FirstConnectListPos == inputlist ? FirstFamilyNode->InputElement.push_back(NewElement) : FirstFamilyNode->OutputElement.push_back(NewElement);
					}
					else {
						AfxMessageBox(_T("LinearACFormula : DuplicateMerger() FirstFamilyNode != SecondFamilyNode"));
					}

					m_TempList.push_back(NewElement);

					OperationFlag = TRUE;
					break;
				}
			}
		}
		++nodepos;
	}
}


void NonLinearACFormula::Initialize(list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	m_SecMax = pFrame->m_SecMax;
	m_VoltMax = pFrame->m_VoltMax;

	list<CBaseElement*>::iterator provpos = Provlist.begin();
	while (provpos != Provlist.end()) {

		if ((*provpos)->m_InputElement.size() > 0) {
			list<CBaseElement*>::iterator CompareElement = (*provpos)->m_InputElement.begin();	//���κ� ��ġ

			list<Node>::iterator nodepos = m_NodeList.begin();
			while (nodepos != m_NodeList.end()) {

				list<CBaseElement*>::iterator searchpos = find((*nodepos).LineElement.begin(), (*nodepos).LineElement.end(), (*CompareElement));
				if (searchpos != (*nodepos).LineElement.end()) {
					IPCData NewData;
					NewData.ComplexCheckFlag = FALSE;
					NewData.NodeNumber = (*nodepos).NodeNumber;
					NewData.ProvNumber = (*provpos)->m_ElementNumber;
					(*nodepos).FirstFlag == TRUE ? NewData.FirstFlag = TRUE : NewData.FirstFlag = FALSE;
					Datalist.push_back(NewData);
				}
				++nodepos;
			}
		}
		++provpos;
	}
}



void NonLinearACFormula::IPCDataConvert(list<IPCData> &Datalist, int iCount)
{
	list<IPCData>::iterator pos = Datalist.begin();
	while (pos != Datalist.end()) {
		(*pos).GrapehValue[iCount] = m_Result[(*pos).NodeNumber];
		++pos;
	}
}

void NonLinearACFormula::LinearStencilConvert(double fluidVoltage)
{
	int NodeSize = m_NodeList.size();

	m_StencilBuffer = new double*[NodeSize];
	m_StencilBuffer[0] = new double[NodeSize * (NodeSize + 1)];
	memset(m_StencilBuffer[0], 0, sizeof(double) * NodeSize * (NodeSize + 1));
	for	(int iCount = 1; iCount < NodeSize; iCount++) {
		m_StencilBuffer[iCount] = m_StencilBuffer[iCount -1] + (NodeSize + 1);
	}

	//���⼭ �߿������� �ռ������� �����ؾ� �Ѵٴ°��̴�
	//�Ϲ� ������ 1 / R1 �� ����� �ϸ� 
	//�ռ� ������ ������ ������ �Ϲ��������� ����� ���ؾ� �Ѵ�. (�Ϲ� �ռ����� ���İ� �ٸ���)
	//���� ���ٽ� ����߿� ���� ��ġ�� ���ʰ� �����ʿ� ���� ������ �ش���ڵ��� �ٸ����� ���Ѱ͸� �����Ѵ�.
	//����� ��� �����̴�.

	int LineNumber = 0;
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		if ((*pos).FirstFlag) {		//���� ��ġ ����̸� ����� ����			
			list<CBaseElement*>::iterator lowpos = (*pos).AllElement.begin();
			while (lowpos != (*pos).AllElement.end()) {
				if ((*lowpos)->m_Flag == acpower) {
					m_StencilBuffer[LineNumber][LineNumber] = 1.0;
					m_StencilBuffer[LineNumber][NodeSize] = fluidVoltage;
					break;
				}
				++lowpos;
			}		
		}
		else {
			GetInputCombinedResistance((*pos), m_StencilBuffer[LineNumber]);
			m_StencilBuffer[LineNumber][LineNumber] += GetCombinedResistance(*pos);
			GetOutputCombinedResistance((*pos), m_StencilBuffer[LineNumber]);
		}
		++LineNumber;
		++pos;
	}
}


double NonLinearACFormula::GetCombinedResistance(Node &CurrentNode)
{
	double CombinedResistance = 0;

	list<CBaseElement*>::iterator pos = CurrentNode.AllElement.begin();
	while (pos != CurrentNode.AllElement.end()) {

		double CurrentValue = 0;
		switch ((*pos)->m_Flag) {
		case resistance:
			CurrentValue = GetElementProperty((*pos),  CString(_T("Resistance")));
			break;

		case inductor:
		case capacity:
			CurrentValue = GetElementProperty((*pos),  CString(_T("Impedance")));
			break;

		default:
			break;
		}

		CombinedResistance += (1/CurrentValue);
		++pos;
	}
	return CombinedResistance;
}

double NonLinearACFormula::GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
{
	double CombinedResistance = 0;
	list<CBaseElement*>::iterator inputpos = CurrentNode.InputElement.begin();
	while (inputpos != CurrentNode.InputElement.end()) {

		int NodeNumber = IsDuplicateNode(CurrentNode, (*inputpos));
		if (NodeNumber != -1) {	

			switch ((*inputpos)->m_Flag) {
				case resistance:
					StencilBuffer[NodeNumber] -= (1/GetElementProperty((*inputpos),  CString(_T("Resistance"))));
					break;

				case inductor:
				case capacity:
					StencilBuffer[NodeNumber] -= (1/GetElementProperty((*inputpos),  CString(_T("Impedance"))));
					break;

				default:
					break;
			}
		}
		++inputpos;
	}

	return CombinedResistance;
}

double NonLinearACFormula::GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
{
	double CombinedResistance = 0;

	list<CBaseElement*>::iterator outputpos = CurrentNode.OutputElement.begin();
	while (outputpos != CurrentNode.OutputElement.end()) {

		int NodeNumber = IsDuplicateNode(CurrentNode, (*outputpos));
		if (NodeNumber != -1) {
			switch ((*outputpos)->m_Flag) {
				case resistance:
					StencilBuffer[NodeNumber] -= (1/GetElementProperty((*outputpos),  CString(_T("Resistance"))));
					break;

				case inductor:
				case capacity:		
					StencilBuffer[NodeNumber] -= (1/GetElementProperty((*outputpos),  CString(_T("Impedance"))));
					break;

				default:
					break;
			}
		}
		++outputpos;
	}

	return CombinedResistance;
}

double NonLinearACFormula::GetElementProperty(CBaseElement *pElement, CString &PropertyString)
{
	//�ش� ��ü�� ������ ����
	list<ElementData> DataList;			
	pElement->GetProperties(DataList);

	//������ ������ ���ٽǿ� ����
	SearchData data;
	data.string = PropertyString;
	list<ElementData>::iterator searchpos = find_if(DataList.begin(), DataList.end(), data);
	if (searchpos != DataList.end()) {
		return (*searchpos).Value;
	}
	return 0;
}

int NonLinearACFormula::IsDuplicateNode(Node &BaseNode, CBaseElement *pElement)
{
	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {
		if ((*nodepos).NodeNumber != BaseNode.NodeNumber) {

			list<CBaseElement*>::iterator searchpos = find((*nodepos).AllElement.begin(), (*nodepos).AllElement.end(), pElement);
			if (searchpos != (*nodepos).AllElement.end()) {
				return (*nodepos).NodeNumber;
			}
		}
		++nodepos;
	}
	return -1;
}

BOOL NonLinearACFormula::GaussianElimination()
{
	double **a, *x, eps=1.e-10, sum;
	int i,j,k,n,np,ip;

	n = m_NodeList.size();
	np = n+1;
	x= (double*)malloc(n*sizeof(double));    // Ax=b ���� x(��, �츮�� ���Ϸ��� ��.����)
	x--;
	a= (double**)malloc(n*sizeof(double*));   // Ax=b ���� A�� b(matrix�� ����,���׼���)
	a--;

	for(i=1;i<=n;i++){
		a[i]=(double*)malloc(np*sizeof(double));
		a[i]--;
	}


	for(i=1;i<=n;i++)
		for(j=1;j<=np;j++)
			a[i][j] = m_StencilBuffer[i-1][j-1];

	for(i=1;i<n;i++){
		ip=i+1;
		for(j=ip;j<=n;j++){
			if(fabs(a[i][i])<eps){            //matrix�� �ظ� ������ ���������� �˻�.
				TRACE("Error: ����� �밢���Ұ� 0�Դϴ�.\n");
				return FALSE;
			} 
			a[j][i]=a[j][i]/a[i][i];
			for(k=ip;k<=np;k++)
				a[j][k]= a[j][k]-a[j][i]*a[i][k]; // (���콺 �ҰŹ�)Gaussian elimination  
		}
	}

	x[n]=a[n][np]/a[n][n];
	for(i=n-1;i>0;i--){
		sum=0.;
		for(j=i+1;j<=n;j++) 
			sum+=a[i][j]*x[j];
		x[i]=(a[i][np]-sum)/a[i][i]; // ���� ���Թ�(backward substitution)
	}

	m_Result = new double[n];
	memset(m_Result, 0, sizeof(double)*n);
	for(i=1;i<=n;i++)
		m_Result[i-1] = x[i];   //��� ����.


	//�޸� ����
	for(i=1;i<=n;i++){
		a[i]++;
		free(a[i]);
	}

	x++;
	free(x);
	a++;
	free(a);

	return TRUE;
}



////////////////////////////////////////////////////////////////////////// opamp�� ���� ����

BOOL NonLinearACFormula::OPAmpProcessing(list<IPCData> &VoltageList, list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	//copy
	m_NodeList.clear();
	m_NodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_NodeList.begin());

	m_ProvList.clear();
	m_ProvList.resize(Provlist.size());
	copy(Provlist.begin(), Provlist.end(), m_ProvList.begin());

	//Initialize and New IPCData
	Initialize(Datalist, m_ProvList);

	//ProveList, NodeList Backup
/*	CircultBackUpList();*/

// 	//All Section Composition Impedance
// 	SectionCompositonImpedance(Datalist);			//���ο� �׺�� ���� ����
// 
// 	//ProveList, NodeList, ReBackUp
// 	RestitutionList();

	if (CCircultSimulation::m_ACData.Amplitude != -1) {	//���� ������ �´� -1�� ���´�.

		list<IPCData>::iterator Datapos = VoltageList.begin();
		for (double iCount = 0; iCount < DivideSector; iCount++) {

			double fluidVoltage = (*Datapos).GrapehValue[(int)iCount];
			if (CCircultSimulation::m_RLCCase == rr) {					//�Ϲ����� R-R ȸ�� �϶��� ���ٽ��� ������ ����þ� �ҰŹ� ����
				//stencil convert (matrix)
				LinearStencilConvert(fluidVoltage);
				//matrix -> result
				GaussianElimination();

				//result->Data
				IPCDataConvert(Datalist, (int)iCount);
			}
		}
	}

	//���� �̵�
//	Phasemovement(Datalist);

	//��ó�� (���� RLC ���� �̵����� ���� MaxVolate ������ ��ȯ)
	//�ﰢ�� ���ý� (������ -> �ﰢ��)
	RearProcessing(Datalist);

	return TRUE;
}