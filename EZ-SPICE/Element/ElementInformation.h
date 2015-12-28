#pragma once

#include "StructInformation.h"

list<ElementGroup> g_ElementList;

void InitializeElement()
{
	//Voltage
	{
		//DC
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Voltage"));

		ElementInfo DCNewInfo;
		DCNewInfo.ElementGroupName = CString(_T("DC"));
		DCNewInfo.ElementName = CString(_T("DCVoltage"));
		DCNewInfo.ElementIntroduceString = CString(_T("2�� ���� ������ �������� �����̴�. �׷��� �� �ؼ��� �ð��� ���Ͽ� ��ȭ���� �ʴ� ������ ���Ѵ�."));
		DCNewInfo.ElementImageNumber = 0;
		DCNewInfo.ElementNumber = dcpower;
		NewGroup.ElementList.push_back(DCNewInfo);
		
		//ac
		ElementInfo ACNewInfo;
		ACNewInfo.ElementGroupName = CString(_T("AC"));
		ACNewInfo.ElementName = CString(_T("ACVoltage"));
		ACNewInfo.ElementIntroduceString = CString(_T("���� ��ȣ�� ��ũ/��ũ �� ������ ���� ũ��. �ٷ� �� ���� ������ �������� �����ʹ� �޶� �������� �ƴϴ�. ��10~-10V������ ������ ���� ��ȣ�� ���� ������ 20VAC�� �ȴ�."));
		ACNewInfo.ElementImageNumber = 1;
		ACNewInfo.ElementNumber = acpower;
		NewGroup.ElementList.push_back(ACNewInfo);
		g_ElementList.push_back(NewGroup);
	}

	//ACPower
	{

	}
	
	//Resistance
	{
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Resistance"));

		ElementInfo NewInfo;
		NewInfo.ElementGroupName = CString(_T("Resistance"));
		NewInfo.ElementName = CString(_T("Resistance"));
		NewInfo.ElementIntroduceString = CString(_T("������ �帣�� ���� ���� �ۿ�. ������ ��(��) 1��=1V�� ������ ���� ��, 1A�� ������ �帣�� ��ü�� ����. [��ü�� ���¿� ����] ���� R�� ���� l�� ����ϰ� �ܸ��� S�� �ݺ���Ѵ�."));
		NewInfo.ElementImageNumber = 2;
		NewInfo.ElementNumber = resistance;
		NewGroup.ElementList.push_back(NewInfo);
		g_ElementList.push_back(NewGroup);
	}

	//Ground
	{
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Ground"));

		ElementInfo NewInfo;
		NewInfo.ElementGroupName = CString(_T("Ground"));
		NewInfo.ElementName = CString(_T("Ground"));
		NewInfo.ElementIntroduceString = CString(_T("����ȸ�� �Ǵ� ��ġ�� ������ ���� ������ ���Ͽ� �ǵ������� �����ϴ� ��. ���� ��ſ� �׿� ��ġ�� ���� ��ü�� ����ϴ� ���� ������ ������ ������ ���� ���� �����ϱ� ���ؼ��̸� ��ü�� ������ Ȯ���ϰų� ����� Ȯ���� ������ �����ϴµ� �ִ�"));
		NewInfo.ElementImageNumber = 6;
		NewInfo.ElementNumber = ground;
		NewGroup.ElementList.push_back(NewInfo);
		g_ElementList.push_back(NewGroup);
	}

	//Intuctor
	{
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Inductor"));

		ElementInfo NewInfo;
		NewInfo.ElementGroupName = CString(_T("Inductor"));
		NewInfo.ElementName = CString(_T("Inductor"));
		NewInfo.ElementIntroduceString = CString(_T("����(coil)�̶� ���� ����� ����ϱ� ���ؼ� ������ ���� ���� �����̴�. �δ���(Inductor)�� ����(���� ����)�̶�� �Ѵ�. ���� ������ ���� �ڷ¼��� ����ϱ� ���� �������� ���� ���� �Ǽ��̶�� �Ѵ�."));
		NewInfo.ElementImageNumber = 3;
		NewInfo.ElementNumber = inductor;
		NewGroup.ElementList.push_back(NewInfo);
		g_ElementList.push_back(NewGroup);
	}

	//Capacity
	{
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Capacitor"));

		ElementInfo NewInfo;
		NewInfo.ElementGroupName = CString(_T("Capacitor"));
		NewInfo.ElementName = CString(_T("Capacitor"));
		NewInfo.ElementIntroduceString = CString(_T("�� ��ü ������ ������ �������� ������ ��ġ�̴�. ������� ���� �� ���� ��ü ������ �����Ǿ� �ְ�, �� ���̿� ����ü�� ����. ���⿡�� �� ���� ǥ��� ����ü�� ��� �κп� ���ϰ� ����ǰ�, �� ǥ�鿡 ���̴� ���Ϸ��� ũ��� ������ ��ȣ�� �ݴ��̴�. �������� ���Ϸ��� Q, �������� V��� �ϸ� �̶� ����뷮 C�� ������ ���� �����Ѵ�."));
		NewInfo.ElementImageNumber = 4;
		NewInfo.ElementNumber = capacity;
		NewGroup.ElementList.push_back(NewInfo);
		g_ElementList.push_back(NewGroup);
	}

	//Diode
	{
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Diode"));

		ElementInfo NewInfo;
		NewInfo.ElementGroupName = CString(_T("Diode"));
		NewInfo.ElementName = CString(_T("Diode"));
		NewInfo.ElementIntroduceString = CString(_T("��ü�� �⺻���� ���. ������ ���� ������ �ֳ��(���), �ٸ� ���� ������ ĳ�ҵ�(����)��� �θ���, ������ �ֳ�忡�� ĳ�ҵ� �������θ� �帥��. �̿� ���� ������ ���� ȸ�� � �̿�ȴ�. ��, �ֳ���ĳ�ҵ�� ������ �帣�� ��쿡�� ������ �־� �ֳ�塤ĳ�ҵ� ������ ������(����)�� ���� ���� �̻��� �Ǿ�߸� �ϴµ� �̸� �����̿��� ���С��̶�� �Ѵ�."));
		NewInfo.ElementImageNumber = 5;
		NewInfo.ElementNumber = diode;
		NewGroup.ElementList.push_back(NewInfo);
		g_ElementList.push_back(NewGroup);
	}

	//transistor
	{
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Transistor"));

		ElementInfo NewInfo;
		NewInfo.ElementGroupName = CString(_T("Transistor"));
		NewInfo.ElementName = CString(_T("Transistor"));
		NewInfo.ElementIntroduceString = CString(_T("P���� N���� �ݵ�ü�� 3�� ���ʷ� ���ӽ�Ų 3���� �ݵ�ü ���ڸ� �̸��� ������, �����ۿ��� �Ѵ�. 	PN������ ���̿��忡 P�� �ݵ�ü�� �ϳ� �� �߰��ϸ� PNP�� Ʈ�������Ͱ� �ȴ�. ���̽��� �ݷ��Ͱ��� ���������� �Ǿ� �ֱ� ������ �����Ϳ� ���̽����� ������ ������ ���� ������ ������ �帣�� �ʴ´�. �׷��� �����Ϳ� ���̽����� ���������� ������ ���ϸ� ���������� ������ ���̽��� �̵������� ���̽��� ���� ��� ������ �� ��κ��� ���̽��� ���� ���� �ݷ��Ϳ� �����Ѵ�. "));
		NewInfo.ElementImageNumber = 7;
		NewInfo.ElementNumber = transistor;
		NewGroup.ElementList.push_back(NewInfo);
		g_ElementList.push_back(NewGroup);
	}

	//Amplifier
	{
		//Inverting
		ElementGroup NewGroup;
		NewGroup.GroupName = CString(_T("Amplifier"));

		ElementInfo NewInverting;
		NewInverting.ElementGroupName = CString(_T("Amplifier"));
		NewInverting.ElementName = CString(_T("InverAmp"));
		NewInverting.ElementIntroduceString = CString(_T("���� �������� �⺻���� ȸ�α����� �ϳ��μ�, ������ �׸��� ���� ȸ���̴�. ������ ��ü�� ����� ���Ǵ����� ���� Rin, Rout�� �ϸ� �׸��� �������� ����� ���Ǵ����� ���� R1 �� Rout/(1��A��)�� �ȴ�."));
		NewInverting.ElementImageNumber = 8;
		NewInverting.ElementNumber = InvertingAmplifier;
		NewGroup.ElementList.push_back(NewInverting);

		//NonInverting
		ElementInfo NewNonInverting;
		NewNonInverting.ElementGroupName = CString(_T("Amplifier"));
		NewNonInverting.ElementName = CString(_T("NInverAmp"));
		NewNonInverting.ElementIntroduceString = CString(_T("���� �������� �⺻���� ȸ�α����� �ϳ���, �Ʒ� �׸������� ���� ���ӵ� ��. ������ ��ü�� ����� ���Ǵ����� ���� Rin, R0�� �ϸ�, ������� ����� ���Ǵ����� ���� Rin(1+A��) �� R0(1+A��)�� �ȴ�."));
		NewNonInverting.ElementImageNumber = 9;
		NewNonInverting.ElementNumber = NonInvertingAmplifier;
		NewGroup.ElementList.push_back(NewNonInverting);
		
		g_ElementList.push_back(NewGroup);
	}
}

