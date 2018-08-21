#pragma once
#include <string>

typedef int(__stdcall *FInitComm) (int iPort);
typedef int(__stdcall *FCloseComm) ();
typedef int(__stdcall *FAuthenticate) ();
typedef int(__stdcall *FRead_Content) (int iActive);
typedef int(__stdcall *FGetPeopleName) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetPeopleSex) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetPeopleNation) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetPeopleBirthday) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetPeopleAddress) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetPeopleIDCode) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetStartDate) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetEndDate) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetDepartment) (char* pData, UINT iDataLen);
//typedef int (__stdcall *FGetEndDate ) (char* pData, UINT iDataLen);
typedef int(__stdcall *FGetPhotoBMP) (char* pData, UINT iDataLen);

using namespace std;

class CIdCardReader
{
public:
	CIdCardReader();
	~CIdCardReader();
	FInitComm			InitComm;
	FCloseComm			CloseComm;
	FAuthenticate		Authenticate;
	FRead_Content		Read_Content;
	FGetPeopleName      GetPeopleName;
	FGetPeopleSex       GetPeopleSex;
	FGetPeopleNation    GetPeopleNation;
	FGetPeopleBirthday  GetPeopleBirthday;
	FGetPeopleAddress   GetPeopleAddress;
	FGetPeopleIDCode    GetPeopleIDCode;
	FGetStartDate       GetStartDate;
	FGetEndDate         GetEndDate;
	FGetDepartment      GetDepartment;
	FGetPhotoBMP		GetPhotoBMP;

	int Init(std::string& strModulePath);
};

