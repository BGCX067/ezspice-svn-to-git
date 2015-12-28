#include "StdAfx.h"
#include "NodePretreatment.h"
#include "BaseElement.h"
#include "MainFrm.h"
#include <algorithm>
#include "CircultSimulation.h"
#include "ElementFactory.h"

CBaseElement *pStartElement = NULL; 
CBaseElement *pBeforeElement = NULL;

enum LineCheck
{
	Antidromic, Normal, Orthodromic 
};

class CircuitElementRankingsort
{
public:
	bool operator() (const CBaseElement *pFirstElement, const CBaseElement *pSecondElement) const {
		return pFirstElement->m_Flag < pSecondElement->m_Flag;
	}
};

class CircuitConnectPreferenceRank
{
public:
	BOOL operator() (const CBaseElement *pFirstElement, const CBaseElement *pSecondElement) const {
		
		BOOL nFlag  = FALSE;
		if (pFirstElement->m_Flag == lines && pSecondElement->m_Flag == lines) {
			
			CPoint FirstPoint = pFirstElement->m_LineTo + pFirstElement->m_MoveTo;
			CPoint SecondPoint = pSecondElement->m_LineTo + pSecondElement->m_MoveTo;
			
			if (FirstPoint.y == SecondPoint.y) {
				FirstPoint.x > SecondPoint.x ? nFlag = TRUE : nFlag = FALSE;
			}
			else if (FirstPoint.y < SecondPoint.y) nFlag = TRUE;
			else nFlag = FALSE;	
		}
		else if (pFirstElement->m_Flag == lines && pSecondElement->m_Flag != lines) {
			nFlag = TRUE;
		}
		else if (pFirstElement->m_Flag != lines && pSecondElement->m_Flag == lines) {
			nFlag = FALSE;
		}

		return nFlag;
	}
};

NodePretreatment::NodePretreatment(void)
{
}

NodePretreatment::~NodePretreatment(void)
{
}

void NodePretreatment::AheadProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList) //전처리
{
	switch (CCircultSimulation::m_CircultCase) {
		case linearDC:	
		case nonlinearDC:
			{
				//Inductor -> lines
				list<CBaseElement*>::iterator pos = CircultList.begin();
				while (pos != CircultList.end()) {
					if ((*pos)->m_Flag == inductor) {
						(*pos)->m_Flag = lines;
						m_TempInductor.push_back((*pos));
					}
					++pos;
				}

				//diode -> already direction decide -> IsExceptionElement() 
			}
			break;

		case linearAC:
			{
				//non
			}
			break;

		default:
			break;
	}
}

BOOL NodePretreatment::IsDiodeAntidromic(Node &CurrentNode, CBaseElement *pDiodeElement, BOOL DirectionFlag)
{
	list<CPoint>::iterator connectpos = pDiodeElement->m_ConnectPoint.begin();
	CPoint DiodeInputPosition = pDiodeElement->m_Position + (*connectpos);

	list<CBaseElement*>::iterator pos = CurrentNode.LineElement.begin();
	while (pos != CurrentNode.LineElement.end()) {
		
		if ((*pos)->m_LineTo == DiodeInputPosition || (*pos)->m_MoveTo == DiodeInputPosition) {
			return TRUE;
		}
		++pos;
	}
	return FALSE;
}

void NodePretreatment::RearProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList)	//후처리
{
	switch (CCircultSimulation::m_CircultCase) {
		case linearDC:	
		case nonlinearDC:
			{
				//m_TempInductor is Element -> inpuctor
				list<CBaseElement*>::iterator Inductorpos = m_TempInductor.begin();
				while (Inductorpos != m_TempInductor.end()) {
					(*Inductorpos)->m_Flag = inductor;
					++Inductorpos;
				}
				m_TempInductor.clear();

				//캐패시터 관련 노드 삭제
				list<list<Node>::iterator> Deletelist;
				list<Node>::iterator nodepos = NodeList.begin();
				while (nodepos != NodeList.end()) {

					list<CBaseElement*>::iterator pos = (*nodepos).AllElement.begin();
					while (pos != (*nodepos).AllElement.end()) {
						if ((*pos)->m_Flag == capacity) {
							break;
						}
						++pos;
					}

					if (pos != (*nodepos).AllElement.end()) {
						CBaseElement *DeleteElement = (*pos);
						(*nodepos).AllElement.remove((*pos));

						list<CBaseElement*>::iterator searchpos = find((*nodepos).InputElement.begin(), (*nodepos).InputElement.end(), DeleteElement);
						if (searchpos != (*nodepos).InputElement.end()) {
							(*nodepos).InputElement.remove(DeleteElement);
						}

						searchpos = find((*nodepos).OutputElement.begin(), (*nodepos).OutputElement.end(), DeleteElement);
						if (searchpos != (*nodepos).OutputElement.end()) {
							(*nodepos).OutputElement.remove(DeleteElement);
						}
					}
					++nodepos;
				}

				//Nonlinear diode RearProcessing
				{
					//diode ->  linear companion model. and value copy
					list<Node>::iterator nodepos = NodeList.begin();
					while (nodepos != NodeList.end()) {

						list<CBaseElement*> DeleteInputList;
						list<CBaseElement*>::iterator inputpos = (*nodepos).InputElement.begin();
						while (inputpos != (*nodepos).InputElement.end()) {

							if ((*inputpos)->m_Flag == diode) {
								//Geq, Leq 생성 및 대체
								CElementFactory *pFactory = CMainFrame::CreateElementFactory();

								CBaseElement *NewGeqElement = pFactory->CreateElement(geq);
								list<ElementData> DiodeDataList;
								(*inputpos)->GetProperties(DiodeDataList);
								NewGeqElement->OnEditProperties(&DiodeDataList);

								CBaseElement *NewLeqElement = pFactory->CreateElement(leq);

								ElementData NewData;
								NewData.Text = CString(_T("GeqElement"));
								NewData.pElement = NewGeqElement;
								DiodeDataList.push_back(NewData);
								NewLeqElement->OnEditProperties(&DiodeDataList);

								if ((*inputpos)->m_AntidromicFlag) {
									NewLeqElement->m_AntidromicFlag = TRUE;
									NewGeqElement->m_AntidromicFlag = TRUE;
								}

								(*nodepos).InputElement.push_back(NewGeqElement);
								(*nodepos).AllElement.push_back(NewGeqElement);

								(*nodepos).InputElement.push_back(NewLeqElement);
								(*nodepos).AllElement.push_back(NewLeqElement);
								DeleteInputList.push_back((*inputpos));

								//Diode가 연결된 객체도 대체
								list<Node>::iterator lowpos = NodeList.begin();
								while (lowpos != NodeList.end()) {
									if ((*lowpos).NodeNumber != (*nodepos).NodeNumber) {

										list<CBaseElement*>::iterator searchpos = find((*lowpos).OutputElement.begin(), (*lowpos).OutputElement.end(), (*inputpos));
										if (searchpos != (*lowpos).OutputElement.end()) {
											(*lowpos).AllElement.remove((*inputpos));
											(*lowpos).AllElement.push_back(NewGeqElement);
											(*lowpos).AllElement.push_back(NewLeqElement);

											(*lowpos).OutputElement.remove((*inputpos));
											(*lowpos).OutputElement.push_back(NewGeqElement);
											(*lowpos).OutputElement.push_back(NewLeqElement);
										}
									}
									++lowpos;
								}
							}
							++inputpos;
						}

						list<CBaseElement*> DeleteOutputList;
						list<CBaseElement*>::iterator outputpos = (*nodepos).OutputElement.begin();
						while (outputpos != (*nodepos).OutputElement.end()) {
							if ((*outputpos)->m_Flag == diode) {
								CElementFactory *pFactory = CMainFrame::CreateElementFactory();

								CBaseElement *NewGeqElement = pFactory->CreateElement(geq);
								list<ElementData> DiodeDataList;
								(*outputpos)->GetProperties(DiodeDataList);
								NewGeqElement->OnEditProperties(&DiodeDataList);

								CBaseElement *NewLeqElement = pFactory->CreateElement(leq);

								ElementData NewData;
								NewData.Text = CString(_T("GeqElement"));
								NewData.pElement = NewGeqElement;
								DiodeDataList.push_back(NewData);
								NewLeqElement->OnEditProperties(&DiodeDataList);

								if ((*outputpos)->m_AntidromicFlag) {
									NewLeqElement->m_AntidromicFlag = TRUE;
									NewGeqElement->m_AntidromicFlag = TRUE;
								}

								(*nodepos).OutputElement.push_back(NewGeqElement);
								(*nodepos).AllElement.push_back(NewGeqElement);

								(*nodepos).OutputElement.push_back(NewLeqElement);
								(*nodepos).AllElement.push_back(NewLeqElement);

								DeleteOutputList.push_back((*outputpos));

								//Diode가 연결된 객체도 대체
								list<Node>::iterator lowpos = NodeList.begin();
								while (lowpos != NodeList.end()) {
									if ((*lowpos).NodeNumber != (*nodepos).NodeNumber) {

										list<CBaseElement*>::iterator searchpos = find((*lowpos).InputElement.begin(), (*lowpos).InputElement.end(), (*outputpos));
										if (searchpos != (*lowpos).InputElement.end()) {
											(*lowpos).AllElement.remove((*outputpos));
											(*lowpos).AllElement.push_back(NewGeqElement);
											(*lowpos).AllElement.push_back(NewLeqElement);

											(*lowpos).InputElement.remove((*outputpos));
											(*lowpos).InputElement.push_back(NewGeqElement);
											(*lowpos).InputElement.push_back(NewLeqElement);
										}
									}
									++lowpos;
								}
							}
							++outputpos;
						}

						list<CBaseElement*>::iterator deleteinputpos = DeleteInputList.begin();
						while (deleteinputpos != DeleteInputList.end()) {
							(*nodepos).InputElement.remove((*deleteinputpos));
							(*nodepos).AllElement.remove((*deleteinputpos));
							++deleteinputpos;
						}

						list<CBaseElement*>::iterator deleteoutputpos = DeleteOutputList.begin();
						while (deleteoutputpos != DeleteOutputList.end()) {
							(*nodepos).OutputElement.remove((*deleteoutputpos));
							(*nodepos).AllElement.remove((*deleteoutputpos));
							++deleteoutputpos;
						}

						++nodepos;
					}
				}
			}
			break;

		case linearAC:
			{
				//AC전압 체크는 주기에 따라 변하기 때문에 AC 내부에서 해야 할듯
				//non
			}
			break;

		default:
			break;
	}
}

BOOL NodePretreatment::IsNegativeVoltage(list<Node> &NodeList)
{
	list<Node>::iterator nodepos = NodeList.begin();
	while (nodepos != NodeList.end()) {

		if ((*nodepos).FirstFlag) {
			list<CBaseElement*>::iterator insidepos = (*nodepos).AllElement.begin();
			while (insidepos != (*nodepos).AllElement.end()) {

				if ((*insidepos)->m_Flag == dcpower) {
					
					list<ElementData> GetList;
					(*insidepos)->GetProperties(GetList);

					list<ElementData>::iterator datapos = GetList.begin();
					while (datapos != GetList.end()) {
							
						if ((*datapos).Text == CString(_T("Volatage")) && (*datapos).Value < 0) {
							return TRUE;
						}
						++datapos;
					}
				}
				++insidepos;
			}
		}
		++nodepos;
	}
	return FALSE;
}

BOOL NodePretreatment::Processing( list<CBaseElement*> &CircultList , list<Node> &NodeList)
{
	//output listbox initialize
	m_pFrame = (CMainFrame*)AfxGetMainWnd();
	m_pFrame->OnUserDockingOutputview();
	m_pFrame->m_wndOutput.ClearString();
	PrintOutString(CString(_T(">> 컴파일을 시작합니다.")));

	//Circult copy and Initialize
	m_CircultList.clear();
	m_CircultList.resize(CircultList.size());
	copy(CircultList.begin(), CircultList.end(), m_CircultList.begin());

	list<CBaseElement*>::iterator pos = m_CircultList.begin();
	while (pos != m_CircultList.end()) {
		if ((*pos)->m_Flag != prov) {
			(*pos)->m_CheckFlag = Normal;
			(*pos)->m_GroundFlag = FALSE;
			(*pos)->m_AntidromicFlag = FALSE;
			(*pos)->m_InputElement.clear();
			(*pos)->m_OutputElement.clear();
		}
		++pos;
	}

	//Circult Connect Check
	if (!CheckCircult()) {	
		return FALSE;
	}

	// if Circultcase is linearDC, capacity is Antidromic
	AheadProcessing(m_CircultList, NodeList);

	//Circuit preferenceRnak	//회로 병렬 우선 순위 지정
	PreferenceRank();

	//Circuit DirectionDecide
	DirectionDecide();	

	//Gournd Extentsearch
	GroundExtentSearch();

	//Node Distinction
	NodeDistinction(NodeList);

	//if Circult is linearDC, Inductor is merger
	RearProcessing(m_CircultList, NodeList);

	return TRUE;
}

void NodePretreatment::PreferenceRank()
{
	list<CBaseElement*>::iterator pos = m_CircultList.begin();
	while (pos != m_CircultList.end()) {
		(*pos)->m_ConnectElement.sort(CircuitConnectPreferenceRank());
		++pos;
	}
}

void NodePretreatment::NodeDistinction(list<Node> &NodeList)	//노드 판별 시작
{
	//CheckFlag = Normal;
	list<CBaseElement*>::iterator Initializepos = m_CircultList.begin();
	while (Initializepos != m_CircultList.end()) {
		if ((CCircultSimulation::m_CircultCase == linearDC || CCircultSimulation::m_CircultCase == nonlinearDC)
			&& (*Initializepos)->m_Flag == capacity) {
			(*Initializepos)->m_CheckFlag = Orthodromic;
		}
		else {
			(*Initializepos)->m_CheckFlag = Normal;
		}
		++Initializepos;
	}

	//Node Search
	int NodeCount = 0;
	list<CBaseElement*>::iterator pos = m_CircultList.begin();
	while (pos != m_CircultList.end()) {
		if ((*pos)->m_Flag != lines && (*pos)->m_Flag != ground && (*pos)->m_Flag != prov) {
			
			pStartElement = (*pos);
			pStartElement->m_CheckFlag = Orthodromic;
			
			list<CBaseElement*>::iterator inputpos = (*pos)->m_InputElement.begin();
			while (inputpos != (*pos)->m_InputElement.end()) {
				
				if ((*inputpos)->m_GroundFlag == FALSE) {
					Node InputNode;
					if ((*pos)->m_Flag == dcpower || (*pos)->m_Flag == acpower) InputNode.FirstFlag = TRUE;

					InputNode.OutputElement.push_back((*pos));
					InputNode.AllElement.push_back((*pos));

					if (RecusiveNode((*inputpos), InputNode, input) == Orthodromic) {
						InputNode.NodeNumber = NodeCount++;
						NodeList.push_back(InputNode);
					}
				}
				++inputpos;
			}


			list<CBaseElement*>::iterator outputpos = (*pos)->m_OutputElement.begin();
			while (outputpos != (*pos)->m_OutputElement.end()) {

				if ((*outputpos)->m_GroundFlag == FALSE) {
					Node OutputNode;
					if ((*pos)->m_Flag == dcpower || (*pos)->m_Flag == acpower) OutputNode.FirstFlag = TRUE;
					OutputNode.InputElement.push_back((*pos));
					OutputNode.AllElement.push_back((*pos));

					if (RecusiveNode((*outputpos), OutputNode, output) == Orthodromic) {
						OutputNode.NodeNumber = NodeCount++;
						NodeList.push_back(OutputNode);
					}
				}
				++outputpos;
			}
		}
		++pos;
	}


	//전류원 체크
	list<Node>::iterator nodepos = NodeList.begin();
	while (nodepos != NodeList.end()) {
		
		list<CBaseElement*>::iterator pos = (*nodepos).AllElement.begin();
		while (pos != (*nodepos).AllElement.end()) {
			if ((*pos)->m_Flag == dcpower || (*pos)->m_Flag == acpower) {
				(*nodepos).FirstFlag = TRUE;
				break;
			}
			++pos;
		}
		++nodepos;
	}
	
}

int NodePretreatment::RecusiveNode(CBaseElement *CurrentElement, Node &NewNode, BOOL InOutFlag)
{
	if (CurrentElement->m_Flag != lines && pStartElement != CurrentElement) {
		if (InOutFlag == input) {
			NewNode.InputElement.push_back(CurrentElement);
		}
		else {
			NewNode.OutputElement.push_back(CurrentElement);
		}
		NewNode.AllElement.push_back(CurrentElement);
		return Orthodromic;
	}
	else if (CurrentElement->m_Flag == lines) {
		NewNode.LineElement.push_back(CurrentElement);
	}

	CurrentElement->m_CheckFlag = Orthodromic;

	int nFlag = Antidromic;
	list<CBaseElement*>::iterator Inputpos = CurrentElement->m_InputElement.begin();
	while (Inputpos != CurrentElement->m_InputElement.end()) {
		if ((*Inputpos)->m_GroundFlag == FALSE && (*Inputpos)->m_CheckFlag == Normal) {
			RecusiveNode((*Inputpos), NewNode, input);
			nFlag = Orthodromic;
		}
		++Inputpos;
	}

	list<CBaseElement*>::iterator Outputpos = CurrentElement->m_OutputElement.begin();
	while (Outputpos != CurrentElement->m_OutputElement.end()) {
		if ((*Outputpos)->m_GroundFlag == FALSE && (*Outputpos)->m_CheckFlag == Normal) {
			RecusiveNode((*Outputpos), NewNode, output);
			nFlag = Orthodromic;
		}
		++Outputpos;
	}

	return nFlag;
}

void NodePretreatment::GroundExtentSearch() //Ground 범위 탐색시작
{
	list<CBaseElement*>::iterator pos = m_CircultList.begin();
	while (pos != m_CircultList.end()) {
		if ((*pos)->m_Flag == ground) {
			RecusiveExtent(NULL, (*pos));
		}
		++pos;
	}
}

int NodePretreatment::RecusiveExtent(CBaseElement *parent, CBaseElement *CurrentElement)	//Ground 범위 탐색 재귀함수
{
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {
		if ((*pos) != parent &&(*pos)->m_Flag == lines && (*pos)->m_GroundFlag == FALSE) {
			(*pos)->m_GroundFlag = TRUE;
			RecusiveExtent(CurrentElement, (*pos));
		}
		++pos;
	}
	return Orthodromic;
}

void NodePretreatment::DirectionDecide()	//진행방향 결정 시작 루틴
{
	//일반적으로 우선순위를 조정하여 진행 시작 구간을 정한다
	//진행방향 결정에는 두가지가 있는데
	//첫번째는 일반적인 경우이다. 이 경우에는 전압부를 시작으로 우상단을 기준으로 진행하며 진행방향을 결정한다.
	//특징은 출입구가 정해져 있는 소자가 있을수 있기 때문에 예외처리가 되어 있다.

	//두번째 경우는 AC 회로 && R-R 회로 && 구형파 모델일경우이다. 이 경우에는 캐패시터를 중심으로 좌우로 진행방향을 진행한다.
	//이것은 R-C회로의 구형파의 경우 특성에 의하여 바라보는 시각의 합성 저항이 필요하기 때문에 노드 생성을 용의하게 하기 위해
	//이렇게 진행된다. 그래서 중복 탐색을 막기 위해 그라운드 지역을 먼저 체크 한후 진행한다.

	//우선 순위 이동
	m_CircultList.sort(CircuitElementRankingsort());

	if (CCircultSimulation::m_CircultCase == linearAC && CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {		
		////R-C 회로 일때는 캐패시터를 중심 진로 결정
		//그라운드를 미리 체크
		GroundExtentSearch();

		list<CBaseElement*>::iterator pos = m_CircultList.begin();
		while (pos != m_CircultList.end()) {
			if ( (*pos)->m_Flag == capacity) {

				list<CBaseElement*>::iterator lowpos = (*pos)->m_ConnectElement.begin();
				while (lowpos != (*pos)->m_ConnectElement.end()) {
					(*pos)->m_CheckFlag = TRUE;
					(*lowpos)->m_InputElement.push_back((*pos));
					RecusiveDecide((*lowpos));
					++lowpos;
				}

				lowpos = (*pos)->m_ConnectElement.begin();
				while (lowpos != (*pos)->m_ConnectElement.end()) {
					(*pos)->m_OutputElement.push_back((*lowpos));
					++lowpos;
				}
			}
			++pos;
		}
	}
	else {
		//일반적인 경로 지정
		list<CBaseElement*>::iterator pos = m_CircultList.begin();
		while (pos != m_CircultList.end()) {
			if ((*pos)->m_Flag != lines && (*pos)->m_Flag != ground) {
				//ConnectPoint 첫번째꺼가 시작 위치

				if ((*pos)->m_Flag == capacity && (CCircultSimulation::m_CircultCase == linearDC || CCircultSimulation::m_CircultCase == nonlinearDC)) {
					NULL;
				}
				else {
					list<CPoint>::iterator StartConnectPostion = (*pos)->m_ConnectPoint.begin();
					list<CBaseElement*>::iterator lowpos = (*pos)->m_ConnectElement.begin();
					while (lowpos != (*pos)->m_ConnectElement.end()) {

						pBeforeElement = (*pos);
						CPoint ConnectPosition = (*lowpos)->ComparePosition((*pos)->m_Position + (*StartConnectPostion));
						if (ConnectPosition.x !=0 && ConnectPosition.y !=0 && (*lowpos)->m_CheckFlag != Orthodromic) {	//시작점과 연결이 되어있음	
							(*pos)->m_OutputElement.push_back((*lowpos));
							(*pos)->m_CheckFlag = TRUE;
							(*lowpos)->m_InputElement.push_back((*pos));

							RecusiveDecide((*lowpos));
							break;
						}
						++lowpos;
					}
				}
			}
			++pos;
		}
	}

	//Ground는 위에서 예외처리 되어 아래 루틴에서 진행방향 결정
	//바로 연결된 객체의 전체 링크에서 역 진행방향으로 연결
// 	list<CBaseElement*>::iterator groundpos = m_CircultList.begin();
// 	while (groundpos != m_CircultList.end()) {
// 		if ((*groundpos)->m_Flag == ground && (*groundpos)->m_OutputElement.size() == 0) {
// 
// 			if ((*groundpos)->m_ConnectElement.size() == 0) break;
// 
// 			list<CBaseElement*>::iterator Firstuppos = (*groundpos)->m_ConnectElement.begin();
// 			list<CBaseElement*>::iterator Findpos = (*Firstuppos)->m_ConnectElement.begin();
// 			while (Findpos != (*Firstuppos)->m_ConnectElement.end()) {
// 				if((*Findpos) != (*groundpos)) {
// 
// 					list<CBaseElement*>::iterator Searchpos = (*Firstuppos)->m_ConnectElement.begin();
// 					while (Searchpos != (*Firstuppos)->m_ConnectElement.end()) {
// 						list<CBaseElement*>::iterator doorpos = 
// 							find((*Findpos)->m_OutputElement.begin(), (*Findpos)->m_OutputElement.end(), (*Searchpos));
// 						if (doorpos != (*Findpos)->m_OutputElement.end()) {
// 							(*Findpos)->m_OutputElement.push_back((*Firstuppos));
// 							(*Firstuppos)->m_InputElement.push_back(*Findpos);
// 
// 							(*Firstuppos)->m_OutputElement.push_back((*groundpos));
// 							(*groundpos)->m_InputElement.push_back((*Findpos));
// 							break;
// 						}
// 						++Searchpos;
// 					}					
// 				}
// 				++Findpos;
// 			}
// 		}
// 		++groundpos;
// 	}
}

//진행 방향 결정 재귀 함수
int NodePretreatment::RecusiveDecide(CBaseElement *CurrentElement)
{
	CurrentElement->m_CheckFlag = Orthodromic; //들어오면 일단 체크 	

	//R-C & 구형파 일때 이미 그라운드 체크 되었기 때문에 그라운드면 정방향 리턴
	if (CurrentElement->m_GroundFlag == TRUE ) return Orthodromic;

	int Result = IsExceptionElement(CurrentElement);
	if (Result != Normal) { //현재 객체가 예외 소자인가?
		CurrentElement->m_CheckFlag = Normal;
		return Result;
	}


	//출구가 존재하냐?
	CBaseElement* OutputElement = IsOutletOK(CurrentElement);
	if (OutputElement != NULL) {									//출구가 존재하면 출구로 연결...
		CurrentElement->m_OutputElement.push_back(OutputElement);
		OutputElement->m_InputElement.push_back(CurrentElement);
		return Orthodromic;
	}

	//역방향이 존재하냐?
	if (IsAntidromic(CurrentElement)) {
		if (IsRemainOutlet(CurrentElement) == FALSE) {	//진행되지 않은 경로가 존재하지 않으면 역방향 리턴
			CurrentElement->m_CheckFlag = Antidromic;
			return Antidromic;
		}
	}

	//역방향 제외한 모든 객체 연결
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {

		int i = (*pos)->m_Flag;
		if (IsParentChild(CurrentElement, (*pos)) && (*pos)->m_CheckFlag != Orthodromic) {

			if ((*pos)->m_Flag == transistor && IsExceptionConnectPos(CurrentElement, (*pos))) {	//TR 일때는 이미 Input 구간이 2개라서 예외처리
				CurrentElement->m_OutputElement.push_back((*pos));
				(*pos)->m_InputElement.push_back(CurrentElement);				
			}
			else if ((*pos)->m_InputElement.empty()){
				CurrentElement->m_OutputElement.push_back((*pos));
				(*pos)->m_InputElement.push_back(CurrentElement);				
			}
		}
		++pos;
	}

	//연결된 outputlist가 없으면 역방향 리턴
	if (CurrentElement->m_OutputElement.size() == 0) {
		CurrentElement->m_CheckFlag = Normal;
		return Antidromic;
	}

	list<CBaseElement*> DeleteList;	//outputlist에서 삭제
	int AntidromicCount = 0;
	BOOL AllAntidromicFlag = FALSE;
	list<CBaseElement*>::iterator Recusivepos = CurrentElement->m_OutputElement.begin();

	while (Recusivepos != CurrentElement->m_OutputElement.end()) {
		pBeforeElement = CurrentElement;

		if ((*Recusivepos)->m_CheckFlag != Orthodromic) {
			if(RecusiveDecide((*Recusivepos)) == Antidromic) { //역방향 리턴 

				(*Recusivepos)->m_InputElement.clear();
				if (CurrentElement->m_OutputElement.size() == ++AntidromicCount) {	//출구가 하나면?
					CurrentElement->m_OutputElement.clear();
					CurrentElement->m_CheckFlag = Antidromic;
					AllAntidromicFlag = TRUE;
					break;
				}
				else {
					DeleteList.push_back((*Recusivepos));
				}
			}
		}
		++Recusivepos;
	}

	//delete pos
	list<CBaseElement*>::iterator Deletepos = DeleteList.begin();
	while (Deletepos != DeleteList.end()) {
		CurrentElement->m_OutputElement.remove((*Deletepos));
		++Deletepos;
	}

	if (AllAntidromicFlag) {
		CurrentElement->m_CheckFlag = Normal;
		return Antidromic;
	}
	return Orthodromic;
}

BOOL NodePretreatment::IsExceptionConnectPos(CBaseElement *parent, CBaseElement *child)
{
	if (parent->m_Flag == transistor) {
		int ConnectPosition = parent->ConnectPositionDirection(child->m_MoveTo);
		if (ConnectPosition == outputposition) {
			return TRUE;
		}

		ConnectPosition = parent->ConnectPositionDirection(child->m_LineTo);
		if (ConnectPosition == outputposition) {
			return TRUE;
		}

		return FALSE;
	}
	return TRUE;
}

int NodePretreatment::IsExceptionElement(CBaseElement* CurrentElement)
{
	int Result = Normal;
	switch (CurrentElement->m_Flag)
	{
	case ground:
		Result = Orthodromic;
		break;

	case capacity:
		{
			if (CCircultSimulation::m_CircultCase == linearDC || CCircultSimulation::m_CircultCase == nonlinearDC) {
				return Orthodromic;
			}
		}
		break;

	case dcpower:
	case transistor:
		{	
			//처음 power은 Output이 정의 되었기에 출구가 있는것으로 판단되어 처리 되었음
			//만약 처음 power이외에 다른 power가 있다면
			//ConnectPoint위치와 라인의 위치 비교해서 input방향인지 output방향인지 결정후 진행값 리턴(정방향,역방향)			

			int ConnectPosition = CurrentElement->ConnectPositionDirection(pBeforeElement->m_MoveTo);
			if (ConnectPosition == inputposition) {
				if (CurrentElement->m_Flag == transistor) {
					return Orthodromic;
				}
				return Normal;
			}

			ConnectPosition = CurrentElement->ConnectPositionDirection(pBeforeElement->m_LineTo);
			if (ConnectPosition == inputposition) {
				if (CurrentElement->m_Flag == transistor) {
					return Orthodromic;
				}
				return Normal;
			}

			return Antidromic;
		}
		break;
	}

	if (CurrentElement->m_Flag != lines) {
		//pBeforeElement와 비교하여 현재 다이오드의 첫번째가 일치하면 역방향
		list<CPoint>::iterator connectpos = CurrentElement->m_ConnectPoint.begin();
		CPoint DiodeInputPosition = CurrentElement->m_Position + (*connectpos);

		if (pBeforeElement->m_LineTo == DiodeInputPosition || pBeforeElement->m_MoveTo == DiodeInputPosition) {
			CurrentElement->m_AntidromicFlag = TRUE;
		}
		else {
			CurrentElement->m_AntidromicFlag = FALSE;
		}
	}

	return Result;
}

CBaseElement* NodePretreatment::IsGround(list<CBaseElement*> &ElementList)
{
	list<CBaseElement*>::iterator pos = ElementList.begin();
	while (pos != ElementList.end()) {
		if ((*pos)->m_Flag == ground) {
			return (*pos);
		}
		++pos;
	}
	return NULL;
}

BOOL NodePretreatment::IsAntidromic(CBaseElement* CurrentElement)
{
	//해당 객체의 ConnectElement 리스트에서 역방향이 존재하는지 검사
	BOOL AntidromicFlag = FALSE;
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {

		if (IsParentChild(CurrentElement, (*pos))) {	//부모에 현재 검사 값이 없고, 현재 위치의 input,output에도 중복이 안될때
			if ((*pos)->m_InputElement.size() != 0) { //역방향이
				AntidromicFlag = TRUE;
			}
		}
		++pos;
	}
	return AntidromicFlag;
}

CBaseElement* NodePretreatment::IsOutletOK(CBaseElement* CurrentElement)
{
	//출구 검사
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {

		//부모에 현재 검사 값이 없고, 현재 위치의 input,output에도 중복이 안될때
		if (IsParentChild(CurrentElement, (*pos))) {

			//해당 객체의 InputElement 리스트 중에 하나라도 CurrentElement객체의 리스트에 있는가?(출구)
			list<CBaseElement*>::iterator lowpos = (*pos)->m_InputElement.begin();
			while (lowpos != (*pos)->m_InputElement.end()) {

				list<CBaseElement*>::iterator findpos = 
					find(CurrentElement->m_ConnectElement.begin(), CurrentElement->m_ConnectElement.end(), (*lowpos));
				if (findpos != CurrentElement->m_ConnectElement.end() && (*findpos)->m_CheckFlag != Antidromic) {
					return(*pos);
				}
				++lowpos;
			}
		}
		++pos;
	}
	return NULL;
}





BOOL NodePretreatment::IsRemainOutlet(CBaseElement* CurrentElement)
{
	//ConnectElement중에 아무것도 체크가 않된것중, input, output 빈것이 있다면 리턴	
	list<CBaseElement*>::iterator Remainpos = CurrentElement->m_ConnectElement.begin();
	while (Remainpos != CurrentElement->m_ConnectElement.end()) {
		if ((*Remainpos)->m_CheckFlag == Normal 
			&& (*Remainpos)->m_InputElement.size() == 0 && (*Remainpos)->m_OutputElement.size() == 0)  {
				return TRUE;
		}

		//또한 아직 타보지 않은 연결 구간이 있다면 ConnectPosition이 있다면 진행 (연결구간이 다중인 소자들, 트랜지스터, Op-amp
		//즉 다중연결구간이 회로의 input핀에 연결된 라인이 inputElement 리스트에 들어 있는지 검사후 없으면 통과됨
		if ((*Remainpos)->m_CheckFlag == Normal) {

	//		int iCount = (int)(*Remainpos)->m_ConnectPoint.size() -1;
			list<CPoint>::iterator connectpos = (*Remainpos)->m_ConnectPoint.begin();
			while (connectpos != (*Remainpos)->m_ConnectPoint.end()) {

				list<CBaseElement*>::iterator lowpos = (*Remainpos)->m_ConnectElement.begin();
				while (lowpos != (*Remainpos)->m_ConnectElement.end()) {

					list<CBaseElement*>::iterator duplicatepos = find((*Remainpos)->m_InputElement.begin(), (*Remainpos)->m_InputElement.end(), (*lowpos));
					if (duplicatepos == (*Remainpos)->m_InputElement.end()) {
						if ((*lowpos)->m_Flag == lines && ((*lowpos)->m_LineTo == (*Remainpos)->m_Position + (*connectpos)
							|| (*lowpos)->m_MoveTo == (*Remainpos)->m_Position + (*connectpos))) {

								if ((*lowpos) == CurrentElement) {
									return TRUE;
								}
						}
					}
					++lowpos;
				}
			//	if (--iCount == 0) break;
				++connectpos;
			}
		}
		++Remainpos;
	}

	return FALSE;
}


BOOL NodePretreatment::IsParentChild(CBaseElement *CurrentElement, CBaseElement* CompareLink)
{
	//CompareLink가 현재 input, oupt에 값이 있느지 체크
	list<CBaseElement*>::iterator InputDuplicatepos =
		find(CurrentElement->m_InputElement.begin(), CurrentElement->m_InputElement.end(), CompareLink);
	list<CBaseElement*>::iterator OutputDuplicatepos = 
		find(CurrentElement->m_OutputElement.begin(), CurrentElement->m_OutputElement.end(), CompareLink);

	if (InputDuplicatepos != CurrentElement->m_InputElement.end()
		|| OutputDuplicatepos != CurrentElement->m_OutputElement.end()) {
			return FALSE;
	}

	//한놈이 주체가 되면 다른 두거리는 서로의 링크를 몰라야 하기 때문에..
	//다른 두거링중 하나가 호출하며 부모로 가서 값이 있나 확인
	list<CBaseElement*>::iterator pos = CurrentElement->m_InputElement.begin();
	while (pos != CurrentElement->m_InputElement.end()) {
		list<CBaseElement*>::iterator findpos = find((*pos)->m_OutputElement.begin(), (*pos)->m_OutputElement.end(), CompareLink);
		if (findpos != (*pos)->m_OutputElement.end()) {
			return FALSE;
		}
		++pos;
	}	
	return TRUE;
}

BOOL NodePretreatment::CheckCircult()
{
	//검사 조건
	//1. 그라운드 꼭 필요
	//2. 전류, 전원 꼭 필요
	//3. 모든 회로가 연결이 되어 있어야 함
	//4. 프로브에 꼭 InputElement에 값이 있어야 함

	//예외조건
	//1. AC 회로에서 캐패시터는 중복안됨
	//2. 구형파는 R-C회로만 가능하다.


	PrintOutString(CString(_T(">> 회로의 상태를 검사합니다.")));
	PrintOutString(CString(_T("==============================================")));

	int TotalCount = 0;
	int WaveModel = sign;
	int CapacityCount = 0;
	BOOL nFlag = TRUE, GroundFlag = FALSE, PowerFlag = FALSE;
	list<CBaseElement*>::iterator pos = m_CircultList.begin();
	while (pos != m_CircultList.end()) {

		//기본 검사
		if ((*pos)->m_Flag != ground && (*pos)->m_Flag != prov && (*pos)->m_ConnectElement.size() < 2) {
			PrintElementInfo((*pos));			
			nFlag = FALSE;
			TotalCount++;
		}

		if ((*pos)->m_Flag == ground && (*pos)->m_ConnectElement.size() == 1) GroundFlag = TRUE;
		if ((*pos)->m_Flag == dcpower || (*pos)->m_Flag == acpower) PowerFlag = TRUE;

		//예외 조건
		if ((*pos)->m_Flag == acpower) {
			
			list<ElementData> GetList;
			(*pos)->GetProperties(GetList);

			list<ElementData>::iterator pos = GetList.begin();
			while (pos != GetList.end()) {
				if ((*pos).Text == CString(_T("WaveModel"))) {
					WaveModel = (int)(*pos).Value;
					break;
				}
				++pos;
			}
		}

		if ((*pos)->m_Flag == capacity) CapacityCount++;	
		++pos;
	}

	if (CCircultSimulation::m_RLCCase != rc && WaveModel == square) {	//RC회로가 아니면서 구형파를 선택시 
		PrintOutString(CString(_T(">> R-C회로일때만 구형파를 선택할 수 있습니다.")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	if (CCircultSimulation::m_RLCCase == rc && CapacityCount > 1 && WaveModel == square) {
		PrintOutString(CString(_T(">> R-C회로에서 구형파 선택시 캐패시터는 중복이 불가능합니다..")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	if (!PowerFlag) {
		PrintOutString(CString(_T(">> 전류 및 전원부가 존재 하지 않습니다.")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	if (!GroundFlag) {
		PrintOutString(CString(_T(">> 그라운드가 존재 하지 않습니다.")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	CString FinalString;
	FinalString.Format(_T("====== 오류 : %d개, 경고 : 0개"), TotalCount);
	PrintOutString(FinalString);
	return nFlag;
}

void NodePretreatment::PrintElementInfo(CBaseElement *pElement)
{
	CString ElementName(_T("문제 소자 : "));
	ElementName +=  pElement->m_Name;
	CString ElementNumber;
	ElementNumber.Format(_T("%d"), pElement->m_ElementNumber);
	ElementName += ElementNumber;
	ElementName += _T(" (회로의 연결이 부적합 합니다)");
	PrintOutString(ElementName);

	list<ElementData> DataList;
	pElement->GetProperties(DataList);

	list<ElementData>::iterator pos = DataList.begin();
	while (pos != DataList.end()) {
		CString Output((*pos).Text);
		CString Append;
		Append.Format(_T(" : %0.1f"), (*pos).Value);
		Output += Append;
		PrintOutString(Output);
		++pos;
	}
	PrintOutString(CString(_T("----------------------------------------------")));
}

void NodePretreatment::PrintOutString(CString &str)
{	
	m_pFrame->m_wndOutput.InsertString(str);
}

