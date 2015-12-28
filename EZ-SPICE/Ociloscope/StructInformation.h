#pragma once

#include <complex>

enum IPCDataType
{
	OciloscopeStart, VoltageData, OciloscopeEnd
};

enum CirCultCase
{
	linearDC, nonlinearDC, linearAC, nonlinearAC,
};

enum ProvColor
{
	darkred, darkgreen, darkyellow, darkblue,
};

enum Wavemodel
{
	square = 1, sign, triangular
};

#define DivideSector 360

//복소수 계산용
typedef std::complex<double> double_complex;	

typedef struct _IPCData
{
	int				NodeNumber;
	int				ProvNumber;
	BOOL			FirstFlag;
	BOOL			ComplexCheckFlag;
	double			NodeElectric;
	double_complex	ComplexNumber;
	double			GrapehValue[DivideSector];
}IPCData;

typedef struct _IPCHeader
{
	int		DataType;
	int		DataCount;
	int		SecMax;
	int		VoltMax; 
}IPCHeader;

typedef struct _IPCSectorHeader
{
	int		Circitcase;
	double	Frequency;
	int		DataColor;
}IPCSectorHeader;


typedef struct _PlotterData
{
	CString		PlotterName;

	double		XValue;
	double		YValue;
}PlotterData;