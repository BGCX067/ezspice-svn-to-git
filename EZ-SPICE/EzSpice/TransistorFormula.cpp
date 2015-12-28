#include "StdAfx.h"
#include "TransistorFormula.h"
#include "BaseElement.h"
#include "CircultSimulation.h"
#include "MainFrm.h"
#include "ElementFactory.h"
#include <algorithm>
#include <math.h>
#include "LinearDCFormula.h"

TransistorFormula::TransistorFormula(void)
{
	m_Transistor = NULL;
}

TransistorFormula::~TransistorFormula(void)
{
	list<CBaseElement*>::iterator deletepos = m_TempList.begin();
	while (deletepos != m_TempList.end()) {
		delete (*deletepos);
		++deletepos;
	}
	m_TempList.clear();
}

void TransistorFormula::Initialize(list<Node> &NodeList, list<CBaseElement*> &Provlist)
{
	m_NodeList.clear();
	m_NodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_NodeList.begin());

	m_TempNodeList.clear();
	m_TempNodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_TempNodeList.begin());

	m_ProvList.clear();
	m_ProvList.resize(Provlist.size());
	copy(Provlist.begin(), Provlist.end(), m_ProvList.begin());

	m_TempProvList.clear();
	m_TempProvList.resize(Provlist.size());
	copy(Provlist.begin(), Provlist.end(), m_TempProvList.begin());

	//원래 연결정보 복사
	list<CBaseElement*>::iterator pos = CCircultSimulation::m_CircuitList.begin();
	while (pos != CCircultSimulation::m_CircuitList.end()) {

		RecoveryConnectElment NewConnect;
		NewConnect.pElement = (*pos);

		NewConnect.ConnectElement.clear();
		NewConnect.ConnectElement.resize((*pos)->m_ConnectElement.size());
		copy((*pos)->m_ConnectElement.begin(), (*pos)->m_ConnectElement.end(), NewConnect.ConnectElement.begin());

		m_CircuitRecoveryList.push_back(NewConnect);
		++pos;
	}

	//모든 인피던스 소자에 주파수 삽입
	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		list<CBaseElement*>::iterator lowpos = (*nodepos).AllElement.begin();
		while (lowpos != (*nodepos).AllElement.end()) {
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
		++nodepos;
	}

	//TR 정보 획득
	m_Transistor = SearchElement(m_NodeList, transistor);
	list<ElementData> GetList;
	m_Transistor->GetProperties(GetList);

	list<ElementData>::iterator getpos = GetList.begin();
	while (getpos != GetList.end()) {

		if ((*getpos).Text == CString(_T("OperatingPoint"))) {
			m_OperatingPoint = (*getpos).Value;
		}
		else if ((*getpos).Text == CString(_T("SaturatingPoint"))) {
			m_SaturatingPoint = (*getpos).Value;
		}
		else if ((*getpos).Text == CString(_T("Beta"))) {
			m_Beta = (*getpos).Value;
		}
		++getpos;
	}
}

void TransistorFormula::CreateIPCDataList(list<Node> &CurrentNodeList, list<IPCData> &CurrentDataList, list<CBaseElement*> &CurrentProvList)
{
	//CurrentNodeList와 CurrentProvList로 CurrentDataList데이터 생성


	list<CBaseElement*>::iterator provpos = CurrentProvList.begin();
	while (provpos != CurrentProvList.end()) {

		if ((*provpos)->m_InputElement.size() > 0) {
			list<CBaseElement*>::iterator CompareElement = (*provpos)->m_InputElement.begin();	//프로브 위치

			list<Node>::iterator nodepos = CurrentNodeList.begin();
			while (nodepos != CurrentNodeList.end()) {

				list<CBaseElement*>::iterator searchpos = find((*nodepos).LineElement.begin(), (*nodepos).LineElement.end(), (*CompareElement));
				if (searchpos != (*nodepos).LineElement.end()) {
					IPCData NewData;
					NewData.ComplexCheckFlag = FALSE;
					NewData.NodeNumber = (*nodepos).NodeNumber;
					NewData.ProvNumber = (*provpos)->m_ElementNumber;
					(*nodepos).FirstFlag == TRUE ? NewData.FirstFlag = TRUE : NewData.FirstFlag = FALSE;
					CurrentDataList.push_back(NewData);
				}
				++nodepos;
			}
		}
		++provpos;
	}
}

BOOL TransistorFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist )
{
	//복사
	Initialize(NodeList, Provlist);	

	//IPC데이터 생성
	CreateIPCDataList(m_NodeList, Datalist, m_ProvList);

	//다이오드 예외처리
	if (ElementException()) {	
		return FALSE;
	}

	//회로 3개 영역으로 분리 + 2개 추가 (dc용)
	DivideCircit();

	//모든 영역 합성 인피던스 획득	
	SectionCompositonImpedance();
	
	double Rb = 0, Re = 0, Rc = 0;
	double Ib = 0, Ie = 0, Ic = 0;
	double Vb = 0, Ve = 0, Vc = 0;
	double Vce = 0;

	Rb = GetDivideCircuitInformation(base, CString(_T("ComplexNumber")));
	Re = GetDivideCircuitInformation(emitter, CString(_T("ComplexNumber")));
	Rc = GetDivideCircuitInformation(collector, CString(_T("ComplexNumber")));

	int OperatorFlag[4] = {FALSE,}; //1 - off, 2 - active, 3 - sat 
	if (CCircultSimulation::m_ACData.Amplitude != -1) {  //base node가 AC일때

		double fluidFrequency = (1/CCircultSimulation::m_ACData.Frequency)/DivideSector;
		for (double iCount = 0; iCount < DivideSector; iCount++) {

			double fluidVoltage = CCircultSimulation::m_ACData.Amplitude*sin(2*PIE*CCircultSimulation::m_ACData.Frequency*(fluidFrequency*iCount));
			if (fluidVoltage <= m_OperatingPoint) {
				
				OperatorFlag[off] = TRUE; 
				list<IPCData>::iterator pos = Datalist.begin();
				while (pos != Datalist.end()) {

					double ResultValue = 0;
					int Nodetype = GetNodeType((*pos).NodeNumber);
					switch (Nodetype) {
					case base:
						ResultValue = fluidVoltage;
						break;

					case collector:
						ResultValue = m_CollectorVcc;
						break;

					case emitter:
						ResultValue = 0;
						break;
					}
					(*pos).GrapehValue[(int)iCount] = ResultValue;
					++pos;
				}
			}
			else {
				Ib = (fluidVoltage - m_OperatingPoint)/(Rb + (m_Beta+1)*Re);
				Ic = m_Beta*Ib;
				Ie = Ib + Ic;

				Vce = m_CollectorVcc - Ic*Rc - Ie*Re;
				if (Vce <= m_SaturatingPoint) {	//Sat
					Vce = m_SaturatingPoint;	
					Ic = (m_CollectorVcc -m_SaturatingPoint)/(Rc+Re);
					Ie = Ib + Ic;

					OperatorFlag[saturated] = TRUE;
				}
				else {	//Active
					OperatorFlag[active] = TRUE;
				}
				
				Ve = Ie * Re;
				Vc = Vce + Ve;
				Vb = Ve + m_OperatingPoint;

				//Ve, Vc, Vb위치에 프로브 및 다른 위치 데이터 삽입

				list<IPCData>::iterator pos = Datalist.begin();
				while (pos != Datalist.end()) {

					list<DivideCircuitInfo>::iterator dividepos = m_DivideList.begin();
					while (dividepos != m_DivideList.end()) {

						if ((*dividepos).TRNodeNumber == (*pos).NodeNumber) {

							double ResultValue = 0;
							switch ((*dividepos).CirciutType) {
								case base: ResultValue = Vb; break;
								case collector: ResultValue = Vc; break;
								case emitter: ResultValue = Ve; break;
							}
							(*pos).GrapehValue[(int)iCount] = ResultValue;
						}
						else {
							list<IPCData>::iterator Lowipcpos = (*dividepos).DataList.begin();
							while (Lowipcpos != (*dividepos).DataList.end()) {
									
								if ((*Lowipcpos).NodeNumber == (*pos).NodeNumber) {

									double CompositionImpedance = sqrt(pow((*Lowipcpos).ComplexNumber.real(), 2) + pow((*Lowipcpos).ComplexNumber.imag(), 2));
									double ResultValue = 0;
									int NodeType = GetNodeType((*pos).NodeNumber);
									switch (NodeType) {
										case base: ResultValue = (Ib * (CompositionImpedance + (m_Beta+1)*Re)) + m_OperatingPoint; break;
										case collector: ResultValue = Vce + Ve + (Ic*CompositionImpedance); break;
										case emitter: ResultValue = Ie * CompositionImpedance; break;
									}
									(*pos).GrapehValue[(int)iCount] = ResultValue;
								}
								++Lowipcpos;
							}
						}
						++dividepos;
					}
					++pos;
				}
			}
		}
	}
	else {	//base node가 DC일때
		//base node에 포함된 전압원 전류 측정

		if (m_BaseVcc <= m_OperatingPoint) {

			OperatorFlag[off] = TRUE; 
			list<IPCData>::iterator pos = Datalist.begin();
			while (pos != Datalist.end()) {
				
				double ResultValue = 0;
				int Nodetype = GetNodeType((*pos).NodeNumber);
				switch (Nodetype) {
					case base:
						ResultValue = m_BaseVcc;
						break;

					case collector:
						ResultValue = m_CollectorVcc;
						break;

					case emitter:
						ResultValue = 0;
						break;
				}

				memset((*pos).GrapehValue, 0, sizeof(double)*DivideSector);
				for (int iCount = 0; iCount < DivideSector; ++iCount) {
					(*pos).GrapehValue[iCount] = ResultValue;
				}
				++pos;
			}
		}
		else {
			double fluidVoltage = m_BaseVcc;
			Ib = (fluidVoltage - m_OperatingPoint)/(Rb + (m_Beta+1)*Re);
			Ic = m_Beta*Ib;
			Ie = Ib + Ic;

			Vce = m_CollectorVcc - Ic*Rc - Ie*Re;
			if (Vce <= m_SaturatingPoint) {	//Sat
				Vce = m_SaturatingPoint;	
				Ic = (m_CollectorVcc -m_SaturatingPoint)/(Rc+Re);
				Ie = Ib + Ic;

				OperatorFlag[saturated] = TRUE;
			}
			else {	//Active
				OperatorFlag[active] = TRUE;
			}

			Ve = Ie * Re;
			Vc = Vce + Ve;
			Vb = Ve + m_OperatingPoint;

			//Ve, Vc, Vb위치에 프로브 있으면 데이터 삽입

			list<IPCData>::iterator pos = Datalist.begin();
			while (pos != Datalist.end()) {

				list<DivideCircuitInfo>::iterator dividepos = m_DivideList.begin();
				while (dividepos != m_DivideList.end()) {

					if ((*dividepos).TRNodeNumber == (*pos).NodeNumber) {

						double ResultValue = 0;
						switch ((*dividepos).CirciutType) {
								case base: ResultValue = Vb; break;
								case collector: ResultValue = Vc; break;
								case emitter: ResultValue = Ve; break;
						}
						memset((*pos).GrapehValue, 0, sizeof(double)*DivideSector);
						for (int iCount = 0; iCount < DivideSector; ++iCount) {
							(*pos).GrapehValue[iCount] = ResultValue;
						}
					}
					else {
						list<IPCData>::iterator Lowipcpos = (*dividepos).DataList.begin();
						while (Lowipcpos != (*dividepos).DataList.end()) {

							if ((*Lowipcpos).NodeNumber == (*pos).NodeNumber) {

								double CompositionImpedance = sqrt(pow((*Lowipcpos).ComplexNumber.real(), 2) + pow((*Lowipcpos).ComplexNumber.imag(), 2));
								double ResultValue = 0;
								int NodeType = GetNodeType((*pos).NodeNumber);
								switch (NodeType) {
										case base: ResultValue = (Ib * (CompositionImpedance + (m_Beta+1)*Re)) + m_OperatingPoint; break;
										case collector: ResultValue = Vce + Ve + (Ic*CompositionImpedance); break;
										case emitter: ResultValue = Ie * CompositionImpedance; break;
								}
								memset((*pos).GrapehValue, 0, sizeof(double)*DivideSector);
								for (int iCount = 0; iCount < DivideSector; ++iCount) {
									(*pos).GrapehValue[iCount] = ResultValue;
								}
							}
							++Lowipcpos;
						}
					}
					++dividepos;
				}
				++pos;
			}
		}
	}


	list<ElementData> SendList;
	ElementData NewData;
	NewData.Text = CString(_T("OperatorFlag"));
	NewData.Value = OperatorFlag[saturated] ? saturated : OperatorFlag[active] ? active : off;
	
	SendList.push_back(NewData);
	m_Transistor->OnEditProperties(&SendList);

	return TRUE;
}

int TransistorFormula::GetNodeType(int nodenumber)
{
	int Resuit = 0;

	list<DivideCircuitInfo>::iterator pos = m_DivideList.begin();
	while (pos != m_DivideList.end()) {

		list<Node>::iterator nodepos = (*pos).NodeList.begin();
		while (nodepos != (*pos).NodeList.end()) {

			if ((*nodepos).NodeNumber == nodenumber) {
				Resuit = (*pos).CirciutType;
				break;
			}
			++nodepos;
		}
		++pos;
	}
	return Resuit;
}

void TransistorFormula::IPCDataConvert(list<IPCData> &DataList)
{
	list<IPCData>::iterator ipcpos = DataList.begin();
	while (ipcpos != DataList.end()) {
	
		list<DivideCircuitInfo>::iterator dividepos = m_DivideList.begin();
		while (dividepos != m_DivideList.end()) {
		
			list<IPCData>::iterator lowpos = (*dividepos).DataList.begin();
			while (lowpos != (*dividepos).DataList.end()) {

				if ((*lowpos).NodeNumber == (*ipcpos).NodeNumber && (*lowpos).ProvNumber == (*ipcpos).ProvNumber) {
					
					memset((*ipcpos).GrapehValue, 0, sizeof(double)*DivideSector);
					memcpy_s((*ipcpos).GrapehValue, sizeof(double)*DivideSector, (*lowpos).GrapehValue, sizeof(double)*DivideSector);
				}
				++lowpos;
			}
			++dividepos;
		}
		++ipcpos;
	}
}

double TransistorFormula::GetDivideCircuitInformation(int DivideType, CString &attribute)
{
	list<DivideCircuitInfo>::iterator pos = m_DivideList.begin();
	while (pos != m_DivideList.end()) {
		
		if ((*pos).CirciutType == DivideType) {
			break;
		}
		++pos;
	}

	if (attribute == CString(_T("ComplexNumber"))) {
		return sqrt(pow((*pos).ComplexNumber.real(), 2) + pow((*pos).ComplexNumber.imag(), 2));
	}

	return 0;
}

void TransistorFormula::SectionCompositonImpedance()
{
 	//CompositionResistance 프로브 별로 나눠서 합성저항 구하고 적용
	list<DivideCircuitInfo>::iterator dividepos = m_DivideList.begin();
	while (dividepos != m_DivideList.end()) {
			
		UINT RepetitionCount = (*dividepos).ProvList.size();
		for (UINT iCount = 0; iCount < RepetitionCount; iCount++) {
			
			//해당 값 초기화
			m_NodeList.clear();
			m_NodeList.resize((*dividepos).NodeList.size());
			copy((*dividepos).NodeList.begin(), (*dividepos).NodeList.end(), m_NodeList.begin());

			int ProvCount = 0;
			list<CBaseElement*>::iterator provpos = (*dividepos).ProvList.begin();
			while (provpos != (*dividepos).ProvList.end()) {
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
			CompostionProvPostion((*dividepos).DataList);
		}

		//전체 합성 인피던스
		//위상을 구하기 위해서는 개개인의 합성 인피던스 각도에서 전체 합성 인피던스 각도를 빼야함
		{
			//해당 값 초기화
			m_TempDataList.clear();
			m_ProvList.clear();
			m_NodeList.clear();
			m_NodeList.resize((*dividepos).NodeList.size());
			copy((*dividepos).NodeList.begin(), (*dividepos).NodeList.end(), m_NodeList.begin());

			IPCData NewData;
			NewData.ComplexCheckFlag = FALSE;

			//첫 구역 체크
			list<Node>::iterator pos = m_NodeList.begin();
			while (pos != m_NodeList.end()) {
				if ((*pos).FirstFlag) {
					NewData.NodeNumber = (*pos).NodeNumber;
					break;
				}
				++pos;
			}
			m_TempDataList.push_back(NewData);

			//테브닌 정리를 통해 등가 교환
			Thevenintheorem();

			//프로브 위치 합성 인피던스 
			CompostionProvPostion(m_TempDataList, TRUE);

			list<IPCData>::iterator TotalComposistion = m_TempDataList.begin();
			(*dividepos).ComplexNumber = (*TotalComposistion).ComplexNumber;
		}
		++dividepos;
	}
}

void TransistorFormula::Thevenintheorem()
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

BOOL TransistorFormula::IsProvArea(Node &CurrentNode)
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
complex<double> TransistorFormula::GetElementInpedance(CBaseElement *pElement)
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

double TransistorFormula::GetElementProperty(CBaseElement *pElement, CString &PropertyString)
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


void TransistorFormula::CompostionProvPostion(list<IPCData> &Datalist, BOOL nFlag /*= FALSE*/)
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

void TransistorFormula::SeriesMerger(BOOL &OperationFlag)
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

void TransistorFormula::RarallelMerger(BOOL &OperationFlag)
{
	//기본적으로 프로브가 포함된 구간은 제외한다.
	//최하위 병렬구간을 병합한다. 단 조건이 병합할 소자들이 연결된 다른 노드가 없어야 한다.

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {

		if (/*IsProvArea((*nodepos)) == FALSE*/TRUE) {	//프로브 구간이 아닐때
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

					list<CBaseElement*>::iterator SecondElement = Mergerlist.begin();
					++SecondElement;
					double_complex SecondInpedance(0,0);
					SecondInpedance = GetElementInpedance((*SecondElement));

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

void TransistorFormula::DuplicateMerger(BOOL &OperationFlag)
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

					list<CBaseElement*>::iterator SecondElement = Mergerlist.begin();
					++SecondElement;
					double_complex SecondInpedance(0,0);
					SecondInpedance = GetElementInpedance((*SecondElement));

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

void TransistorFormula::GetDuplicateNode(Node &CurrentNode, list<CBaseElement*> &MergerList)
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

void TransistorFormula::GetFamilyNode(Node &CurrentNode, CBaseElement *CompareElement, Node **ResultNode)
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

BOOL TransistorFormula::IsAnotherNode(Node &CurrentNode, CBaseElement *pElement, Node **AnotherNode)
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

int TransistorFormula::DeleteElementInNode(Node &CurrentNode, CBaseElement *pElement)
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

void TransistorFormula::DivideCircit()
{
	//TR검색
	CBaseElement *pTransistor = SearchElement(m_NodeList, transistor);
	if (pTransistor != NULL) {

		//트랜지스터의 특성상 회전이 불가능하고 
		//첫번째 ConnectPoint -> 아래, 두번째 -> 위, 세번째 -> 왼쪽으로 설정이 되어있다.
		
		list<Node>::iterator nodepos = m_NodeList.begin();
		while (nodepos != m_NodeList.end()) {
			
			list<CBaseElement*>::iterator elementpos = (*nodepos).AllElement.begin();
			while (elementpos != (*nodepos).AllElement.end()) {

				if ((*elementpos)->m_Flag == transistor) {
					break;
				}
				++elementpos;
			}

			if (elementpos != (*nodepos).AllElement.end()) {
				DivideCircuitInfo NewInfo;
			
				NewInfo.CirciutType = GetTransistorDirection((*nodepos), (*elementpos));
				RecusiveNodeSearch((*nodepos), (*elementpos), NewInfo);

				CBaseElement* NewElement = GetAlternationElement(NewInfo.CirciutType);
				NewInfo.pAddElement = NewElement;
				NewInfo.TRNodeNumber = (*nodepos).NodeNumber;
				list<Node>::iterator TRnode = NewInfo.NodeList.begin();

				ChangeElement((*TRnode), (*elementpos), NewElement);	
				CreateProvList(NewInfo);
				CreateIPCDataList(NewInfo.NodeList, NewInfo.DataList, NewInfo.ProvList);
				CheckVoltage(NewInfo);

				m_DivideList.push_back(NewInfo);
			}
			++nodepos;
		}
	}

// 	//추가 노드 생성 (왼쪽+아래, 위+아래) + TR->Capacity
// 	DivideCircuitInfo BaseCircuit = GetDivideCircuitInfo(base);
// 	DivideCircuitInfo FirstEmitterCircuit = GetDivideCircuitInfo(emitter);
// 
// 	DivideCircuitInfo BaseEmitterInfo = BaseCircuit;
// 	BaseEmitterInfo.CirciutType = baseemitter;
// 	MergerDivideCircuit(BaseEmitterInfo, FirstEmitterCircuit);
// 	m_DivideList.push_back(BaseEmitterInfo);
// 
// 	DivideCircuitInfo CollectorCircuit = GetDivideCircuitInfo(collector);
// 	DivideCircuitInfo SecondEmitterCircuit = GetDivideCircuitInfo(emitter);
// 	
// 	DivideCircuitInfo CollectorEmitterInfo = CollectorCircuit;
// 	CollectorEmitterInfo.CirciutType = collectoremitter;
// 	MergerDivideCircuit(CollectorEmitterInfo, SecondEmitterCircuit);
// 	m_DivideList.push_back(CollectorEmitterInfo);
}

void TransistorFormula::MergerDivideCircuit(DivideCircuitInfo &BaseInfo, DivideCircuitInfo &AddInfo)
{
	list<Node>::iterator nodepos = AddInfo.NodeList.begin();
	while (nodepos != AddInfo.NodeList.end()) {
		
		list<CBaseElement*>::iterator searchpos = find((*nodepos).AllElement.begin(), (*nodepos).AllElement.end(), AddInfo.pAddElement);
		if (searchpos != (*nodepos).AllElement.end()) {
			ChangeElement((*nodepos), AddInfo.pAddElement, BaseInfo.pAddElement);
			break;
		}
		++nodepos;
	}

	nodepos = AddInfo.NodeList.begin();
	while (nodepos != AddInfo.NodeList.end()) {
		(*nodepos).FirstFlag = FALSE;
		BaseInfo.NodeList.push_back((*nodepos));
		++nodepos;
	}
	
	list<CBaseElement*>::iterator provpos = AddInfo.ProvList.begin();
	while (provpos != AddInfo.ProvList.end()) {
		BaseInfo.ProvList.push_back((*provpos));
		++provpos;
	}
}

void TransistorFormula::SetDivideCircuitInfo(int DivideType, DivideCircuitInfo &SaveInfo)
{
	list<DivideCircuitInfo>::iterator pos = m_DivideList.begin();
	while (pos != m_DivideList.end()) {

		if ((*pos).CirciutType == DivideType) {
			break;
		}
		++pos;
	}

	m_DivideList.erase(pos);
	m_DivideList.push_back(SaveInfo);
}

DivideCircuitInfo TransistorFormula::GetDivideCircuitInfo(int DivideType)
{
	list<DivideCircuitInfo>::iterator pos = m_DivideList.begin();
	while (pos != m_DivideList.end()) {
		
		if ((*pos).CirciutType == DivideType) {
			break;
		}
		++pos;
	}
	return (*pos);
}

void TransistorFormula::CheckVoltage(DivideCircuitInfo &NewInfo)
{
	//노드정보를 체크하여 dc Vcc 획득

	CBaseElement *pSearchElement = SearchElement(NewInfo.NodeList, dcpower);
	if (pSearchElement != NULL) {

		list<ElementData> GetList;
		pSearchElement->GetProperties(GetList);

		double Voltage = 0;
		list<ElementData>::iterator getpos = GetList.begin();
		while (getpos != GetList.end()) {

			if ((*getpos).Text == CString(_T("Volatage"))) {
				Voltage = (*getpos).Value;
				break;
			}
			++getpos;
		}
	
		if (NewInfo.CirciutType == base) {
			m_BaseVcc = Voltage;
		}
		else if (NewInfo.CirciutType == collector) {
			m_CollectorVcc = Voltage;
		}
	}
}

void TransistorFormula::CreateProvList(DivideCircuitInfo &NewInfo)
{
	//모든 프로브 리스트 중에 현재 분리 회로에 포함되는 프로브 적용
	list<Node>::iterator nodepos = NewInfo.NodeList.begin();
	while (nodepos != NewInfo.NodeList.end()) {

		list<CBaseElement*>::iterator pos = m_ProvList.begin();
		while (pos != m_ProvList.end()) {

			list<CBaseElement*>::iterator provelement = (*pos)->m_InputElement.begin();
			list<CBaseElement*>::iterator duplicatepos = find((*nodepos).LineElement.begin(), (*nodepos).LineElement.end(), (*provelement));
			if (duplicatepos != (*nodepos).LineElement.end()) {
				NewInfo.ProvList.push_back((*pos));
			}
			++pos;
		}
		++nodepos;
	}
}

void TransistorFormula::ChangeElement(Node &CurrentNode, CBaseElement* RemoveElement, CBaseElement* AlternationElement)
{
	if (AlternationElement->m_Flag == dcpower) {
		CurrentNode.FirstFlag = TRUE;
	}

	list<CBaseElement*>::iterator inputpos = find(CurrentNode.InputElement.begin(), CurrentNode.InputElement.end(), RemoveElement);
	if (inputpos != CurrentNode.InputElement.end()) {

		CurrentNode.InputElement.erase(inputpos);
		CurrentNode.InputElement.push_back(AlternationElement);
	}

	list<CBaseElement*>::iterator outputpos = find(CurrentNode.OutputElement.begin(), CurrentNode.OutputElement.end(), RemoveElement);
	if (outputpos != CurrentNode.OutputElement.end()) {

		CurrentNode.OutputElement.erase(outputpos);
		CurrentNode.OutputElement.push_back(AlternationElement);
	}

	CurrentNode.AllElement.remove(RemoveElement);
	CurrentNode.AllElement.push_back(AlternationElement);
}

void TransistorFormula::RecusiveNodeSearch(Node &CurrnetNode, CBaseElement *CompareElement, DivideCircuitInfo &NewInfo)
{
	//노드를 탐색하여 분리하며 새로운 구성을 추가한다.
	//새로운 구성을 각 회로가 동작되도록 하기 위한 회로 이다.

	//중복검사
	list<Node>::iterator duplicatepos = NewInfo.NodeList.begin();
	while (duplicatepos != NewInfo.NodeList.end()) {
		if ((*duplicatepos).NodeNumber == CurrnetNode.NodeNumber) {
			break;
		}
		++duplicatepos;
	}

	if (duplicatepos != NewInfo.NodeList.end()) {
		return;
	}
	NewInfo.NodeList.push_back(CurrnetNode);

	list<TRNode> RecusiveList;
	list<CBaseElement*>::iterator pos = CurrnetNode.AllElement.begin();
	while (pos != CurrnetNode.AllElement.end()) {
			
		if ((*pos) != CompareElement) {
			list<Node>::iterator nodepos = m_NodeList.begin();
			while (nodepos != m_NodeList.end()) {

				if ((*nodepos).NodeNumber != CurrnetNode.NodeNumber) {

					list<CBaseElement*>::iterator Searchpos = find((*nodepos).AllElement.begin(), (*nodepos).AllElement.end(), (*pos));
					if (Searchpos != (*nodepos).AllElement.end()) {

						TRNode NewNode;
						NewNode.NodeData = (*nodepos);
						NewNode.CompareElement = (*pos);
						RecusiveList.push_back(NewNode);
					}
				}
				++nodepos;
			}
		}
		++pos;
	}

	list<TRNode>::iterator recusivepos = RecusiveList.begin();
	while (recusivepos != RecusiveList.end()) {
		RecusiveNodeSearch((*recusivepos).NodeData, (*recusivepos).CompareElement, NewInfo);
		++recusivepos;
	}
}

CBaseElement* TransistorFormula::GetAlternationElement(int Collectiontype)
{
	CElementFactory *pFactory = CMainFrame::CreateElementFactory();
	switch (Collectiontype) {
	case base:
	case collector:
		{
			CBaseElement *NewCapacity = pFactory->CreateElement(capacity);

			list<ElementData> SendList;
			ElementData NewData;
			NewData.Text = CString(_T("NullFlag"));
			NewData.Value = TRUE;
		
			SendList.push_back(NewData);
			NewCapacity->OnEditProperties(&SendList);

			m_TempList.push_back(NewCapacity);
			return NewCapacity;
		}
		break;

	case emitter:
		{
			CBaseElement *NewPower = pFactory->CreateElement(dcpower);
			m_TempList.push_back(NewPower);
			return NewPower;
		}
		break;
	}
	return NULL;
}


int	TransistorFormula::GetTransistorDirection(Node &CurrnetNode, CBaseElement *pTransistor)
{
	//첫번째 ConnectPoint -> 아래, 두번째 -> 위, 세번째 -> 왼쪽으로 설정이 되어있다.
	int CurrentDirection = -1;

	int ConnectCount = 0;
	list<CPoint>::iterator Connectpos = pTransistor->m_ConnectPoint.begin();
	while (Connectpos != pTransistor->m_ConnectPoint.end()) {

		list<CBaseElement*>::iterator linepos = CurrnetNode.LineElement.begin();
		while (linepos != CurrnetNode.LineElement.end()) {
			
			if ((*linepos)->m_LineTo == pTransistor->m_Position + (*Connectpos) || (*linepos)->m_MoveTo == pTransistor->m_Position + (*Connectpos)) {

				if (ConnectCount == 0) CurrentDirection = emitter;
				else if (ConnectCount == 1) CurrentDirection = collector;
				else CurrentDirection = base;
				
				return CurrentDirection;
			}
			++linepos;
		}
		++ConnectCount;
		++Connectpos;
	}

	return CurrentDirection;
}

BOOL TransistorFormula::ElementException()
{
	//다이오드 삽입시 False
	if (SearchElement(m_NodeList, diode) != NULL) {
		AfxMessageBox(_T("트랜지스터 적용 시 다이오드를 적용할 수 없습니다."));
		return TRUE;
	}
	return FALSE;
}

CBaseElement* TransistorFormula::SearchElement(list<Node> &CurrentNodeList, int ElementType)
{
	list<Node>::iterator nodepos = CurrentNodeList.begin();
	while (nodepos != CurrentNodeList.end()) {

		list<CBaseElement*>::iterator pos = (*nodepos).AllElement.begin();
		while (pos != (*nodepos).AllElement.end()) {

			if ((*pos)->m_Flag == ElementType) {
				return (*pos);
			}
			++pos;
		}
		++nodepos;
	}
	return NULL;
}