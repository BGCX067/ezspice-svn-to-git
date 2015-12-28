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

	//TempList 삭제
	list<CBaseElement*>::iterator pos = m_TempList.begin();
	while (pos != m_TempList.end()) {
		delete (*pos);
		++pos;
	}
}

void LinearACFormula::AheadProcessing(list<IPCData> &Datalist)
{
	//병렬구간 체크
	{
		list<Node>::iterator pos = m_NodeList.begin();
		while (pos != m_NodeList.end()) {
		
			if ((*pos).OutputElement.size() >= 2) {
				(*pos).DivideFlag = TRUE;
			}
			++pos;
		}
	}

	//모든 인피던스 소자에 주파수 삽입
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
	
	if (CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {	//rc회로 이면서 구형파 일때 전압을 0k 저항으로 교체 및 프로브 위치 변경
	
		//AC전압 찾기
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


		//프로브 위치 변경 (캐패시터 주변으로 2개)
		m_ProvList.clear();
		nodepos = m_NodeList.begin();
		while (nodepos != m_NodeList.end()) {

			list<CBaseElement*>::iterator elementpos = (*nodepos).AllElement.begin();
			while (elementpos != (*nodepos).AllElement.end()) {
				if ((*elementpos)->m_Flag == capacity) {

					//캐패시터 앞뒤로 프로브 삽입

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

void LinearACFormula::RearProcessing(list<IPCData> &Datalist)	//후처리
{
	//후처리 (만약 RLC 위상 이동으로 인해 MaxVolate 오버시 변환)
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


	//삼각파 선택시 (정현파 -> 삼각파)
	if (CCircultSimulation::m_ACData.WaveModel == triangular) {
		//데이터는 3600개 
		list<IPCData>::iterator pos = Datalist.begin();
		while (pos != Datalist.end()) {
			
			//첫구간과 다음 구간 비교후 시작 정의
			BOOL UpDownFlag = FALSE;
			(*pos).GrapehValue[0] < (*pos).GrapehValue[1] ? UpDownFlag = TRUE : UpDownFlag = FALSE;

			int LineToCount = 0;
			double LineTopos = (*pos).GrapehValue[0];
			double MoveTopos = (*pos).GrapehValue[1];
			for (int iCount = 1; iCount < DivideSector; ++iCount) {
				
				MoveTopos = (*pos).GrapehValue[iCount];
				if (UpDownFlag) {	//시작이 증가
					if (LineTopos > MoveTopos || iCount == DivideSector -1) {	//한번 꺽이는 지점
						//기울기
						double Slope = ((*pos).GrapehValue[iCount] - (*pos).GrapehValue[LineToCount])/(iCount - LineToCount);
						double PlusValue = (*pos).GrapehValue[iCount] - Slope*iCount;

						for (int jCount = LineToCount; jCount < iCount; jCount++) {
							(*pos).GrapehValue[jCount] = Slope*jCount + PlusValue;
						}
						LineToCount = iCount;
						UpDownFlag = FALSE;
					}
				}		
				else {	//시작이 감소
					if (LineTopos < MoveTopos || iCount == DivideSector -1) {	//한번 꺽이는 지점

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

			//등전위 검사
			if ((*Nodepos).InputElement.size() >= 2) {	//input이 두개면 뒤에 노드 전체를 병렬로 변경 및 등전위 

				//하위 노드에 등전위가 또 있나?
				if (RecusiveAnotherDuplicateMerger((*Nodepos), TRUE, LowNodeList) == FALSE) {	//등전위가 없기때문에 등전위 변환
					//생성된 모든 하위노드 값 증가 및 삽입
					IncreaseNodeImpedance((*Nodepos), LowNodeList);
					IsopotentialFlag = TRUE;
					break;
				}
			}
			++Nodepos;
		}

		//기존 데이터 삭제
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


	//모든 노드의 IPCData생성
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
							AfxMessageBox(_T("등전위 변환 예외 발생"));
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
							AfxMessageBox(_T("등전위 변환 예외 발생"));
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

	//등전위 
	m_IsopotentNodeList.clear();
	m_IsopotentNodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_IsopotentNodeList.begin());

	//등전위 변환
	ConvertIsopotentialNode();

	//ProveList, NodeList Backup
	CircultBackUpList();

	//All Section Composition Impedance
	SectionCompositonImpedance(Datalist);			//내부에 테브닌 정리 포함
	
	//ProveList, NodeList, ReBackUp
	RestitutionList();

	if (CCircultSimulation::m_ACData.Amplitude != -1) {	//값이 없으면 셋다 -1일 들어온다.

		BOOL RCInitializeFlag = FALSE;
		int	RecoveryCount = 0;
		double Vm = 0, Vpp = 0, Timeconstant = 0;
		double fluidAnniversary = 0, CurrentAnniVersary = 0;
		double PlusMoveMinus = 0, MinusMovePlus = 0;	

		double fluidFrequency = (1/CCircultSimulation::m_ACData.Frequency)/DivideSector;
		for (double iCount = 0; iCount < DivideSector; iCount++) {

			double fluidVoltage = CCircultSimulation::m_ACData.Amplitude*sin(2*PIE*CCircultSimulation::m_ACData.Frequency*(fluidFrequency*iCount));
			m_fluidVoltage = fluidVoltage;

			if (CCircultSimulation::m_RLCCase == rr) {	//일반적인 R-R 회로 일때는 스텐실을 적용한 가우시안 소거법 적용
				//stencil convert (matrix)
				LinearStencilConvert(fluidVoltage);

				//matrix -> result
				GaussianElimination();

				//result->Data
				IPCDataConvert(Datalist, (int)iCount);
			}
			else { // rc, rl, rlc //나머지 회로일때는 전류의 특성을 적용

				if (CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {	//R-C 회로 구형파 일때
					
					//1. 전압강하를 통한 새로운 Max전압 체크(vm)
					//2. Vm, Vp-p 계산 (Vm은 abs(Max전압), Vp-p는 (2*Vm)
					//3. 시정수 체크 (그전에 양반향 진행방향을 통한 테브닌 정리 수행)
					//4. 초기 작업을 통한 상승, 하강 초기값 체크
					//5. 수식 대입
					// - -> +
					//초기값 + (Vpp - (vm + 초기값)) * (1-Exp(-t/rc)) // 초기값 MinusMovePlus
					// + -> -
					//초기값 + (Vpp + (vm - 초기값)) * (1-Exp(-t/rc)) // 초기값 PlusMoveMinus


					if (RCInitializeFlag == FALSE) {
						//전압강하를 통한 Max전압 체크
						
						Vm = VoltageDropping();
						Vpp = 2*abs(Vm);

						//시정수 체크
						double Capacitanece = GetCapacitance();
						double CompositionResistance = 0;

						list<IPCData>::iterator ipcpos = Datalist.begin();
						while (ipcpos != Datalist.end()) {
							CompositionResistance += ipcpos->ComplexNumber.real();	
							++ipcpos;
						}
						Timeconstant = Capacitanece * CompositionResistance;

						//초기 작업을 통한 상승, 하강 초기값 체크
						
						fluidAnniversary = (1/CCircultSimulation::m_ACData.Frequency)/(DivideSector/2);	//주기 추출
						double CurrentAnniVersary = 0;
						for (int jCount = 0; jCount <DivideSector/2; ++jCount) {	//상승
							PlusMoveMinus = (Vpp - Vm)*(1-exp(-CurrentAnniVersary/Timeconstant));
							CurrentAnniVersary += fluidAnniversary;
						}

						CurrentAnniVersary = 0;
						for (int jCount = DivideSector/2; jCount < DivideSector; ++jCount) { //하강
							MinusMovePlus = PlusMoveMinus + (-Vpp + (Vm - PlusMoveMinus))*(1-exp(-CurrentAnniVersary/Timeconstant));
							CurrentAnniVersary += fluidAnniversary;
						}

						//IPCDataList 갯수 한개로 변경
						if (Datalist.size() > 1) {
							
							int DeleteCount = Datalist.size() -1;
							for (int iCount = 0; iCount < DeleteCount; iCount++) {
								Datalist.pop_back();
							}
						}

						//첫번째 값 셋팅
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
					double ConvertAlternating = fluidVoltage/FirstCompositionImpedance;	//현재 통합 전류

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
		//위상 이동
		Phasemovement(Datalist);
	}
	

	//후처리 (만약 RLC 위상 이동으로 인해 MaxVolate 오버시 변환)
	//삼각파 선택시 (정현파 -> 삼각파)
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
	//현노드의 앞쪽으로 재귀하며 검색
	//일단 병합구간이 없다고 가정

	BOOL ParalleFlag = FALSE;
	if (CurrentNode.OutputElement.size() >= 2) { //병렬 체크
		ParalleFlag = TRUE;
	}

	if (CurrentNode.FirstFlag) {	//처음 노드 일경우 
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

				//남은 구간도 전류 구해서 삽입
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
			else { //프로브 위치가 처음 노드일때
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

	double RecusiveElectric = 0;	//다시 재귀 호출
	list<CBaseElement*>::iterator pos = CurrentNode.InputElement.begin();
	while (pos != CurrentNode.InputElement.end()) {

		Node *SearchNode = NULL;
		if (IsAnotherTempNode(CurrentNode, (*pos), &SearchNode)) {
			RecusiveElectric += RecusiveParallelSearch(*SearchNode, (*pos));
		}
		++pos;
	}
	
	double CurrentNodeElectric = 0;	//전류구해서 반환
	if (ParalleFlag) {	//병렬 처리
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
		else {	//프로브 찍은 구간이 병렬 구간일때
			CurrentNodeElectric = RecusiveElectric;
		}

		if (CallElement != NULL) {
			//남은 구간도 전류 구해서 삽입
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
	//AC전압원을 DC전압원으로 교체 후 선형 DC 알고리즘 수행하여 캐패시터 전위치의 전압체크
	//캐패시터 전에 새로운 프로브 삽입 

	//현재의 노드 연결 정보 저장 (연결정보가 달라지므로 - 선형 DC 한번 수행함 아래에서)
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

	//AC전압 -> DC 전압 및 회로 초기 상태 변환
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
		else if ((*elementpos)->m_Flag == capacity) {//새로운 프로브 생성(캐패시터 앞뒤)

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
		AfxMessageBox(_T("개발자한테 전화해주세요!!!"));
		return -1;
	}


	list<IPCData> NewDataList;
	LinearDCFormula NewFormula;
	if (NewFormula.Processing(NewNodeList, NewDataList, NewProvList) == FALSE) {
		AfxMessageBox(_T("개발자한테 전화해주세요!!!"));
		return -1;
	}

	// acpower 제거하면서 삭제한 라인에 다시 복구
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

	//원래 진행방향으로 변경
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

	//값 반환
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
		//실제 데이터 노드 합성구항 체크
		//CompositionResistance 프로브 별로 나눠서 합성저항 구하고 적용
		UINT RepetitionCount = m_ProvList.size();
		for (UINT iCount = 0; iCount < RepetitionCount; iCount++) {

			//해당 값 초기화
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

			//테브닌 정리를 통해 등가 교환
			Thevenintheorem();

			//프로브 위치 합성 인피던스 
			CompostionProvPostion(Datalist);
		}
	}
	else { //등전위 모든 구간 합성저항 체크
		CElementFactory *pFactory = CMainFrame::CreateElementFactory();
		list<IPCData>::iterator pos = m_IsopotentIPCList.begin();
		while (pos != m_IsopotentIPCList.end()) {

			//해당 값 초기화
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

			//테브닌 정리를 통해 등가 교환
			Thevenintheorem();

			//프로브 위치 합성 인피던스 
			CompostionProvPostion(m_IsopotentIPCList);
			++pos;
		}
	}
}

double LinearACFormula::RecusiveMergerSearch(Node &CurrentNode, CBaseElement* CallElement)
{
	//뒤로 검색하면서 병합구간 체크

	double RecusiveElectric = 0;	//다시 재귀 호출
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
	//해당 노드 넘버 m_IsopotentIPCList & 매핑 정보로 검색
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

		if ((*pos).FirstFlag == FALSE) {	//시작 노드를 제외한 나머지 위상 이동	

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
	//DivdeCount 매핑테이블 갯수
	//ParallelCount 병렬 위치 체크
	//DivideComplex 최초 병렬구간 분할 합성저항 체크

	BOOL MergerFlag = FALSE;
	BOOL ParallelFlag = FALSE;
	if (!StartFlag) {
		if (CurrentNode.OutputElement.size() >= 2) {
			ParallelCount++;	//병렬구간 체크
			ParallelFlag = TRUE;
		}

		if (DivideCount >= 2) {	//분할+합성구간이므로 직렬로 인식한다. 
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

		if (CurrentComplex.real() == 0) { //R 성분이 0일때 예외처리
			CurrentComplex.imag() > 0 ? CurrentAngle = 90 : CurrentAngle = -90;
		}

		//첫노드 이면서 병렬구간일때 DivideComplex 처리 필요
		if (ParallelFlag) {
			ParallelCount--;
			double_complex ParallelComplex = GetNodeComplex(CurrentNode.NodeNumber);
			double ParallelAngle = atan(ParallelComplex.imag()/ ParallelComplex.real());
			ParallelAngle *= RADTOANGLE;

			if (ParallelComplex.real() == 0) { //R 성분이 0일때 예외처리
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

		if (CurrentComplex.real() == 0) { //R 성분이 0일때 예외처리
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

	if (CallElement == NULL) {	//시작 위치로 다시 도착시 계산
		double_complex CurrentComplex = GetNodeComplex(CurrentNode.NodeNumber);

		if (MergerFlag) {	//병렬+병합 구간 예외처리
			CurrentComplex /= GetDivideCount(CurrentNode.NodeNumber);
		}

		double CurrentAngle = atan(CurrentComplex.imag()/ CurrentComplex.real());
		CurrentAngle *= RADTOANGLE;
		
		if (CurrentComplex.real() == 0) { //R 성분이 0일때 예외처리
			CurrentComplex.imag() > 0 ? CurrentAngle = 90 : CurrentAngle = -90;
		}

		if (DivideComplex.real() == 0 && DivideComplex.imag() == 0) {	//직렬임
			TransAngle -= CurrentAngle;
		}
		else {	//병렬 포함 구간
			double DivideAngle = atan(DivideComplex.imag()/ DivideComplex.real());
			DivideAngle *= RADTOANGLE;
			if (DivideComplex.real() == 0) { //R 성분이 0일때 예외처리
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

					if ((*nodepos).OutputElement.size() == 1) {	//단일 저항(직렬)

						list<CBaseElement*>::iterator Elementpos = (*nodepos).OutputElement.begin();
						(*ipcpos).ComplexNumber = GetElementInpedance((*Elementpos));
					}
					else {	//다중 저항(병렬)

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
		SeriesMerger(OperationFlag);	//직렬 병합
		RarallelMerger(OperationFlag);	//병렬 병합(최하위)
		DuplicateMerger(OperationFlag);	//병렬 병합(두개의 소자의 출,입구 동일)
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
		
		if ((*pos).NodeNumber != CurrentNode.NodeNumber) {	//중복 아닐때
			
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
	//기본적으로 프로브가 포함된 구간은 제외한다.
	//직렬저항시 두개의 저항을 하나로 합치고 하나의 소자를 제거한다.
	//중복처리를 막기 위해 한번 수행 후 종료
	//시작 노드는(전압원 포함 노드) 직렬 병합 제외

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		if ((*nodepos).FirstFlag == FALSE && IsProvArea((*nodepos)) == FALSE) {		//프로브 구간이 아닐때
			if ((*nodepos).InputElement.size() == 1 && (*nodepos).OutputElement.size() == 1) {	//직렬 구간일때
					
				//임피던스 추출
				double_complex CompositionResistance(0,0);
				list<CBaseElement*>::iterator FirstElement = (*nodepos).InputElement.begin();
				CompositionResistance += GetElementInpedance((*FirstElement));

				list<CBaseElement*>::iterator SecondElement = (*nodepos).OutputElement.begin();
				CompositionResistance += GetElementInpedance((*SecondElement));


				//현재 노드를 제외한 좌우 노드 탐색후 해당 소자들 삭제
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

				//새로운 소자 삽입 및 가족 노드 두개에 연결
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
	
				//해당 노드 삭제
				m_NodeList.erase(nodepos);

				//임시 리스트에 저장, 위상 적용후 삭제 하기 위해
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
	//현재 노드의 뒤를 탐색하면서 병합 구간 있나 체크

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
	//기본적으로 프로브가 포함된 구간은 제외한다.
	//최하위 병렬구간을 병합한다. 단 조건이 병합할 소자들이 연결된 다른 노드가 없어야 한다.

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {
	
		if (/*IsProvArea((*nodepos)) == FALSE*/TRUE) {	//프로브 구간이 아닐때 //정정 모든 구역 병합 프로브 구간은 직렬에서만 막음
			if ((*nodepos).OutputElement.size() >= 2) {	//병렬 구간일때

				DuplicateNode fflushNode;
				list<CBaseElement*> Mergerlist;
				list<CBaseElement*>::iterator outputpos = (*nodepos).OutputElement.begin();
				while (outputpos != (*nodepos).OutputElement.end()) {
					if (IsAnotherNode((*nodepos), (*outputpos), &fflushNode.AnotherNode) == FALSE) {
						Mergerlist.push_back((*outputpos));
					}
					++outputpos;
				}

				if (Mergerlist.size() >= 2) {	//병렬 소자가 속한 구간이 없을경우

					//인피던스 추출
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

					//현재 노드에서 두개의 소자 제거
					DeleteElementInNode((*nodepos), (*FirstElement));
					DeleteElementInNode((*nodepos), (*SecondElement));

					//새로운 소자 생성후 합성 인피던스 적용
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

					//현재 노드에 새로운 소자 연결
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
	//outputElement들이 현재 노드를 제외한 다른 노드를 공유검색
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
	//기본적으로 프로브가 포함된 구간은 제외한다.
	//노드가 병렬일때 각 소자들이 연결된 다른 노드가 중복일때 중복병합
	//RarallelMerger()와 비슷하나, 노드 연결이 차이 있음

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		if (/*IsProvArea((*nodepos)) == FALSE*/TRUE) {	//프로브 구간이 아닐때
			if ((*nodepos).OutputElement.size() >= 2) {	//병렬 구간일때

				list<CBaseElement*> Mergerlist;
				GetDuplicateNode((*nodepos), Mergerlist);

				if (Mergerlist.size() >= 2) {

					//인피던스 추출
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

					//현재 노드를 제외한 좌우 노드 탐색후 해당 소자들 삭제
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

					//현재 노드에서 두개의 소자 제거
					DeleteElementInNode((*nodepos), (*FirstElement));
					DeleteElementInNode((*nodepos), (*SecondElement));

					//새로운 소자 생성후 합성 인피던스 적용
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

					//FirstFamilyNode와 SecondFamilyNode는 100%일치 해야 한다.
					if (FirstFamilyNode == SecondFamilyNode) {	//중복된 다른 노드에도 새로운 소자 삽입
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
			list<CBaseElement*>::iterator CompareElement = (*provpos)->m_InputElement.begin();	//프로브 위치

			list<Node>::iterator nodepos = m_NodeList.begin();
			while (nodepos != m_NodeList.end()) {

				//실제 전달 데이터 노드의 IPCData 생성
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

	//모든 노드의 IPCData생성
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

	//여기서 중요한점은 합성저항을 적용해야 한다는것이다
	//일반 저항은 1 / R1 로 계산을 하며 
	//합성 저항은 각각의 저항을 일반저항으로 계산후 더해야 한다. (일반 합성저항 공식과 다르다)
	//또한 스텐실 기법중에 현재 위치의 왼쪽과 오른쪽에 대한 값들은 해당소자들중 다른곳에 속한것만 적용한다.
	//가운데는 모두 적용이다.

	int LineNumber = 0;
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		if ((*pos).FirstFlag) {		//시작 위치 노드이면 상수값 적용			
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
	//해당 객체의 데이터 추출
	list<ElementData> DataList;			
	pElement->GetProperties(DataList);

	//데이터 추출후 스텐실에 적용
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
	x= (double*)malloc(n*sizeof(double));    // Ax=b 에서 x(즉, 우리가 구하려는 값.전압)
	x--;
	a= (double**)malloc(n*sizeof(double*));   // Ax=b 에서 A와 b(matrix의 원소,저항성분)
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
			if(fabs(a[i][i])<eps){            //matrix가 해를 가지는 조건인지를 검사.
				TRACE("Error: 행렬의 대각원소가 0입니다.\n");
				return FALSE;
			} 
			a[j][i]=a[j][i]/a[i][i];
			for(k=ip;k<=np;k++)
				a[j][k]= a[j][k]-a[j][i]*a[i][k]; // (가우스 소거법)Gaussian elimination  
		}
	}

	x[n]=a[n][np]/a[n][n];
	for(i=n-1;i>0;i--){
		sum=0.;
		for(j=i+1;j<=n;j++) 
			sum+=a[i][j]*x[j];
		x[i]=(a[i][np]-sum)/a[i][i]; // 후진 대입법(backward substitution)
	}

	m_Result = new double[n];
	memset(m_Result, 0, sizeof(double)*n);
	for(i=1;i<=n;i++)
		m_Result[i-1] = x[i];   //결과 도출.


	//메모리 해제
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



////////////////////////////////////////////////////////////////////////// opamp를 위한 번외

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

	//등전위 
	m_IsopotentNodeList.clear();
	m_IsopotentNodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_IsopotentNodeList.begin());

	//등전위 변환
	ConvertIsopotentialNode();

	//ProveList, NodeList Backup
	CircultBackUpList();

	//All Section Composition Impedance
	SectionCompositonImpedance(Datalist);			//내부에 테브닌 정리 포함

	//ProveList, NodeList, ReBackUp
	RestitutionList();

	if (CCircultSimulation::m_ACData.Amplitude != -1) {	//값이 없으면 셋다 -1일 들어온다.

		list<IPCData>::iterator Datapos = VoltageList.begin();
		for (double iCount = 0; iCount < DivideSector; iCount++) {

			double fluidVoltage = (*Datapos).GrapehValue[(int)iCount];
			if (CCircultSimulation::m_RLCCase == rr) {					//일반적인 R-R 회로 일때는 스텐실을 적용한 가우시안 소거법 적용
				//stencil convert (matrix)
				LinearStencilConvert(fluidVoltage);
				//matrix -> result
				GaussianElimination();

				//result->Data
				IPCDataConvert(Datalist, (int)iCount);
			}
			else { // rc, rl, rlc //나머지 회로일때는 전류의 특성을 적용

				list<IPCData>::iterator FirstNodepos = m_IsopotentIPCList.begin();

				double FirstCompositionImpedance = sqrt(pow((*FirstNodepos).ComplexNumber.real(), 2) + pow((*FirstNodepos).ComplexNumber.imag(), 2));
				double ConvertAlternating = fluidVoltage/FirstCompositionImpedance;	//현재 통합 전류

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

	//위상 이동
	Phasemovement(Datalist);

	//후처리 (만약 RLC 위상 이동으로 인해 MaxVolate 오버시 변환)
	//삼각파 선택시 (정현파 -> 삼각파)
	RearProcessing(Datalist);

	return TRUE;
}