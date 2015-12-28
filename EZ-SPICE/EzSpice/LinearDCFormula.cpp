#include "StdAfx.h"
#include "LinearDCFormula.h"
#include "BaseElement.h"
#include <algorithm>
#include <math.h>


LinearDCFormula::LinearDCFormula(void)
{
	m_StencilBuffer = NULL;
	m_Result = NULL;
}

LinearDCFormula::~LinearDCFormula(void)
{
	if (m_StencilBuffer != NULL) {
		delete m_StencilBuffer[0];
		delete m_StencilBuffer;
	}

	if (m_Result != NULL) {
		delete m_Result;
	}
}

BOOL LinearDCFormula::Processing( list<Node> &NodeList, list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
{
	//copy
	m_NodeList.clear();
	m_NodeList.resize(NodeList.size());
	copy(NodeList.begin(), NodeList.end(), m_NodeList.begin());

	//stencil convert (matrix)
	LinearStencilConvert();

// 	TRACE("\n");
// 	for (int iCount = 0; iCount < m_NodeList.size(); iCount++) {
// 		for (int jCount = 0; jCount < m_NodeList.size()+1; jCount++) {
// 			TRACE("%lf ", m_StencilBuffer[iCount][jCount]);
// 		}
// 		TRACE("\n");
// 	}

	//matrix -> result
	GaussianElimination();

	//result->Data
	IPCDataConvert(Datalist, Provlist);
	
	return TRUE;
}


void LinearDCFormula::IPCDataConvert(list<IPCData> &Datalist, list<CBaseElement*> &Provlist)
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

void LinearDCFormula::LinearStencilConvert()
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
				if ((*lowpos)->m_Flag == dcpower) {
					m_StencilBuffer[LineNumber][LineNumber] = 1.0;
					m_StencilBuffer[LineNumber][NodeSize] = GetElementProperty((*lowpos), CString(_T("Volatage")));
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


double LinearDCFormula::GetCombinedResistance(Node &CurrentNode)
{
	double CombinedResistance = 0;

	list<CBaseElement*>::iterator pos = CurrentNode.AllElement.begin();
	while (pos != CurrentNode.AllElement.end()) {

		double CurrentValue = GetElementProperty((*pos),  CString(_T("Resistance")));
		CombinedResistance += (1/CurrentValue);
		++pos;
	}
	return CombinedResistance;
}

double LinearDCFormula::GetInputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
{
	double CombinedResistance = 0;
	list<CBaseElement*>::iterator inputpos = CurrentNode.InputElement.begin();
	while (inputpos != CurrentNode.InputElement.end()) {
		
		int NodeNumber = IsDuplicateNode(CurrentNode, (*inputpos));
		if (NodeNumber != -1) {	
			StencilBuffer[NodeNumber] -= (1/GetElementProperty((*inputpos),  CString(_T("Resistance"))));
		}
		++inputpos;
	}

	return CombinedResistance;
}

double LinearDCFormula::GetOutputCombinedResistance(Node &CurrentNode, double *StencilBuffer)
{
	double CombinedResistance = 0;

	list<CBaseElement*>::iterator outputpos = CurrentNode.OutputElement.begin();
	while (outputpos != CurrentNode.OutputElement.end()) {
		
		int NodeNumber = IsDuplicateNode(CurrentNode, (*outputpos));
		if (NodeNumber != -1) {
			StencilBuffer[NodeNumber] -= (1/GetElementProperty((*outputpos),  CString(_T("Resistance"))));
		}
		++outputpos;
	}

	return CombinedResistance;
}

double LinearDCFormula::GetElementProperty(CBaseElement *pElement, CString &PropertyString)
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

int LinearDCFormula::IsDuplicateNode(Node &BaseNode, CBaseElement *pElement)
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

BOOL LinearDCFormula::GaussianElimination()
{
	double **a, *x, eps=1.e-10, sum;
	int i,j,k,n,np,ip;

	n = m_NodeList.size();
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
