//Element DLL Header File

#pragma once

class CBaseElement;
typedef struct _ElementInformation ElementInformation;

#pragma comment(lib, "Element")
__declspec(dllimport) BOOL GetElementInfoListToDll(list<typedef struct _ElementGroup> &CopyList);
__declspec(dllimport) BOOL ElementToInformation(CBaseElement* pElement, ElementInformation &NewInfo);
__declspec(dllimport) CBaseElement* InformationToElement(ElementInformation &CurrentInfo);
__declspec(dllimport) CBaseElement* CreateElementToDll(int ElementNumber);


