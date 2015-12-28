#include "StdAfx.h"
#include "NonLinearDCFormula.h"
#include "BaseElement.h"
#include <algorithm>
#include <math.h>


typedef struct _SearchRaphson
{
	int Flag;

	_SearchRaphson()
	{
		Flag = leq;
	}

	bool operator()(CBaseElement *rs) const {
		return Flag == rs->m_Flag;
	}
}SearchRaphson;

NonLinearDCFormula::NonLinearDCFormula(void)
{
	m_StencilBuffer = NULL;
	m_Result = NULL;
	m_BeforeResult = NULL;
}

NonLinearDCFormula::~NonLinearDCFormula(void)
{
	if (m_StencilBuffer != NULL) {
		delete m_StencilBuffer[0];
		delete m_StencilBuffer;
	}

	if (m_Result != NULL) {
		delete m_Result;
	}

// 	if (m_BeforeResult != NULL) {
// 		delete m_BeforeResult;
// 	}
}

BOOL NonLinearDCFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	m_Voltage = 0;
	m_MaxCount = 30;
	//copy
	m_NodeList.clear();
	m_NodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_NodeList.begin());


	//RaphsonList Initialize
	int NodeSize = 0;
	m_NewtonRaphsonList.clear();
	list<Node>::iterator nodepos = NodeList.begin();
	while (nodepos != NodeList.end()) {

		RaphsonNode NewNode;
		list<CBaseElement*>::iterator findpos = (*nodepos).OutputElement.begin();
		while (findpos != (*nodepos).OutputElement.end()) {
			if ((*findpos)->m_Flag == leq) {
				
				RaphsonData NewData;
				NewData.LeqElement = (*findpos);

				list<ElementData> GetList;
				(*findpos)->GetProperties(GetList);

				list<ElementData>::iterator searchpos = GetList.begin();
				while (searchpos != GetList.end()) {
					if ((*searchpos).Text == CString(_T("GeqElement"))) {
						NewData.GeqElement = (CBaseElement*)(*searchpos).pElement;
					}
					else if ((*searchpos).Text == CString(_T("OperatingPoint"))) {

					}
					++searchpos;
				}
				
				NewData.OutputNodeNumber = (*nodepos).NodeNumber;
				NewData.InputNodeNumber = IsGetNextNode((*nodepos).NodeNumber, NewData.GeqElement);
				NewData.Value = 100;	
				NewData.DifferenceVlaue = 100;
				NewData.LimitVoltage = -100;
				NewData.LimitFlag = FALSE;
				NewNode.RaphsonList.push_back(NewData);
			}
			++findpos;
		}

		if (!NewNode.RaphsonList.empty()) m_NewtonRaphsonList.push_back(NewNode);				

		//node size check (addition node 포함)
		SearchRaphson NewSearch;
		NewSearch.Flag = dcpower;
		findpos = find_if((*nodepos).AllElement.begin(), (*nodepos).AllElement.end(), NewSearch);
		if (findpos != (*nodepos).AllElement.end()) {
			++NodeSize;
		}
		++NodeSize;
		++nodepos;
	}
		
	m_NodeSize = NodeSize;
	m_BeforeResult = new double[m_NodeSize];

	//Newton raphson method 
	do 
	{
		if (--m_MaxCount == 0) {
			AfxMessageBox(_T("회로가 일반적이지 않아 뉴튼 랍손법이 적용되지 않습니다."));
			return FALSE;
		}

		NonlinearStencilConvert(NodeSize);

		TRACE("\n");
		for (int iCount = 0; iCount < NodeSize; iCount++) {
			for (int jCount = 0; jCount < NodeSize+1; jCount++) {
				TRACE("%lf ", m_StencilBuffer[iCount][jCount]);
			}
			TRACE("\n");
		}
		
		GaussianElimination(NodeSize);
	} while (IsCompareAndInsert());

	//result->Data  
	IPCDataConvert(Datalist, Provlist);
	TRACE("============================================================\n");
	return TRUE;
}


double NonLinearDCFormula::RoundXL(double x, int digits) 
{
	char format[5 + 1] = "%.";
	char temp[2 + 1];
	char result[500 + 1];

	if (digits > 99) digits = 99; 

	_itoa_s(digits, temp, 10); 
	strcat_s(format, temp); 
	strcat_s(format, "f"); 

	sprintf_s(result, format, x); 
	return atof(result); 
}

BOOL NonLinearDCFormula::IsCompareAndInsert()
{
	//initialize
	{
		list<Node>::iterator nodepos = m_NodeList.begin();
		while (nodepos != m_NodeList.end()) {
			
			list<CBaseElement*>::iterator elementpos = (*nodepos).AllElement.begin();
			while (elementpos != (*nodepos).AllElement.end()) {
				(*elementpos)->m_UpdataFlag = FALSE;
				++elementpos;	
			}
			++nodepos;
		}
	}

	BOOL nFlag = FALSE;
	list<RaphsonNode>::iterator pos = m_NewtonRaphsonList.begin();
	while (pos != m_NewtonRaphsonList.end()) {

		list<RaphsonData>::iterator datapos = (*pos).RaphsonList.begin();
		while (datapos != (*pos).RaphsonList.end()) {

			double CurrentDifferenceVlaue = 0;
			if ((*datapos).InputNodeNumber != -1) {
				CurrentDifferenceVlaue = m_Result[(*datapos).OutputNodeNumber] - m_Result[(*datapos).InputNodeNumber];
			}
			else {
				CurrentDifferenceVlaue = m_Result[(*datapos).OutputNodeNumber];
			}
 
			TRACE("Result : %lf\n", m_Result[(*datapos).OutputNodeNumber]);
			if (RoundXL((*datapos).DifferenceVlaue, 6) != RoundXL(CurrentDifferenceVlaue, 6)) {

					nFlag = TRUE;
					(*datapos).Value = m_Result[(*datapos).OutputNodeNumber];
					(*datapos).DifferenceVlaue = CurrentDifferenceVlaue;

					TRACE("CurrentDifferenceVlaue : %.15lf\n", CurrentDifferenceVlaue);

					ElementData	NewData;
					NewData.Text = CString(_T("OperatingPoint"));
					NewData.Value = CurrentDifferenceVlaue;

					list<ElementData> UpdateList;
					UpdateList.push_back(NewData);

					(*datapos).GeqElement->OnEditProperties(&UpdateList);
					(*datapos).LeqElement->OnEditProperties(&UpdateList);
			}
			++datapos;
		}
		++pos;
	}

	for (int iCount = 0; iCount < m_NodeSize; iCount++) {
		m_BeforeResult[iCount] = m_Result[iCount];
	}
	return nFlag;
}



int NonLinearDCFormula::IsGetNextNode(int CurrentNodeNumber, CBaseElement* CompareElement)
{
	//CurrentNodeNumber가 아니면서 CompareElement를 포함하는 노드

	list<Node>::iterator nodepos = m_NodeList.begin();
	while (nodepos != m_NodeList.end()) {
		if ((*nodepos).NodeNumber != CurrentNodeNumber) {
			
			list<CBaseElement*>::iterator duplicatepos = find((*nodepos).AllElement.begin(), (*nodepos).AllElement.end(), CompareElement);
			if (duplicatepos != (*nodepos).AllElement.end()) {
				return (*nodepos).NodeNumber;
			}
		}
		++nodepos;
	}

	return -1;
}

// int NonLinearDCFormula::IsGetNextNode(int CurrentNodeNumber)
// {
// 	list<Node>::iterator nodepos = m_NodeList.begin();
// 	while (nodepos != m_NodeList.end()) {
// 		if ((*nodepos).NodeNumber == CurrentNodeNumber) {
// 			break;
// 		}
// 		++nodepos;
// 	}
// 
// 	if (nodepos != m_NodeList.end()) {
// 
// 		list<CBaseElement*>::iterator searchnode = (*nodepos).OutputElement.begin();
// 		while (searchnode != (*nodepos).OutputElement.end()) {
// 			if ((*searchnode)->m_Flag == geq || (*searchnode)->m_Flag == leq) {
// 				break;
// 			}
// 			++searchnode;
// 		}
// 
// 		if (searchnode != (*nodepos).OutputElement.end()) {
// 			list<Node>::iterator lownodepos = m_NodeList.begin();
// 			while (lownodepos != m_NodeList.end()) {
// 				if ((*lownodepos).NodeNumber != CurrentNodeNumber) {
// 
// 					list<CBaseElement*>::iterator duplicatepos = find((*lownodepos).AllElement.begin(), (*lownodepos).AllElement.end(), (*searchnode));
// 					if (duplicatepos != (*lownodepos).AllElement.end()) {
// 						return (*lownodepos).NodeNumber;
// 					}
// 				}
// 				++lownodepos;
// 			}
// 		}
// 	}
// 	return -1;
// }

CBaseElement* NonLinearDCFormula::SearchElement(Node &CurrentNode, int Flag)
{
	list<CBaseElement*>::iterator pos = CurrentNode.AllElement.begin();
	while (pos != CurrentNode.AllElement.end()) {
		if ((*pos)->m_Flag == Flag && (*pos)->m_UpdataFlag == FALSE) {
			(*pos)->m_UpdataFlag = TRUE;
			return (*pos);
		}
		++pos;
	}
	return NULL;
}

double NonLinearDCFormula::GetCombineLeq(Node &CurrentNode)
{
	double CombineValue = 0;
	list<CBaseElement*>::iterator inputpos = CurrentNode.InputElement.begin();
	while (inputpos != CurrentNode.InputElement.end()) {
		if ((*inputpos)->m_Flag == leq) {
			CombineValue += GetElementProperty((*inputpos),  CString(_T("Leq")));
		}
		++inputpos;
	}

	list<CBaseElement*>::iterator outputpos = CurrentNode.OutputElement.begin();
	while (outputpos != CurrentNode.OutputElement.end()) {
		if ((*outputpos)->m_Flag == leq) {
			CombineValue -= GetElementProperty((*outputpos),  CString(_T("Leq")));
		}
		++outputpos;
	}
	return CombineValue;
}



int NonLinearDCFormula::IsDiodeInclusion(Node &CurrentNode)
{
	list<CBaseElement*>::iterator pos = CurrentNode.AllElement.begin();
	while (pos != CurrentNode.AllElement.end()) {
		if ((*pos)->m_Flag == leq || (*pos)->m_Flag == geq) {
			return TRUE;
		}
		++pos;
	}
	return FALSE;
}

void NonLinearDCFormula::IPCDataConvert(list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	list<CBaseElement*>::iterator provpos = Provlist.begin();
	while (provpos != Provlist.end()) {

		if ((*provpos)->m_InputElement.size() > 0) {
			list<CBaseElement*>::iterator CompareElement = (*provpos)->m_InputElement.begin();

			list<Node>::iterator nodepos = m_NodeList.begin();
			while (nodepos != m_NodeList.end()) {

				list<CBaseElement*>::iterator searchpos = find((*nodepos).LineElement.begin(), (*nodepos).LineElement.end(), (*CompareElement));
				if (searchpos != (*nodepos).LineElement.end()) {

					IPCData NewData;
					for (int iCount = 0; iCount < DivideSector; iCount++) {
						NewData.GrapehValue[iCount] = m_Result[(*nodepos).NodeNumber];
					}				
					NewData.ProvNumber = (*provpos)->m_ElementNumber;
					Datalist.push_back(NewData);
				}
				++nodepos;
			}
		}
		++provpos;
	}
}



void NonLinearDCFormula::NonlinearStencilConvert(int NodeSize)
{
	m_StencilBuffer = new double*[NodeSize];
	m_StencilBuffer[0] = new double[NodeSize * (NodeSize + 1)];
	memset(m_StencilBuffer[0], 0, sizeof(double) * NodeSize * (NodeSize + 1));
	for	(int iCount = 1; iCount < NodeSize; iCount++) {
		m_StencilBuffer[iCount] = m_StencilBuffer[iCount -1] + (NodeSize + 1);
	}

	int LineNumber = 0;
	list<Node> AdditionList;
	list<Node>::iterator pos = m_NodeList.begin();
	while (pos != m_NodeList.end()) {

		if ((*pos).FirstFlag) {		//시작 위치 노드이면 상수값 적용			
			list<CBaseElement*>::iterator lowpos = (*pos).AllElement.begin();
			while (lowpos != (*pos).AllElement.end()) {
				if ((*lowpos)->m_Flag == dcpower) {

					//기본 변환 수행
					m_StencilBuffer[LineNumber][LineNumber] = 1.0;
					m_StencilBuffer[LineNumber][NodeSize] = GetElementProperty((*lowpos), CString(_T("Volatage")));
					m_Voltage = m_StencilBuffer[LineNumber][NodeSize];
					AdditionList.push_back((*pos));			//마지막에 추가 스텐실 적용 Is에 대한 메트릭스
 					break;
				}
				++lowpos;
			}		
		}
		else {

			GetInputCombinedResistance((*pos), m_StencilBuffer[LineNumber]);		
			m_StencilBuffer[LineNumber][LineNumber] += GetCombinedResistance(*pos);
			GetOutputCombinedResistance((*pos), m_StencilBuffer[LineNumber]);

			if (IsDiodeInclusion((*pos))) {
				m_StencilBuffer[LineNumber][NodeSize] = GetCombineLeq((*pos));
			}
		}
		++LineNumber;
		++pos;
	}

	//추가 작업
	pos = AdditionList.begin();
	while (pos != AdditionList.end()) {

		if (IsDiodeInclusion((*pos))) {
			m_StencilBuffer[LineNumber][(*pos).NodeNumber] -= GetCombinedResistance(*pos);
			m_StencilBuffer[LineNumber][LineNumber] = 1.0;
			GetOutputPlusCombinedResistance((*pos), m_StencilBuffer[LineNumber]);

			list<CBaseElement*>::iterator outputpos = (*pos).OutputElement.begin();
			while (outputpos != (*pos).OutputElement.end()) {
				if ((*outputpos)->m_Flag == leq) {
					m_StencilBuffer[LineNumber][NodeSize] += GetElementProperty((*outputpos),  CString(_T("Leq")));
				}
				++outputpos;
			}			 
		}
		else {
			m_StencilBuffer[LineNumber][(*pos).NodeNumber] -= GetCombinedResistance(*pos);
			m_StencilBuffer[LineNumber][LineNumber] = 1.0;
			GetOutputPlusCombinedResistance((*pos), m_StencilBuffer[LineNumber]);

		}
		++LineNumber;
		++pos;
	}
}

double NonLinearDCFormula::GetCombinedResistance(Node &CurrentNode)
{
	double CombinedResistance = 0;

	list<CBaseElement*>::iterator pos = CurrentNode.AllElement.begin();
	while (pos != CurrentNode.AllElement.end()) {

		switch ((*pos)->m_Flag) {
			case resistance:
				{
					double CurrentValue = GetElementProperty((*pos),  CString(_T("Resistance")));
					CombinedResistance += (1/CurrentValue);
				}
				break;

			case geq:
				{
					double CurrentValue = GetElementProperty((*pos),  CString(_T("Geq")));
					CombinedResistance += CurrentValue;
				}
				break;
		}
		++pos;
	}
	return CombinedResistance;
}

double NonLinearDCFormula::GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
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

			case geq:
				StencilBuffer[NodeNumber] -= GetElementProperty((*inputpos),  CString(_T("Geq")));
				break;
			}
		}
		++inputpos;
	}

	return CombinedResistance;
}


double NonLinearDCFormula::GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
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

			case geq:
				StencilBuffer[NodeNumber] -= GetElementProperty((*outputpos),  CString(_T("Geq")));
				break;
			}
		}
		++outputpos;
	}

	return CombinedResistance;
}

double NonLinearDCFormula::GetOutputPlusCombinedResistance(Node &CurrentNode, double *StencilBuffer)
{
	double CombinedResistance = 0;

	list<CBaseElement*>::iterator outputpos = CurrentNode.OutputElement.begin();
	while (outputpos != CurrentNode.OutputElement.end()) {

		int NodeNumber = IsDuplicateNode(CurrentNode, (*outputpos));
		if (NodeNumber != -1) {

			switch ((*outputpos)->m_Flag) {
			case resistance:
				StencilBuffer[NodeNumber] += (1/GetElementProperty((*outputpos),  CString(_T("Resistance"))));
				break;

			case geq:
				StencilBuffer[NodeNumber] += GetElementProperty((*outputpos),  CString(_T("Geq")));
				break;
			}
		}
		++outputpos;
	}

	return CombinedResistance;
}

double NonLinearDCFormula::GetElementProperty(CBaseElement *pElement, CString &PropertyString)
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

int NonLinearDCFormula::IsDuplicateNode(Node &BaseNode, CBaseElement *pElement)
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

BOOL NonLinearDCFormula::GaussianElimination(int NodeSize)
{
	double **a, *x, eps=1.e-10, sum;
	int i,j,k,n,np,ip;

	n = NodeSize;
	np=n+1;
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
				AfxMessageBox(_T("Error: 행렬의 대각원소가 0입니다."));
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
	for(i=1;i<=n;i++) {
		m_Result[i-1] = x[i];   //결과 도출.
		TRACE("X%d, Result : %f\n", i, m_Result[i-1]);
	}


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
