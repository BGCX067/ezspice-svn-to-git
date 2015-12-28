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
		DCNewInfo.ElementIntroduceString = CString(_T("2점 간의 전위의 전위차가 전압이다. 그러나 그 극성이 시간에 대하여 변화하지 않는 전압을 말한다."));
		DCNewInfo.ElementImageNumber = 0;
		DCNewInfo.ElementNumber = dcpower;
		NewGroup.ElementList.push_back(DCNewInfo);
		
		//ac
		ElementInfo ACNewInfo;
		ACNewInfo.ElementGroupName = CString(_T("AC"));
		ACNewInfo.ElementName = CString(_T("ACVoltage"));
		ACNewInfo.ElementIntroduceString = CString(_T("전기 신호의 피크/피크 간 전압의 진폭 크기. 바로 그 성질 때문에 교류에는 직류와는 달라서 정전압이 아니다. ＋10~-10V까지의 진폭을 갖는 신호의 교류 전압은 20VAC가 된다."));
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
		NewInfo.ElementIntroduceString = CString(_T("전류가 흐르는 것을 막는 작용. 단위는 옴(Ω) 1Ω=1V의 전압을 가한 때, 1A의 전류가 흐르는 도체의 저항. [도체의 형태와 저항] 저항 R은 길이 l에 비례하고 단면적 S에 반비례한다."));
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
		NewInfo.ElementIntroduceString = CString(_T("전기회로 또는 장치의 적당한 곳을 대지에 대하여 의도적으로 접속하는 것. 대지 대신에 그에 대치될 만한 도체를 사용하는 수도 있지만 목적은 전압의 기준 점을 설정하기 위해서이며 인체의 안전을 확보하거나 기기의 확실한 동작을 보증하는데 있다"));
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
		NewInfo.ElementIntroduceString = CString(_T("코일(coil)이란 유도 계수를 사용하기 위해서 전선을 감은 수동 소자이다. 인덕터(Inductor)나 선륜(낡은 말투)이라고도 한다. 전자 유도에 의한 자력선을 사용하기 위해 전선으로 감은 것을 권선이라고 한다."));
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
		NewInfo.ElementIntroduceString = CString(_T("두 도체 사이의 공간에 전기장을 모으는 장치이다. 축전기는 보통 두 개의 도체 판으로 구성되어 있고, 그 사이에 절연체가 들어간다. 여기에서 각 판의 표면과 절연체의 경계 부분에 전하가 비축되고, 양 표면에 모이는 전하량의 크기는 같지만 부호는 반대이다. 축전기의 전하량을 Q, 전위차를 V라고 하면 이때 전기용량 C를 다음과 같이 정의한다."));
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
		NewInfo.ElementIntroduceString = CString(_T("도체의 기본적인 요소. 단자의 한쪽 방향을 애노드(양극), 다른 한쪽 방향을 캐소드(음극)라고 부르며, 전류는 애노드에서 캐소드 방향으로만 흐른다. 이와 같은 성질은 정류 회로 등에 이용된다. 또, 애노드→캐소드로 전류가 흐르는 경우에도 조건이 있어 애노드·캐소드 사이의 전위차(전압)가 일정 수준 이상이 되어야만 하는데 이를 ’다이오드 전압’이라고 한다."));
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
		NewInfo.ElementIntroduceString = CString(_T("P형과 N형의 반도체를 3개 차례로 접속시킨 3단자 반도체 소자를 이르는 것으로, 증폭작용을 한다. 	PN접합의 다이오드에 P형 반도체를 하나 더 추가하면 PNP형 트랜지스터가 된다. 베이스와 콜렉터간은 역방향으로 되어 있기 때문에 에미터와 베이스간에 전압이 가해져 있지 않으면 전류는 흐르지 않는다. 그러나 에미터와 베이스간에 순방향으로 전압을 가하면 에미터쪽의 정공은 베이스로 이동하지만 베이스의 폭이 얇기 때문에 그 대부분이 베이스를 빠져 나가 콜렉터에 돌입한다. "));
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
		NewInverting.ElementIntroduceString = CString(_T("연산 증폭기의 기본적인 회로구조의 하나로서, 다음의 그림과 같은 회로이다. 증폭기 본체의 입출력 임피던스를 각각 Rin, Rout로 하면 그림의 증폭기의 입출력 임피던스는 각각 R1 및 Rout/(1＋Aβ)로 된다."));
		NewInverting.ElementImageNumber = 8;
		NewInverting.ElementNumber = InvertingAmplifier;
		NewGroup.ElementList.push_back(NewInverting);

		//NonInverting
		ElementInfo NewNonInverting;
		NewNonInverting.ElementGroupName = CString(_T("Amplifier"));
		NewNonInverting.ElementName = CString(_T("NInverAmp"));
		NewNonInverting.ElementIntroduceString = CString(_T("연산 증폭기의 기본적인 회로구조의 하나로, 아래 그림에서와 같이 접속된 것. 증폭기 본체의 입출력 임피던스를 각각 Rin, R0로 하면, 폐루프의 입출력 임피던스는 각각 Rin(1+Aβ) 및 R0(1+Aβ)로 된다."));
		NewNonInverting.ElementImageNumber = 9;
		NewNonInverting.ElementNumber = NonInvertingAmplifier;
		NewGroup.ElementList.push_back(NewNonInverting);
		
		g_ElementList.push_back(NewGroup);
	}
}

