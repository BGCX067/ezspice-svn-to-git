#include "StdAfx.h"
#include "LinearACFormula.h"
#include "BaseElement.h"
#include <algorithm>
#include <math.h>
#include "MainFrm.h"
#include "ElementFactory.h"
#include "NodePretreatment.h"
#include "LinearDCFormula.h"
#include "CircultSimulation.h"

double	LinearACFormula::m_fluidVoltage = 0;

LinearACFormula::LinearACFormula(void)
{
	m_StencilBuffer = NULL;
	m_Result = NULL;
	m_TempACPower = NULL;
	m_TempResistance = NULL;
}

LinearACFormula::~LinearACFormula(void)
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

void LinearACFormula::AheadProcessing(list<IPCData> &Datalist)
{
	//���ı��� üũ
	{
		list<Node>::iterator pos = m_NodeList.begin();
		while (pos != m_NodeList.end()) {
		
			if ((*pos).OutputElement.size() >= 2) {
				(*pos).DivideFlag = TRUE;
			}
			++pos;
		}
	}

	//��� ���Ǵ��� ���ڿ� ���ļ� ����
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		list<CBaseElement*>::iterator lowpos = (*pos).AllElement.begin();
		while (lowpos != (*pos).AllElement.end()) {
			if((*lowpos)->m_Flag == capacity || (*lowpos)->m_Flag == inductor) {

				list<ElementData> SendList;
				ElementData NewData;
				NewData.Text = CString(_T("Frequency"));
				NewData.Value = CCircultSimulation::m_ACData.Frequency;
				SendList.push_back(NewData);
				(*lowpos)->OnEditProperties(&SendList);
			}
			++lowpos;
		}
		++pos;
	}
	
	if (CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {	//rcȸ�� �̸鼭 ������ �϶� ������ 0k �������� ��ü �� ���κ� ��ġ ����
	
		//AC���� ã��
		list<Node>::iterator nodepos = m_NodeList.begin();
		while (nodepos != m_NodeList.end()) {
	
			if ((*nodepos).FirstFlag) {
	
				list<CBaseElement*>::iterator searchpos = (*nodepos).AllElement.begin();
				while (searchpos != (*nodepos).AllElement.end()) {
					if ((*searchpos)->m_Flag == acpower) {
						break;
					}
					++searchpos;
				}

				if (searchpos != (*nodepos).AllElement.end()) {
					m_TempACPower = (*searchpos);
					(*nodepos).FirstFlag = FALSE;
					break;
				}
			}
			++nodepos;
		}
		

		if (m_TempACPower != NULL) {	//acpower -> 0kohm resistance
			CElementFactory *pFactory = CMainFrame::CreateElementFactory();
			CBaseElement *NewElement = pFactory->CreateElement(resistance);

			list<ElementData> SendList;
			ElementData NewResistance;
			NewResistance.Text = CString(_T("Resistance"));
			NewResistance.Value = 0;
			SendList.push_back(NewResistance);

			NewElement->OnEditProperties(&SendList);

			(*nodepos).AllElement.remove(m_TempACPower);
			(*nodepos).AllElement.push_back(NewElement);
			list<CBaseElement*>::iterator powersearch = find((*nodepos).InputElement.begin(), (*nodepos).InputElement.end(), m_TempACPower);
			if (powersearch != (*nodepos).InputElement.end()) {
				(*nodepos).InputElement.remove(m_TempACPower);
				(*nodepos).InputElement.push_back(NewElement);
			}

			powersearch = find((*nodepos).OutputElement.begin(), (*nodepos).OutputElement.end(), m_TempACPower);
			if (powersearch != (*nodepos).OutputElement.end()) {
				(*nodepos).OutputElement.remove(m_TempACPower);
				(*nodepos).OutputElement.push_back(NewElement);
			}
		}


		//���κ� ��ġ ���� (ĳ�н��� �ֺ����� 2��)
		m_ProvList.clear();
		nodepos = m_NodeList.begin();
		while (nodepos != m_NodeList.end()) {

			list<CBaseElement*>::iterator elementpos = (*nodepos).AllElement.begin();
			while (elementpos != (*nodepos).AllElement.end()) {
				if ((*elementpos)->m_Flag == capacity) {

					//ĳ�н��� �յڷ� ���κ� ����

					list<CBaseElement*>::iterator outputpos = (*elementpos)->m_OutputElement.begin();
					while (outputpos != (*elementpos)->m_OutputElement.end()) {

						CElementFactory *pFactory = CMainFrame::CreateElementFactory();
						CBaseElement *NewElement = pFactory->CreateElement(prov);
						NewElement->m_InputElement.push_back((*outputpos));		
						m_ProvList.push_back(NewElement);
						++outputpos;
					}
					Datalist.clear();
					Initialize(Datalist, m_ProvList);
					return;
				}
				++elementpos;
			}
			++nodepos;
		}
	}
}

void LinearACFormula::RearProcessing(list<IPCData> &Datalist)	//��ó��
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

BOOL LinearACFormula::RecusiveAnotherDuplicateMerger(Node &CompareNode, BOOL FirstFlag, list<Node> &LowNodeList)
{
	if (FirstFlag) {
		Node NewNode;
		NewNode.NodeNumber = CompareNode.NodeNumber;
		NewNode.FirstFlag = CompareNode.FirstFlag;

		list<CBaseElement*>::iterator pos = CompareNode.OutputElement.begin();
		while (pos != CompareNode.OutputElement.end()) {
			NewNode.OutputElement.push_back((*pos));
			NewNode.AllElement.push_back((*pos));
			++pos;
		}
		LowNodeList.push_back(NewNode);
	}
	else {
		if (CompareNode.InputElement.size() >= 2) {
			return TRUE;
		}
	}

	BOOL nFlag = FALSE;
	list<CBaseElement*>::iterator pos = CompareNode.OutputElement.begin();
	while (pos != CompareNode.OutputElement.end()) {

		BOOL SearchFlag = FALSE;
		list<Node>::iterator nodepos = m_NodeList.begin();
		while (nodepos != m_NodeList.end()) {

			if ((*nodepos).NodeNumber != CompareNode.NodeNumber) {
				list<CBaseElement*>::iterator searchpos = find((*nodepos).AllElement.begin(), (*nodepos).AllElement.end(), (*pos));
				if (searchpos != (*nodepos).AllElement.end()) {
					SearchFlag = TRUE;
					LowNodeList.push_back((*nodepos));
					break;
				}
			}
			++nodepos;
		}

		if (SearchFlag) {
			if (RecusiveAnotherDuplicateMerger((*nodepos) , FALSE, LowNodeList)) {
				return TRUE;
			}
		}
		++pos;
	}
	return FALSE;
}

void LinearACFormula::ConvertIsopotentialNode()
{
 	BOOL IsopotentialFlag = FALSE;
	do 
	{
		IsopotentialFlag = FALSE;
		list<Node> LowNodeList;
		list<Node>::iterator Nodepos = m_IsopotentNodeList.begin();
		while (Nodepos != m_IsopotentNodeList.end()) {

			//������ �˻�
			if ((*Nodepos).InputElement.size() >= 2) {	//input�� �ΰ��� �ڿ� ��� ��ü�� ���ķ� ���� �� ������ 

				//���� ��忡 �������� �� �ֳ�?
				if (RecusiveAnotherDuplicateMerger((*Nodepos), TRUE, LowNodeList) == FALSE) {	//�������� ���⶧���� ������ ��ȯ
					//������ ��� ������� �� ���� �� ����
					IncreaseNodeImpedance((*Nodepos), LowNodeList);
					IsopotentialFlag = TRUE;
					break;
				}
			}
			++Nodepos;
		}

		//���� ������ ����
		list<Node>::iterator lownodepos = LowNodeList.begin();
		while (lownodepos != LowNodeList.end()) {
			
			list<Node>::iterator Nodepos = m_IsopotentNodeList.begin();
			while (Nodepos != m_IsopotentNodeList.end()) {
				if ((*lownodepos).NodeNumber == (*Nodepos).NodeNumber) {
					break;
				}
				++Nodepos;
			}

			if (Nodepos != m_IsopotentNodeList.end()) {
				m_IsopotentNodeList.erase(Nodepos);
			}
			++lownodepos;
		}
	} while (IsopotentialFlag);


	//��� ����� IPCData����
	{
		list<Node>::iterator nodepos = m_IsopotentNodeList.begin();
		while (nodepos != m_IsopotentNodeList.end()) {
			IPCData NewData;
			NewData.ComplexCheckFlag = FALSE;
			NewData.NodeNumber = (*nodepos).NodeNumber;
			(*nodepos).FirstFlag == TRUE ? NewData.FirstFlag = TRUE : NewData.FirstFlag = FALSE;
			m_IsopotentIPCList.push_back(NewData);
			++nodepos;
		}
	}
}

void LinearACFormula::IncreaseNodeImpedance(Node &CurrentNode, list<Node> &LowNodeList)
{
	int AmplificationCount = CurrentNode.InputElement.size();
	list<CBaseElement*>::iterator pos = CurrentNode.InputElement.begin();
	while (pos != CurrentNode.InputElement.end()) {

		list<Node> NewList;
		CopyNodeGroup(NewList, LowNodeList, AmplificationCount);

		list<Node>::iterator connectpos = NewList.begin();
		(*connectpos).InputElement.push_back((*pos));
		(*connectpos).AllElement.push_back((*pos));
		
		while (connectpos != NewList.end()) {
			m_IsopotentNodeList.push_back((*connectpos));
			++connectpos;
		}
		++pos;
	}
}

void LinearACFormula::CopyNodeGroup(list<Node> &EmptyList, list<Node> &CopyList, int AmplificationCount)
{
	list<CopyNodeElement> DuplicateList;

	CElementFactory *pFactory = CMainFrame::CreateElementFactory();
	list<Node>::iterator nodepos = CopyList.begin();
	while (nodepos != CopyList.end()) {

		Node NewNode;
		NewNode.FirstFlag = (*nodepos).FirstFlag;
		CBaseElement *NewProvLine = pFactory->CreateElement(lines);
		NewNode.LineElement.push_back(NewProvLine);

		MappingNodeNumber NewNumber;
		NewNumber.BaseNumber = (*nodepos).NodeNumber;
		NewNumber.CopyNumber = NewNode.NodeNumber;
		m_IsopotentMappingList.push_back(NewNumber);

		//inputelement
		{
			list<CBaseElement*>::iterator elementpos = (*nodepos).InputElement.begin();
			while (elementpos != (*nodepos).InputElement.end()) {

				CopyNodeElement DuplicateSearch;
				DuplicateSearch.BaseElement = (*elementpos);

				list<CopyNodeElement>::iterator Duplicatepos = find_if(DuplicateList.begin(),DuplicateList.end(), DuplicateSearch);
				if (Duplicatepos != DuplicateList.end()) {

					NewNode.InputElement.push_back((*Duplicatepos).CopyElement);
					NewNode.AllElement.push_back((*Duplicatepos).CopyElement);
				}
				else {
					CBaseElement *CopyElement = (*elementpos)->Clone();
					m_TempList.push_back(CopyElement);

					double Value = 0;
					switch (CopyElement->m_Flag) {

						case resistance:
							Value = GetElementProperty(CopyElement, CString(_T("Resistance")));
							SetElementProperty(CopyElement, CString(_T("Resistance")), Value * AmplificationCount * 1e-3);
							break;

						case inductor:
							Value = GetElementProperty(CopyElement, CString(_T("Inductance")));
							SetElementProperty(CopyElement, CString(_T("Inductance")), Value * AmplificationCount);
							SetElementProperty(CopyElement, CString(_T("Frequency")), CCircultSimulation::m_ACData.Frequency);
							break;

						case capacity:
							Value = GetElementProperty(CopyElement, CString(_T("Capacitance")));
							SetElementProperty(CopyElement, CString(_T("Capacitance")), (Value * 1e+6)/AmplificationCount);
							SetElementProperty(CopyElement, CString(_T("Frequency")), CCircultSimulation::m_ACData.Frequency);
							break;

						default:
							AfxMessageBox(_T("������ ��ȯ ���� �߻�"));
							break;
					}
					NewNode.InputElement.push_back(CopyElement);
					NewNode.AllElement.push_back(CopyElement);

					CopyNodeElement NewCopy;
					NewCopy.BaseElement = (*elementpos);
					NewCopy.CopyElement = CopyElement;
					DuplicateList.push_back(NewCopy);
				}
				++elementpos;
			}
		}

		//outputelement
		{
			list<CBaseElement*>::iterator elementpos = (*nodepos).OutputElement.begin();
			while (elementpos != (*nodepos).OutputElement.end()) {

				CopyNodeElement DuplicateSearch;
				DuplicateSearch.BaseElement = (*elementpos);

				list<CopyNodeElement>::iterator Duplicatepos = find_if(DuplicateList.begin(),DuplicateList.end(), DuplicateSearch);
				if (Duplicatepos != DuplicateList.end()) {

					NewNode.OutputElement.push_back((*Duplicatepos).CopyElement);
					NewNode.AllElement.push_back((*Duplicatepos).CopyElement);
				}
				else {
					CBaseElement *CopyElement = (*elementpos)->Clone();
					m_TempList.push_back(CopyElement);

					double Value = 0;
					switch (CopyElement->m_Flag) {

						case resistance:
							Value = GetElementProperty(CopyElement, CString(_T("Resistance")));
							SetElementProperty(CopyElement, CString(_T("Resistance")), Value * AmplificationCount * 1e-3);
							break;

						case inductor:
							Value = GetElementProperty(CopyElement, CString(_T("Inductance")));
							SetElementProperty(CopyElement, CString(_T("Inductance")), Value * AmplificationCount);
							SetElementProperty(CopyElement, CString(_T("Frequency")), CCircultSimulation::m_ACData.Frequency);
							break;

						case capacity:
							Value = GetElementProperty(CopyElement, CString(_T("Capacitance")));
							SetElementProperty(CopyElement, CString(_T("Capacitance")), (Value * 1e+6)/AmplificationCount);
							SetElementProperty(CopyElement, CString(_T("Frequency")), CCircultSimulation::m_ACData.Frequency);
							break;

						default:
							AfxMessageBox(_T("������ ��ȯ ���� �߻�"));
							break;
					}
					NewNode.OutputElement.push_back(CopyElement);
					NewNode.AllElement.push_back(CopyElement);

					CopyNodeElement NewCopy;
					NewCopy.BaseElement = (*elementpos);
					NewCopy.CopyElement = CopyElement;
					DuplicateList.push_back(NewCopy);
				}
				++elementpos;
			}
		}
		EmptyList.push_back(NewNode);
		++nodepos;
	}
}

void LinearACFormula::SetElementProperty(CBaseElement *pElement, CString &PropertyString, double Value)
{
	list<ElementData> SendList;

	ElementData NewData;
	NewData.Text = PropertyString;
	NewData.Value = Value;

	SendList.push_back(NewData);
	pElement->OnEditProperties(&SendList);
}

BOOL LinearACFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
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

	//Capacity, Inductor -> Insert Frequency;
	AheadProcessing(Datalist);

	//������ 
	m_IsopotentNodeList.clear();
	m_IsopotentNodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_IsopotentNodeList.begin());

	//������ ��ȯ
	ConvertIsopotentialNode();

	//ProveList, NodeList Backup
	CircultBackUpList();

	//All Section Composition Impedance
	SectionCompositonImpedance(Datalist);			//���ο� �׺�� ���� ����
	
	//ProveList, NodeList, ReBackUp
	RestitutionList();

	if (CCircultSimulation::m_ACData.Amplitude != -1) {	//���� ������ �´� -1�� ���´�.

		BOOL RCInitializeFlag = FALSE;
		int	RecoveryCount = 0;
		double Vm = 0, Vpp = 0, Timeconstant = 0;
		double fluidAnniversary = 0, CurrentAnniVersary = 0;
		double PlusMoveMinus = 0, MinusMovePlus = 0;	

		double fluidFrequency = (1/CCircultSimulation::m_ACData.Frequency)/DivideSector;
		for (double iCount = 0; iCount < DivideSector; iCount++) {

			double fluidVoltage = CCircultSimulation::m_ACData.Amplitude*sin(2*PIE*CCircultSimulation::m_ACData.Frequency*(fluidFrequency*iCount));
			m_fluidVoltage = fluidVoltage;

			if (CCircultSimulation::m_RLCCase == rr) {	//�Ϲ����� R-R ȸ�� �϶��� ���ٽ��� ������ ����þ� �ҰŹ� ����
				//stencil convert (matrix)
				LinearStencilConvert(fluidVoltage);

				//matrix -> result
				GaussianElimination();

				//result->Data
				IPCDataConvert(Datalist, (int)iCount);
			}
			else { // rc, rl, rlc //������ ȸ���϶��� ������ Ư���� ����

				if (CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {	//R-C ȸ�� ������ �϶�
					
					//1. ���а��ϸ� ���� ���ο� Max���� üũ(vm)
					//2. Vm, Vp-p ��� (Vm�� abs(Max����), Vp-p�� (2*Vm)
					//3. ������ üũ (������ ����� ��������� ���� �׺�� ���� ����)
					//4. �ʱ� �۾��� ���� ���, �ϰ� �ʱⰪ üũ
					//5. ���� ����
					// - -> +
					//�ʱⰪ + (Vpp - (vm + �ʱⰪ)) * (1-Exp(-t/rc)) // �ʱⰪ MinusMovePlus
					// + -> -
					//�ʱⰪ + (Vpp + (vm - �ʱⰪ)) * (1-Exp(-t/rc)) // �ʱⰪ PlusMoveMinus


					if (RCInitializeFlag == FALSE) {
						//���а��ϸ� ���� Max���� üũ
						
						Vm = VoltageDropping();
						Vpp = 2*abs(Vm);

						//������ üũ
						double Capacitanece = GetCapacitance();
						double CompositionResistance = 0;

						list<IPCData>::iterator ipcpos = Datalist.begin();
						while (ipcpos != Datalist.end()) {
							CompositionResistance += ipcpos->ComplexNumber.real();	
							++ipcpos;
						}
						Timeconstant = Capacitanece * CompositionResistance;

						//�ʱ� �۾��� ���� ���, �ϰ� �ʱⰪ üũ
						
						fluidAnniversary = (1/CCircultSimulation::m_ACData.Frequency)/(DivideSector/2);	//�ֱ� ����
						double CurrentAnniVersary = 0;
						for (int jCount = 0; jCount <DivideSector/2; ++jCount) {	//���
							PlusMoveMinus = (Vpp - Vm)*(1-exp(-CurrentAnniVersary/Timeconstant));
							CurrentAnniVersary += fluidAnniversary;
						}

						CurrentAnniVersary = 0;
						for (int jCount = DivideSector/2; jCount < DivideSector; ++jCount) { //�ϰ�
							MinusMovePlus = PlusMoveMinus + (-Vpp + (Vm - PlusMoveMinus))*(1-exp(-CurrentAnniVersary/Timeconstant));
							CurrentAnniVersary += fluidAnniversary;
						}

						//IPCDataList ���� �Ѱ��� ����
						if (Datalist.size() > 1) {
							
							int DeleteCount = Datalist.size() -1;
							for (int iCount = 0; iCount < DeleteCount; iCount++) {
								Datalist.pop_back();
							}
						}

						//ù��° �� ����
						list<IPCData>::iterator pos = Datalist.begin();
						(*pos).GrapehValue[(int)iCount] = MinusMovePlus;

						CurrentAnniVersary = 0;
						RCInitializeFlag = TRUE;
					}
					else{

						if ((int)iCount == DivideSector/2) CurrentAnniVersary = 0;

						list<IPCData>::iterator pos = Datalist.begin();
						if ((int)iCount < DivideSector/2) {
							CurrentAnniVersary += fluidAnniversary;
							(*pos).GrapehValue[(int)iCount] = MinusMovePlus + (Vpp - (Vm + MinusMovePlus)) * (1-exp(-CurrentAnniVersary/Timeconstant));
						}
						else {
							CurrentAnniVersary += fluidAnniversary;
							(*pos).GrapehValue[(int)iCount] =  PlusMoveMinus + (-Vpp + (Vm - PlusMoveMinus))*(1-exp(-CurrentAnniVersary/Timeconstant));
						}
					}
				}
				else {
					list<IPCData>::iterator FirstNodepos = m_IsopotentIPCList.begin();

					double FirstCompositionImpedance = sqrt(pow((*FirstNodepos).ComplexNumber.real(), 2) + pow((*FirstNodepos).ComplexNumber.imag(), 2));
					double ConvertAlternating = fluidVoltage/FirstCompositionImpedance;	//���� ���� ����

					list<IPCData>::iterator pos = Datalist.begin();
					while (pos != Datalist.end()) {

						double LocalElectric = GetNodeElectric((*pos).NodeNumber);
						double_complex LocalComplex = GetMappingComplex((*pos).NodeNumber);

						double CurrentCompositionImpedance = sqrt(pow(LocalComplex.real(), 2) + pow(LocalComplex.imag(), 2));
						int DivideCount = GetDivideCount((*pos).NodeNumber);
						CurrentCompositionImpedance /= DivideCount;
						double ConvertVoltage = LocalElectric*CurrentCompositionImpedance;

						(*pos).GrapehValue[(int)iCount] = ConvertVoltage;
						++pos;
					}
				}
			}
		}
	}

	if (CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {
		NULL;
	}
	else {
		//���� �̵�
		Phasemovement(Datalist);
	}
	

	//��ó�� (���� RLC ���� �̵����� ���� MaxVolate ������ ��ȯ)
	//�ﰢ�� ���ý� (������ -> �ﰢ��)
	RearProcessing(Datalist);
	return TRUE;
}

int LinearACFormula::GetDivideCount(int NodeNumber)
{
	int iCount = 0;
	list<MappingNodeNumber>::iterator mapingpos = m_IsopotentMappingList.begin();
	while (mapingpos != m_IsopotentMappingList.end()) {

		if (mapingpos->BaseNumber == NodeNumber || mapingpos->CopyNumber == NodeNumber) {
			iCount++;
		}
		++mapingpos;
	}

	if (iCount ==0 ) {
		list<Node>::iterator nodepos = m_IsopotentNodeList.begin();
		while (nodepos != m_IsopotentNodeList.end()) {
			if ((*nodepos).NodeNumber == NodeNumber) {
				iCount++;
				break;
			}
			++nodepos;
		}
	}

	return iCount;
}

double LinearACFormula::GetNodeElectric(int NodeNumber)
{
	double ConvertElectric = 0;
	BOOL nFlag = FALSE;

	list<MappingNodeNumber>::iterator mapingpos = m_IsopotentMappingList.begin();
	while (mapingpos != m_IsopotentMappingList.end()) {

		if (mapingpos->BaseNumber == NodeNumber) {

			list<Node>::iterator nodepos = m_IsopotentNodeList.begin();
			while (nodepos != m_IsopotentNodeList.end()) {
				if ((*nodepos).NodeNumber == mapingpos->CopyNumber) {
					break;
				}
				++nodepos;
			}
			ConvertElectric += RecusiveParallelSearch((*nodepos));
			nFlag = TRUE;			
		}
		++mapingpos;
	}

	if (nFlag == FALSE) {
		list<Node>::iterator nodepos = m_IsopotentNodeList.begin();
		while (nodepos != m_IsopotentNodeList.end()) {
			if ((*nodepos).NodeNumber == NodeNumber) {
				break;
			}
			++nodepos;
		}
		ConvertElectric = RecusiveParallelSearch((*nodepos));
	}

	return ConvertElectric;
}



double LinearACFormula::RecusiveParallelSearch(Node &CurrentNode, CBaseElement* CallElement/* = NULL*/)
{
	//������� �������� ����ϸ� �˻�
	//�ϴ� ���ձ����� ���ٰ� ����

	BOOL ParalleFlag = FALSE;
	if (CurrentNode.OutputElement.size() >= 2) { //���� üũ
		ParalleFlag = TRUE;
	}

	if (CurrentNode.FirstFlag) {	//ó�� ��� �ϰ�� 
		double FirstNodeElectric = 0;
		if (ParalleFlag) {
			if (CallElement != NULL) {
				DivideNode SearchNode;
				SearchNode.ElementNumber = CallElement->m_ElementNumber;
				list<DivideNode>::iterator dividepos = find_if(CurrentNode.DivideElement.begin(), CurrentNode.DivideElement.end(), SearchNode);
				if (dividepos != CurrentNode.DivideElement.end()) {

					double CompositionImpedance = sqrt(pow((*dividepos).DivideComplexNumber.real(), 2) + pow((*dividepos).DivideComplexNumber.imag(), 2));
					FirstNodeElectric = m_fluidVoltage/CompositionImpedance;
					(*dividepos).DivideElectric = FirstNodeElectric;
				}

				//���� ������ ���� ���ؼ� ����
				list<DivideNode>::iterator dividenodepos = CurrentNode.DivideElement.begin();
				while (dividenodepos != CurrentNode.DivideElement.end()) {

					if ((*dividenodepos).ElementNumber != CallElement->m_ElementNumber) {

						double DivideImpedance = sqrt(pow((*dividenodepos).DivideComplexNumber.real(), 2) + pow((*dividenodepos).DivideComplexNumber.imag(), 2));
						double OtherNodeElectric = m_fluidVoltage/DivideImpedance;
						(*dividenodepos).DivideElectric = OtherNodeElectric;
					}
					++dividenodepos;
				}
			}
			else { //���κ� ��ġ�� ó�� ����϶�
				double_complex CurrentComplex = GetNodeComplex(CurrentNode.NodeNumber);

				double CompositionImpedance = sqrt(pow(CurrentComplex.real(), 2) + pow(CurrentComplex.imag(), 2));
				FirstNodeElectric = m_fluidVoltage/CompositionImpedance;	
			}
		}
		else {
			double_complex CurrentComplex = GetNodeComplex(CurrentNode.NodeNumber);

			double CompositionImpedance = sqrt(pow(CurrentComplex.real(), 2) + pow(CurrentComplex.imag(), 2));
			FirstNodeElectric = m_fluidVoltage/CompositionImpedance;	
		}
		return FirstNodeElectric;
	}

	double RecusiveElectric = 0;	//�ٽ� ��� ȣ��
	list<CBaseElement*>::iterator pos = CurrentNode.InputElement.begin();
	while (pos != CurrentNode.InputElement.end()) {

		Node *SearchNode = NULL;
		if (IsAnotherTempNode(CurrentNode, (*pos), &SearchNode)) {
			RecusiveElectric += RecusiveParallelSearch(*SearchNode, (*pos));
		}
		++pos;
	}
	
	double CurrentNodeElectric = 0;	//�������ؼ� ��ȯ
	if (ParalleFlag) {	//���� ó��
		double_complex CurrentComplex = GetNodeComplex(CurrentNode.NodeNumber);

		double CompositionImpedance = sqrt(pow(CurrentComplex.real(), 2) + pow(CurrentComplex.imag(), 2));
		double NodeVoltage = RecusiveElectric*CompositionImpedance;

		if (CallElement != NULL) {
			
			DivideNode SearchNode;
			SearchNode.ElementNumber = CallElement->m_ElementNumber;
			list<DivideNode>::iterator dividepos = find_if(CurrentNode.DivideElement.begin(), CurrentNode.DivideElement.end(), SearchNode);
			if (dividepos != CurrentNode.DivideElement.end()) {
				
				double DivideImpedance = sqrt(pow((*dividepos).DivideComplexNumber.real(), 2) + pow((*dividepos).DivideComplexNumber.imag(), 2));
				CurrentNodeElectric = NodeVoltage/DivideImpedance;
				(*dividepos).DivideElectric = CurrentNodeElectric;
			}
		} 
		else {	//���κ� ���� ������ ���� �����϶�
			CurrentNodeElectric = RecusiveElectric;
		}

		if (CallElement != NULL) {
			//���� ������ ���� ���ؼ� ����
			list<DivideNode>::iterator dividenodepos = CurrentNode.DivideElement.begin();
			while (dividenodepos != CurrentNode.DivideElement.end()) {

				if ((*dividenodepos).ElementNumber != CallElement->m_ElementNumber) {

					double DivideImpedance = sqrt(pow((*dividenodepos).DivideComplexNumber.real(), 2) + pow((*dividenodepos).DivideComplexNumber.imag(), 2));
					double OtherNodeElectric = NodeVoltage/DivideImpedance;
					(*dividenodepos).DivideElectric = OtherNodeElectric;
				}
				++dividenodepos;
			}
		}
	}
	else {
		CurrentNodeElectric = RecusiveElectric;
	}

	return CurrentNodeElectric;
}

double_complex LinearACFormula::GetNodeComplex(int NodeNumber)
{
	list<IPCData>::iterator pos = m_IsopotentIPCList.begin();
	while (pos != m_IsopotentIPCList.end()) {
		
		if ((*pos).NodeNumber == NodeNumber) {
			return (*pos).ComplexNumber;
		}
		++pos;
	}
	return double_complex(0,0);
}



double LinearACFormula::VoltageDropping()
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

double LinearACFormula::GetCapacitance()
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

void LinearACFormula::RestitutionList()
{
	m_NodeList.clear();
	m_NodeList.resize(m_TempNodeList.size());
	copy(m_TempNodeList.begin(), m_TempNodeList.end(), m_NodeList.begin());

	m_ProvList.clear();
	m_ProvList.resize(m_TempProvList.size());
	copy(m_TempProvList.begin(), m_TempProvList.end(), m_ProvList.begin());
}

void LinearACFormula::SectionCompositonImpedance(list<IPCData> &Datalist)
{
	if (CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {
		//���� ������ ��� �ռ����� üũ
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
	}
	else { //������ ��� ���� �ռ����� üũ
		CElementFactory *pFactory = CMainFrame::CreateElementFactory();
		list<IPCData>::iterator pos = m_IsopotentIPCList.begin();
		while (pos != m_IsopotentIPCList.end()) {

			//�ش� �� �ʱ�ȭ
			m_NodeList.clear();
			m_NodeList.resize(m_IsopotentNodeList.size());
			copy(m_IsopotentNodeList.begin(), m_IsopotentNodeList.end(), m_NodeList.begin());

			list<Node>::iterator nodepos = m_NodeList.begin();
			while (nodepos != m_NodeList.end()) {

				if ((*pos).NodeNumber == (*nodepos).NodeNumber) {
					list<CBaseElement*>::iterator linepos = (*nodepos).LineElement.begin();
					CBaseElement *NewElement = pFactory->CreateElement(prov);
					NewElement->m_InputElement.push_back((*linepos));
					m_TempList.push_back(NewElement);

					m_ProvList.clear();
					m_ProvList.push_back(NewElement);
					break;
				}
				++nodepos;
			}

			//�׺�� ������ ���� � ��ȯ
			Thevenintheorem();

			//���κ� ��ġ �ռ� ���Ǵ��� 
			CompostionProvPostion(m_IsopotentIPCList);
			++pos;
		}
	}
}

double LinearACFormula::RecusiveMergerSearch(Node &CurrentNode, CBaseElement* CallElement)
{
	//�ڷ� �˻��ϸ鼭 ���ձ��� üũ

	double RecusiveElectric = 0;	//�ٽ� ��� ȣ��
	list<CBaseElement*>::iterator pos = CurrentNode.OutputElement.begin();
	while (pos != CurrentNode.OutputElement.end()) {

		Node *SearchNode = NULL;
		if (IsAnotherNode(CurrentNode, (*pos), &SearchNode)) {
			RecusiveElectric = RecusiveMergerSearch(*SearchNode, (*pos));
		}
		++pos;
	}

	if (CurrentNode.InputElement.size() >= 2) {

		int DivideCount = CurrentNode.InputElement.size();
		list<IPCData>::iterator searchpos = m_AllNodeDataList.begin();
		while (searchpos != m_AllNodeDataList.end()) {
			
			if ((*searchpos).NodeNumber == CurrentNode.NodeNumber) {
				break;
			}
			++searchpos;
		}

		if (searchpos != m_AllNodeDataList.end()) {

			double CompositionImpedance = sqrt(pow((*searchpos).ComplexNumber.real(), 2) + pow((*searchpos).ComplexNumber.imag(), 2));
			CompositionImpedance *= DivideCount;

		}
	}
	return 0;
}

double_complex LinearACFormula::GetMappingComplex(int NodeNumber)
{
	//�ش� ��� �ѹ� m_IsopotentIPCList & ���� ������ �˻�
	BOOL nFlag = FALSE;
	list<MappingNodeNumber>::iterator mappingpos = m_IsopotentMappingList.begin();
	while (mappingpos != m_IsopotentMappingList.end()) {

		if ((*mappingpos).BaseNumber == NodeNumber) {
			nFlag = TRUE;
			break;
		}
		++mappingpos;
	}

	double_complex CurrentComplex;
	if (nFlag) {
		CurrentComplex = GetNodeComplex((*mappingpos).CopyNumber);
	}
	else {
		CurrentComplex = GetNodeComplex(NodeNumber);
	}

	return CurrentComplex;
}
void LinearACFormula::Phasemovement (list<IPCData> &Datalist)
{
	list<IPCData>::iterator pos = Datalist.begin();
	while (pos != Datalist.end()) {

		if ((*pos).FirstFlag == FALSE) {	//���� ��带 ������ ������ ���� �̵�	

			double CurrentAngle = LocalPhaseAngle((*pos).NodeNumber);

			//Shift
			if (CurrentAngle > 0) {
				CurrentAngle *= MultSector;
				int ShiftCount = abs((int)CurrentAngle);

				double *ShiftData = new double[ShiftCount];
				memset(ShiftData, 0, sizeof(double)*ShiftCount);
				memcpy_s((char*)ShiftData, sizeof(double)*ShiftCount, (char*)(*pos).GrapehValue + sizeof(double)*(DivideSector - ShiftCount), sizeof(double)*ShiftCount);
				memcpy_s((char*)(*pos).GrapehValue + sizeof(double)*ShiftCount, sizeof(double)*(DivideSector - ShiftCount), (char*)(*pos).GrapehValue, sizeof(double)*(DivideSector - ShiftCount));
				memcpy_s((char*)(*pos).GrapehValue, sizeof(double)*ShiftCount, (char*)ShiftData, sizeof(double)*ShiftCount);

				delete [] ShiftData;
			}
			else if (CurrentAngle < 0) {
				CurrentAngle *= MultSector;
				int ShiftCount = abs((int)CurrentAngle);

				double *ShiftData = new double[ShiftCount];
				memset(ShiftData, 0, sizeof(double)*ShiftCount);
				memcpy_s((char*)ShiftData, sizeof(double)*ShiftCount, (char*)(*pos).GrapehValue, sizeof(double)*ShiftCount);
				memcpy_s((char*)(*pos).GrapehValue, sizeof(double)*(DivideSector - ShiftCount), (char*)(*pos).GrapehValue + sizeof(double)*ShiftCount, sizeof(double)*(DivideSector - ShiftCount));
				memcpy_s((char*)(*pos).GrapehValue + sizeof(double)*(DivideSector - ShiftCount), sizeof(double)*ShiftCount, (char*)ShiftData, sizeof(double)*ShiftCount);

				delete [] ShiftData;
			}
		}
		++pos;
	}
}

double LinearACFormula::LocalPhaseAngle(int NodeNumber)
{
	list<MappingNodeNumber>::iterator mappingpos = m_IsopotentMappingList.begin();
	while (mappingpos != m_IsopotentMappingList.end()) {
	
		if ((*mappingpos).BaseNumber == NodeNumber) {
			NodeNumber = (*mappingpos).CopyNumber;	
			break;
		}
		++mappingpos;
	}

	list<Node>::iterator nodepos = m_IsopotentNodeList.begin();
	while (nodepos != m_IsopotentNodeList.end()) {
		if ((*nodepos).NodeNumber == NodeNumber) {
			break;
		}
		++nodepos;
	}

	int ParalleCount = 0;
	if (mappingpos != m_IsopotentMappingList.end()) {
		NodeNumber = (*mappingpos).BaseNumber;
	}
	int DivideCount = GetDivideCount(NodeNumber);
	double_complex DivideComplex(0,0);

	return RecusiveGetParallelNode((*nodepos), TRUE, NULL, DivideComplex, ParalleCount, DivideCount);
}

double LinearACFormula::RecusiveGetParallelNode(Node &CurrentNode, BOOL StartFlag, CBaseElement *CallElement, double_complex &DivideComplex, int &ParallelCount, int DivideCount)
{
	//DivdeCount �������̺� ����
	//ParallelCount ���� ��ġ üũ
	//DivideComplex ���� ���ı��� ���� �ռ����� üũ

	BOOL MergerFlag = FALSE;
	BOOL ParallelFlag = FALSE;
	if (!StartFlag) {
		if (CurrentNode.OutputElement.size() >= 2) {
			ParallelCount++;	//���ı��� üũ
			ParallelFlag = TRUE;
		}

		if (DivideCount >= 2) {	//����+�ռ������̹Ƿ� ���ķ� �ν��Ѵ�. 
			DivideCount -= CurrentNode.OutputElement.size();
			ParallelFlag = FALSE;
			ParallelCount--;
		}
	}
	else {
		DivideCount >= 2 ? MergerFlag = TRUE : NULL;
	}

	if (CurrentNode.FirstFlag) {
		double_complex CurrentComplex = GetNodeComplex(CurrentNode.NodeNumber);
		double CurrentAngle = atan(CurrentComplex.imag()/ CurrentComplex.real());
		CurrentAngle *= RADTOANGLE;

		if (CurrentComplex.real() == 0) { //R ������ 0�϶� ����ó��
			CurrentComplex.imag() > 0 ? CurrentAngle = 90 : CurrentAngle = -90;
		}

		//ù��� �̸鼭 ���ı����϶� DivideComplex ó�� �ʿ�
		if (ParallelFlag) {
			ParallelCount--;
			double_complex ParallelComplex = GetNodeComplex(CurrentNode.NodeNumber);
			double ParallelAngle = atan(ParallelComplex.imag()/ ParallelComplex.real());
			ParallelAngle *= RADTOANGLE;

			if (ParallelComplex.real() == 0) { //R ������ 0�϶� ����ó��
				ParallelComplex.imag() > 0 ? ParallelAngle = 90 : ParallelAngle = -90;
			}
			CurrentAngle -= ParallelAngle;

			if (ParallelCount == 0) {
				DivideNode SearchNode;
				SearchNode.ElementNumber = CallElement->m_ElementNumber;
				list<DivideNode>::iterator dividepos = find_if(CurrentNode.DivideElement.begin(), CurrentNode.DivideElement.end(), SearchNode);
				if (dividepos != CurrentNode.DivideElement.end()) {
					DivideComplex = (*dividepos).DivideComplexNumber;	
				}
			}
		}
		return CurrentAngle;
	}

	Node *SearchNode = NULL;
	double TransAngle = 0;

	list<CBaseElement*>::iterator elementpos = CurrentNode.InputElement.begin();
	if (IsAnotherTempNode(CurrentNode, (*elementpos), &SearchNode)) {
		TransAngle = RecusiveGetParallelNode(*SearchNode, FALSE, (*elementpos), DivideComplex, ParallelCount, DivideCount);
	}

	if (ParallelFlag) {
		ParallelCount--;
		double_complex CurrentComplex = GetNodeComplex(CurrentNode.NodeNumber);
		double CurrentAngle = atan(CurrentComplex.imag()/ CurrentComplex.real());
		CurrentAngle *= RADTOANGLE;

		if (CurrentComplex.real() == 0) { //R ������ 0�϶� ����ó��
			CurrentComplex.imag() > 0 ? CurrentAngle = 90 : CurrentAngle = -90;
		}
		TransAngle -= CurrentAngle;

		if (ParallelCount == 0) {
			DivideNode SearchNode;
			SearchNode.ElementNumber = CallElement->m_ElementNumber;
			list<DivideNode>::iterator dividepos = find_if(CurrentNode.DivideElement.begin(), CurrentNode.DivideElement.end(), SearchNode);
			if (dividepos != CurrentNode.DivideElement.end()) {
				DivideComplex = (*dividepos).DivideComplexNumber;	
			}
		}
	}

	if (CallElement == NULL) {	//���� ��ġ�� �ٽ� ������ ���
		double_complex CurrentComplex = GetNodeComplex(CurrentNode.NodeNumber);

		if (MergerFlag) {	//����+���� ���� ����ó��
			CurrentComplex /= GetDivideCount(CurrentNode.NodeNumber);
		}

		double CurrentAngle = atan(CurrentComplex.imag()/ CurrentComplex.real());
		CurrentAngle *= RADTOANGLE;
		
		if (CurrentComplex.real() == 0) { //R ������ 0�϶� ����ó��
			CurrentComplex.imag() > 0 ? CurrentAngle = 90 : CurrentAngle = -90;
		}

		if (DivideComplex.real() == 0 && DivideComplex.imag() == 0) {	//������
			TransAngle -= CurrentAngle;
		}
		else {	//���� ���� ����
			double DivideAngle = atan(DivideComplex.imag()/ DivideComplex.real());
			DivideAngle *= RADTOANGLE;
			if (DivideComplex.real() == 0) { //R ������ 0�϶� ����ó��
				DivideComplex.imag() > 0 ? DivideAngle = 90 : DivideAngle = -90;
			}

			TransAngle += (DivideAngle - CurrentAngle);
		}
	}
	return TransAngle;
}

// double_complex LinearACFormula::RecusiveGetParallelNode(Node &CurrentNode, BOOL StartFlag, CBaseElement *CallElement, int DivideCount)
// {
// 	if (!StartFlag) {
// 		if (CurrentNode.OutputElement.size() >= 2) {
// 
// 			if (--DivideCount == 0) {
// 				list<Node>::iterator nodepos = m_IsopotentNodeList.begin();
// 				while (nodepos != m_IsopotentNodeList.end()) {
// 					if ((*nodepos).NodeNumber == CurrentNode.NodeNumber) {
// 						break;
// 					}
// 					++nodepos;
// 				}
// 
// 				double_complex LocalImpedance(0, 0);
// 				DivideNode SearchNode;
// 				SearchNode.ElementNumber = CallElement->m_ElementNumber;
// 				list<DivideNode>::iterator dividepos = find_if(CurrentNode.DivideElement.begin(), CurrentNode.DivideElement.end(), SearchNode);
// 				if (dividepos != CurrentNode.DivideElement.end()) {
// 					LocalImpedance = (*dividepos).DivideComplexNumber;	
// 				}
// 				return LocalImpedance;	
// 			}
// 		}
// 	}
// 
// 	if (CurrentNode.FirstFlag) {
// 		return GetNodeComplex(CurrentNode.NodeNumber);
// 	}
// 
// 	list<CBaseElement*>::iterator elementpos = CurrentNode.InputElement.begin();
// 
// 	Node *SearchNode = NULL;
// 	if (IsAnotherTempNode(CurrentNode, (*elementpos), &SearchNode)) {
// 		return RecusiveGetParallelNode(*SearchNode, FALSE, (*elementpos), DivideCount);
// 	}
// 
// 	return double_complex(0, 0);
// }


void LinearACFormula::CompostionProvPostion(list<IPCData> &Datalist, BOOL nFlag /*= FALSE*/)
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

void LinearACFormula::CircultBackUpList()
{
	m_TempNodeList.clear();
	m_TempNodeList.resize(m_NodeList.size());
	copy(m_NodeList.begin(), m_NodeList.end(), m_TempNodeList.begin());

	m_TempProvList.clear();
	m_TempProvList.resize(m_ProvList.size());
	copy(m_ProvList.begin(), m_ProvList.end(), m_TempProvList.begin());
}

void LinearACFormula::Thevenintheorem()
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

BOOL LinearACFormula::IsProvArea(Node &CurrentNode)
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

complex<double> LinearACFormula::GetElementInpedance(CBaseElement *pElement)
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

void LinearACFormula::GetFamilyNode(Node &CurrentNode, CBaseElement *CompareElement, Node **ResultNode)
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

int LinearACFormula::DeleteElementInNode(Node &CurrentNode, CBaseElement *pElement)
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

void LinearACFormula::SeriesMerger(BOOL &OperationFlag)
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
				NewElement->m_ElementNumber = (*FirstElement)->m_ElementNumber;

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

BOOL LinearACFormula::IsAnotherTempNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode)
{
	//���� ����� �ڸ� Ž���ϸ鼭 ���� ���� �ֳ� üũ

	list<Node>::iterator pos = m_IsopotentNodeList.begin();
	while (pos != m_IsopotentNodeList.end()) {

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

BOOL LinearACFormula::IsAnotherNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode)
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

void LinearACFormula::RarallelMerger(BOOL &OperationFlag)
{
	//�⺻������ ���κ갡 ���Ե� ������ �����Ѵ�.
	//������ ���ı����� �����Ѵ�. �� ������ ������ ���ڵ��� ����� �ٸ� ��尡 ����� �Ѵ�.

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {
	
		if (/*IsProvArea((*nodepos)) == FALSE*/TRUE) {	//���κ� ������ �ƴҶ� //���� ��� ���� ���� ���κ� ������ ���Ŀ����� ����
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
					InsertDivideElement((*nodepos).NodeNumber, (*FirstElement), FirstInpedance);

					list<CBaseElement*>::iterator SecondElement = Mergerlist.begin();
					++SecondElement;
					double_complex SecondInpedance(0,0);
					SecondInpedance = GetElementInpedance((*SecondElement));
					InsertDivideElement((*nodepos).NodeNumber, (*SecondElement), SecondInpedance);

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

void LinearACFormula::InsertDivideElement(int NodeNumber, CBaseElement *pElement, double_complex &Complex)
{
	Node *TempNode = GetTempNode(NodeNumber);

	if (TempNode != NULL) {
		DivideNode NewDivideNode;
		NewDivideNode.ElementNumber = pElement->m_ElementNumber;
		NewDivideNode.DivideComplexNumber = Complex;

		list<DivideNode>::iterator Duplicatepos = find_if(TempNode->DivideElement.begin(), TempNode->DivideElement.end(), NewDivideNode);
		if (Duplicatepos == TempNode->DivideElement.end()) {
			TempNode->DivideElement.push_back(NewDivideNode);
		}
	}
}

Node* LinearACFormula::GetTempNode(int Nodenumber)
{
	list<Node>::iterator nodepos = m_IsopotentNodeList.begin();
	while (nodepos != m_IsopotentNodeList.end()) {

		if ((*nodepos).NodeNumber == Nodenumber) {
			return &(*nodepos);
		}
		++nodepos;
	}
	return NULL;
}

void LinearACFormula::GetDuplicateNode(Node &CurrentNode, list<CBaseElement*> &MergerList)
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



void LinearACFormula::DuplicateMerger(BOOL &OperationFlag)
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
//					InsertDivideElement((*nodepos).NodeNumber, (*FirstElement), FirstInpedance);

					list<CBaseElement*>::iterator SecondElement = Mergerlist.begin();
					++SecondElement;
					double_complex SecondInpedance(0,0);
					SecondInpedance = GetElementInpedance((*SecondElement));
//					InsertDivideElement((*nodepos).NodeNumber, (*SecondElement), SecondInpedance);

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


void LinearACFormula::Initialize(list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
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

				//���� ���� ������ ����� IPCData ����
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

	//��� ����� IPCData����
	{
		list<Node>::iterator nodepos = m_NodeList.begin();
		while (nodepos != m_NodeList.end()) {
			IPCData NewData;
			NewData.ComplexCheckFlag = FALSE;
			NewData.NodeNumber = (*nodepos).NodeNumber;
			(*nodepos).FirstFlag == TRUE ? NewData.FirstFlag = TRUE : NewData.FirstFlag = FALSE;
			m_AllNodeDataList.push_back(NewData);
			++nodepos;
		}
	}
}



void LinearACFormula::IPCDataConvert(list<IPCData> &Datalist, int iCount)
{
	list<IPCData>::iterator pos = Datalist.begin();
	while (pos != Datalist.end()) {
		(*pos).GrapehValue[iCount] = m_Result[(*pos).NodeNumber];
		++pos;
	}
}

void LinearACFormula::LinearStencilConvert(double fluidVoltage)
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


double LinearACFormula::GetCombinedResistance(Node &CurrentNode)
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

double LinearACFormula::GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
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

double LinearACFormula::GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
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

double LinearACFormula::GetElementProperty(CBaseElement *pElement, CString &PropertyString)
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

int LinearACFormula::IsDuplicateNode(Node &BaseNode, CBaseElement *pElement)
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

BOOL LinearACFormula::GaussianElimination()
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

BOOL LinearACFormula::OPAmpProcessing(list<IPCData> &VoltageList, list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
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

	//Capacity, Inductor -> Insert Frequency;
	AheadProcessing(Datalist);

	//������ 
	m_IsopotentNodeList.clear();
	m_IsopotentNodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_IsopotentNodeList.begin());

	//������ ��ȯ
	ConvertIsopotentialNode();

	//ProveList, NodeList Backup
	CircultBackUpList();

	//All Section Composition Impedance
	SectionCompositonImpedance(Datalist);			//���ο� �׺�� ���� ����

	//ProveList, NodeList, ReBackUp
	RestitutionList();

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
			else { // rc, rl, rlc //������ ȸ���϶��� ������ Ư���� ����

				list<IPCData>::iterator FirstNodepos = m_IsopotentIPCList.begin();

				double FirstCompositionImpedance = sqrt(pow((*FirstNodepos).ComplexNumber.real(), 2) + pow((*FirstNodepos).ComplexNumber.imag(), 2));
				double ConvertAlternating = fluidVoltage/FirstCompositionImpedance;	//���� ���� ����

				list<IPCData>::iterator pos = Datalist.begin();
				while (pos != Datalist.end()) {

					double LocalElectric = GetNodeElectric((*pos).NodeNumber);
					double_complex LocalComplex = GetMappingComplex((*pos).NodeNumber);

					double CurrentCompositionImpedance = sqrt(pow(LocalComplex.real(), 2) + pow(LocalComplex.imag(), 2));
					int DivideCount = GetDivideCount((*pos).NodeNumber);
					CurrentCompositionImpedance /= DivideCount;
					double ConvertVoltage = LocalElectric*CurrentCompositionImpedance;

					(*pos).GrapehValue[(int)iCount] = ConvertVoltage;
					++pos;
				}
			}
		}
	}

	//���� �̵�
	Phasemovement(Datalist);

	//��ó�� (���� RLC ���� �̵����� ���� MaxVolate ������ ��ȯ)
	//�ﰢ�� ���ý� (������ -> �ﰢ��)
	RearProcessing(Datalist);

	return TRUE;
}