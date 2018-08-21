#include "stdafx.h"
#include "IdCardReader.h"
#include <string>
using namespace std;

CIdCardReader::CIdCardReader()
{

}


CIdCardReader::~CIdCardReader()
{
}


int CIdCardReader::Init(std::string& strModulePath)
{

	string szTmp=strModulePath+"termb.dll";
	HMODULE hMod = LoadLibrary(szTmp.c_str());
	if (hMod == NULL) {
		//::MessageBox(NULL, ("身份证读卡器库装载失败!"), NULL, MB_OK);
		return -1;
	}

	InitComm = (FInitComm)GetProcAddress(hMod, "InitComm");
	CloseComm = (FCloseComm)GetProcAddress(hMod, "CloseComm");
	Authenticate = (FAuthenticate)GetProcAddress(hMod, "Authenticate");
	//ad_Content = (FRead_Content)GetProcAddress(hMod, "CVR_Read_Content");
	Read_Content = (FRead_Content)GetProcAddress(hMod, "Read_Content");

	GetPeopleName = (FGetPeopleName)GetProcAddress(hMod, "GetPeopleName");
	GetPeopleSex = (FGetPeopleSex)GetProcAddress(hMod, "GetPeopleSex");
	GetPeopleNation = (FGetPeopleNation)GetProcAddress(hMod, "GetPeopleNation");
	GetPeopleBirthday = (FGetPeopleBirthday)GetProcAddress(hMod, "GetPeopleBirthday");
	GetPeopleAddress = (FGetPeopleAddress)GetProcAddress(hMod, "GetPeopleAddress");
	GetPeopleIDCode = (FGetPeopleIDCode)GetProcAddress(hMod, "GetPeopleIDCode");
	GetStartDate = (FGetStartDate)GetProcAddress(hMod, "GetStartDate");
	GetEndDate = (FGetEndDate)GetProcAddress(hMod, "GetEndDate");
	GetDepartment = (FGetEndDate)GetProcAddress(hMod, "GetDepartment");
	GetPhotoBMP = (FGetPhotoBMP)GetProcAddress(hMod, "GetPhotoBMP");

	if (InitComm == NULL || CloseComm == NULL ||
		Authenticate == NULL || Read_Content == NULL ||
		GetPeopleName == NULL || GetPeopleSex == NULL ||
		GetPeopleNation == NULL || GetPeopleBirthday == NULL ||
		GetPeopleAddress == NULL || GetPeopleIDCode == NULL ||
		GetStartDate == NULL || GetEndDate == NULL ||
		GetPhotoBMP == NULL || GetDepartment == NULL
		) {// || GetPeopleName == NULL) {
		//::MessageBox(NULL, ("身份证读卡器取函数失败!"), NULL, MB_OK);
		return -2;
	}
	return 0;
}
