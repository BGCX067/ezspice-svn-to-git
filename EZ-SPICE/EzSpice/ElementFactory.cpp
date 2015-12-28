#include "StdAfx.h"
#include "ElementFactory.h"
#include "BaseElement.h"
#include "DLLheader.h"

CElementFactory::CElementFactory(void)
{
	//모든 소자 정보 생성 및 저장

}

CElementFactory::~CElementFactory(void)
{
}

CBaseElement* CElementFactory::CreateElement( int elementKind )
{
	return ::CreateElementToDll(elementKind);
}

ElementInformation CElementFactory::GetElementToInformation(CBaseElement *pElement)
{
	ElementInformation NewInformation;
	::ElementToInformation(pElement, NewInformation);

	return NewInformation;
}

CBaseElement* CElementFactory::GetInformationToElement( ElementInformation &CurrentInformation )
{
	return ::InformationToElement(CurrentInformation);
}







