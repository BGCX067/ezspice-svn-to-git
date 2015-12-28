#pragma once

//���� �⺻ ���� ���� �� ���� ���� ����, Ŭ���� ���丮 ���ϻ��
//�̷��� ���� �������� MainFrame�� �����Ѵ�. ���� ��������â ������ �ε��ؼ� ����

class CBaseElement;
typedef struct  _ElementInformation ElementInformation;

class CElementFactory		
{
public:
	CElementFactory(void);
	~CElementFactory(void);

public:
	CBaseElement* CreateElement(int elementKind);									//��ü ����

	ElementInformation GetElementToInformation(CBaseElement *pElement);				//��ü -> ���� ��ü
	CBaseElement* GetInformationToElement(ElementInformation &CurrentInformation);	//���� ��ü -> ��ü

};
