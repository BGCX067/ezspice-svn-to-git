#pragma once

//소자 기본 정보 저장 및 소자 생성 대행, 클래스 팩토리 패턴사용
//이러한 공유 정보들은 MainFrame에 생성한다. 또한 유저소자창 정보로 로드해서 설정

class CBaseElement;
typedef struct  _ElementInformation ElementInformation;

class CElementFactory		
{
public:
	CElementFactory(void);
	~CElementFactory(void);

public:
	CBaseElement* CreateElement(int elementKind);									//객체 생성

	ElementInformation GetElementToInformation(CBaseElement *pElement);				//객체 -> 저장 객체
	CBaseElement* GetInformationToElement(ElementInformation &CurrentInformation);	//저장 객체 -> 객체

};
