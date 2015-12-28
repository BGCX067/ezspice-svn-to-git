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

void NodePretreatment::AheadProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList) //��ó��
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

void NodePretreatment::RearProcessing(list<CBaseElement*> &CircultList , list<Node> &NodeList)	//��ó��
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

				//ĳ�н��� ���� ��� ����
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
								//Geq, Leq ���� �� ��ü
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

								//Diode�� ����� ��ü�� ��ü
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

								//Diode�� ����� ��ü�� ��ü
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
				//AC���� üũ�� �ֱ⿡ ���� ���ϱ� ������ AC ���ο��� �ؾ� �ҵ�
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
	PrintOutString(CString(_T(">> �������� �����մϴ�.")));

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

	//Circuit preferenceRnak	//ȸ�� ���� �켱 ���� ����
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

void NodePretreatment::NodeDistinction(list<Node> &NodeList)	//��� �Ǻ� ����
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


	//������ üũ
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

void NodePretreatment::GroundExtentSearch() //Ground ���� Ž������
{
	list<CBaseElement*>::iterator pos = m_CircultList.begin();
	while (pos != m_CircultList.end()) {
		if ((*pos)->m_Flag == ground) {
			RecusiveExtent(NULL, (*pos));
		}
		++pos;
	}
}

int NodePretreatment::RecusiveExtent(CBaseElement *parent, CBaseElement *CurrentElement)	//Ground ���� Ž�� ����Լ�
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

void NodePretreatment::DirectionDecide()	//������� ���� ���� ��ƾ
{
	//�Ϲ������� �켱������ �����Ͽ� ���� ���� ������ ���Ѵ�
	//������� �������� �ΰ����� �ִµ�
	//ù��°�� �Ϲ����� ����̴�. �� ��쿡�� ���кθ� �������� ������ �������� �����ϸ� ��������� �����Ѵ�.
	//Ư¡�� ���Ա��� ������ �ִ� ���ڰ� ������ �ֱ� ������ ����ó���� �Ǿ� �ִ�.

	//�ι�° ���� AC ȸ�� && R-R ȸ�� && ������ ���ϰ���̴�. �� ��쿡�� ĳ�н��͸� �߽����� �¿�� ��������� �����Ѵ�.
	//�̰��� R-Cȸ���� �������� ��� Ư���� ���Ͽ� �ٶ󺸴� �ð��� �ռ� ������ �ʿ��ϱ� ������ ��� ������ �����ϰ� �ϱ� ����
	//�̷��� ����ȴ�. �׷��� �ߺ� Ž���� ���� ���� �׶��� ������ ���� üũ ���� �����Ѵ�.

	//�켱 ���� �̵�
	m_CircultList.sort(CircuitElementRankingsort());

	if (CCircultSimulation::m_CircultCase == linearAC && CCircultSimulation::m_RLCCase == rc && CCircultSimulation::m_ACData.WaveModel == square) {		
		////R-C ȸ�� �϶��� ĳ�н��͸� �߽� ���� ����
		//�׶��带 �̸� üũ
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
		//�Ϲ����� ��� ����
		list<CBaseElement*>::iterator pos = m_CircultList.begin();
		while (pos != m_CircultList.end()) {
			if ((*pos)->m_Flag != lines && (*pos)->m_Flag != ground) {
				//ConnectPoint ù��°���� ���� ��ġ

				if ((*pos)->m_Flag == capacity && (CCircultSimulation::m_CircultCase == linearDC || CCircultSimulation::m_CircultCase == nonlinearDC)) {
					NULL;
				}
				else {
					list<CPoint>::iterator StartConnectPostion = (*pos)->m_ConnectPoint.begin();
					list<CBaseElement*>::iterator lowpos = (*pos)->m_ConnectElement.begin();
					while (lowpos != (*pos)->m_ConnectElement.end()) {

						pBeforeElement = (*pos);
						CPoint ConnectPosition = (*lowpos)->ComparePosition((*pos)->m_Position + (*StartConnectPostion));
						if (ConnectPosition.x !=0 && ConnectPosition.y !=0 && (*lowpos)->m_CheckFlag != Orthodromic) {	//�������� ������ �Ǿ�����	
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

	//Ground�� ������ ����ó�� �Ǿ� �Ʒ� ��ƾ���� ������� ����
	//�ٷ� ����� ��ü�� ��ü ��ũ���� �� ����������� ����
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

//���� ���� ���� ��� �Լ�
int NodePretreatment::RecusiveDecide(CBaseElement *CurrentElement)
{
	CurrentElement->m_CheckFlag = Orthodromic; //������ �ϴ� üũ 	

	//R-C & ������ �϶� �̹� �׶��� üũ �Ǿ��� ������ �׶���� ������ ����
	if (CurrentElement->m_GroundFlag == TRUE ) return Orthodromic;

	int Result = IsExceptionElement(CurrentElement);
	if (Result != Normal) { //���� ��ü�� ���� �����ΰ�?
		CurrentElement->m_CheckFlag = Normal;
		return Result;
	}


	//�ⱸ�� �����ϳ�?
	CBaseElement* OutputElement = IsOutletOK(CurrentElement);
	if (OutputElement != NULL) {									//�ⱸ�� �����ϸ� �ⱸ�� ����...
		CurrentElement->m_OutputElement.push_back(OutputElement);
		OutputElement->m_InputElement.push_back(CurrentElement);
		return Orthodromic;
	}

	//�������� �����ϳ�?
	if (IsAntidromic(CurrentElement)) {
		if (IsRemainOutlet(CurrentElement) == FALSE) {	//������� ���� ��ΰ� �������� ������ ������ ����
			CurrentElement->m_CheckFlag = Antidromic;
			return Antidromic;
		}
	}

	//������ ������ ��� ��ü ����
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {

		int i = (*pos)->m_Flag;
		if (IsParentChild(CurrentElement, (*pos)) && (*pos)->m_CheckFlag != Orthodromic) {

			if ((*pos)->m_Flag == transistor && IsExceptionConnectPos(CurrentElement, (*pos))) {	//TR �϶��� �̹� Input ������ 2���� ����ó��
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

	//����� outputlist�� ������ ������ ����
	if (CurrentElement->m_OutputElement.size() == 0) {
		CurrentElement->m_CheckFlag = Normal;
		return Antidromic;
	}

	list<CBaseElement*> DeleteList;	//outputlist���� ����
	int AntidromicCount = 0;
	BOOL AllAntidromicFlag = FALSE;
	list<CBaseElement*>::iterator Recusivepos = CurrentElement->m_OutputElement.begin();

	while (Recusivepos != CurrentElement->m_OutputElement.end()) {
		pBeforeElement = CurrentElement;

		if ((*Recusivepos)->m_CheckFlag != Orthodromic) {
			if(RecusiveDecide((*Recusivepos)) == Antidromic) { //������ ���� 

				(*Recusivepos)->m_InputElement.clear();
				if (CurrentElement->m_OutputElement.size() == ++AntidromicCount) {	//�ⱸ�� �ϳ���?
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
			//ó�� power�� Output�� ���� �Ǿ��⿡ �ⱸ�� �ִ°����� �ǴܵǾ� ó�� �Ǿ���
			//���� ó�� power�̿ܿ� �ٸ� power�� �ִٸ�
			//ConnectPoint��ġ�� ������ ��ġ ���ؼ� input�������� output�������� ������ ���ప ����(������,������)			

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
		//pBeforeElement�� ���Ͽ� ���� ���̿����� ù��°�� ��ġ�ϸ� ������
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
	//�ش� ��ü�� ConnectElement ����Ʈ���� �������� �����ϴ��� �˻�
	BOOL AntidromicFlag = FALSE;
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {

		if (IsParentChild(CurrentElement, (*pos))) {	//�θ� ���� �˻� ���� ����, ���� ��ġ�� input,output���� �ߺ��� �ȵɶ�
			if ((*pos)->m_InputElement.size() != 0) { //��������
				AntidromicFlag = TRUE;
			}
		}
		++pos;
	}
	return AntidromicFlag;
}

CBaseElement* NodePretreatment::IsOutletOK(CBaseElement* CurrentElement)
{
	//�ⱸ �˻�
	list<CBaseElement*>::iterator pos = CurrentElement->m_ConnectElement.begin();
	while (pos != CurrentElement->m_ConnectElement.end()) {

		//�θ� ���� �˻� ���� ����, ���� ��ġ�� input,output���� �ߺ��� �ȵɶ�
		if (IsParentChild(CurrentElement, (*pos))) {

			//�ش� ��ü�� InputElement ����Ʈ �߿� �ϳ��� CurrentElement��ü�� ����Ʈ�� �ִ°�?(�ⱸ)
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
	//ConnectElement�߿� �ƹ��͵� üũ�� �ʵȰ���, input, output ����� �ִٸ� ����	
	list<CBaseElement*>::iterator Remainpos = CurrentElement->m_ConnectElement.begin();
	while (Remainpos != CurrentElement->m_ConnectElement.end()) {
		if ((*Remainpos)->m_CheckFlag == Normal 
			&& (*Remainpos)->m_InputElement.size() == 0 && (*Remainpos)->m_OutputElement.size() == 0)  {
				return TRUE;
		}

		//���� ���� Ÿ���� ���� ���� ������ �ִٸ� ConnectPosition�� �ִٸ� ���� (���ᱸ���� ������ ���ڵ�, Ʈ��������, Op-amp
		//�� ���߿��ᱸ���� ȸ���� input�ɿ� ����� ������ inputElement ����Ʈ�� ��� �ִ��� �˻��� ������ �����
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
	//CompareLink�� ���� input, oupt�� ���� �ִ��� üũ
	list<CBaseElement*>::iterator InputDuplicatepos =
		find(CurrentElement->m_InputElement.begin(), CurrentElement->m_InputElement.end(), CompareLink);
	list<CBaseElement*>::iterator OutputDuplicatepos = 
		find(CurrentElement->m_OutputElement.begin(), CurrentElement->m_OutputElement.end(), CompareLink);

	if (InputDuplicatepos != CurrentElement->m_InputElement.end()
		|| OutputDuplicatepos != CurrentElement->m_OutputElement.end()) {
			return FALSE;
	}

	//�ѳ��� ��ü�� �Ǹ� �ٸ� �ΰŸ��� ������ ��ũ�� ����� �ϱ� ������..
	//�ٸ� �ΰŸ��� �ϳ��� ȣ���ϸ� �θ�� ���� ���� �ֳ� Ȯ��
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
	//�˻� ����
	//1. �׶��� �� �ʿ�
	//2. ����, ���� �� �ʿ�
	//3. ��� ȸ�ΰ� ������ �Ǿ� �־�� ��
	//4. ���κ꿡 �� InputElement�� ���� �־�� ��

	//��������
	//1. AC ȸ�ο��� ĳ�н��ʹ� �ߺ��ȵ�
	//2. �����Ĵ� R-Cȸ�θ� �����ϴ�.


	PrintOutString(CString(_T(">> ȸ���� ���¸� �˻��մϴ�.")));
	PrintOutString(CString(_T("==============================================")));

	int TotalCount = 0;
	int WaveModel = sign;
	int CapacityCount = 0;
	BOOL nFlag = TRUE, GroundFlag = FALSE, PowerFlag = FALSE;
	list<CBaseElement*>::iterator pos = m_CircultList.begin();
	while (pos != m_CircultList.end()) {

		//�⺻ �˻�
		if ((*pos)->m_Flag != ground && (*pos)->m_Flag != prov && (*pos)->m_ConnectElement.size() < 2) {
			PrintElementInfo((*pos));			
			nFlag = FALSE;
			TotalCount++;
		}

		if ((*pos)->m_Flag == ground && (*pos)->m_ConnectElement.size() == 1) GroundFlag = TRUE;
		if ((*pos)->m_Flag == dcpower || (*pos)->m_Flag == acpower) PowerFlag = TRUE;

		//���� ����
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

	if (CCircultSimulation::m_RLCCase != rc && WaveModel == square) {	//RCȸ�ΰ� �ƴϸ鼭 �����ĸ� ���ý� 
		PrintOutString(CString(_T(">> R-Cȸ���϶��� �����ĸ� ������ �� �ֽ��ϴ�.")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	if (CCircultSimulation::m_RLCCase == rc && CapacityCount > 1 && WaveModel == square) {
		PrintOutString(CString(_T(">> R-Cȸ�ο��� ������ ���ý� ĳ�н��ʹ� �ߺ��� �Ұ����մϴ�..")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	if (!PowerFlag) {
		PrintOutString(CString(_T(">> ���� �� �����ΰ� ���� ���� �ʽ��ϴ�.")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	if (!GroundFlag) {
		PrintOutString(CString(_T(">> �׶��尡 ���� ���� �ʽ��ϴ�.")));
		PrintOutString(CString(_T("----------------------------------------------")));
		nFlag = FALSE;
		TotalCount++;
	}

	CString FinalString;
	FinalString.Format(_T("====== ���� : %d��, ��� : 0��"), TotalCount);
	PrintOutString(FinalString);
	return nFlag;
}

void NodePretreatment::PrintElementInfo(CBaseElement *pElement)
{
	CString ElementName(_T("���� ���� : "));
	ElementName +=  pElement->m_Name;
	CString ElementNumber;
	ElementNumber.Format(_T("%d"), pElement->m_ElementNumber);
	ElementName += ElementNumber;
	ElementName += _T(" (ȸ���� ������ ������ �մϴ�)");
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

