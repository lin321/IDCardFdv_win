// MTLibIDCardReader.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "IdCardReader.h"

CIdCardReader* MTLib_pIdCardReader = NULL;

string ExtractFilePath(const string& szFile)
{
	if (szFile == "")
		return "";

	size_t idx = szFile.find_last_of("\\:");

	if (-1 == idx)
		return "";
	return string(szFile.begin(), szFile.begin() + idx + 1);
}

int __stdcall LoadIDCardReader()
{
	if (NULL == MTLib_pIdCardReader)
		MTLib_pIdCardReader = new CIdCardReader();

	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	std::string strModulePath = ExtractFilePath(szPath);
	int error = MTLib_pIdCardReader->Init(strModulePath);
	if (error)
	{
		delete MTLib_pIdCardReader;
		MTLib_pIdCardReader = NULL;
		switch (error)
		{
		case -1:
			//"���֤��������װ��ʧ��!";
			break;
		case -2:
			//"���֤��������֧�����蹦��!";
			break;
		default:
			break;
		}
	}

	return error;
}

int __stdcall OpenIDCardReader()
{
	if (NULL == MTLib_pIdCardReader)
		return -1;

	int usbport = -1;
	for (int port = 1001; port <= 1016; ++port)
	{
		if (MTLib_pIdCardReader->InitComm(port) == 1) {
			usbport = port;
			break;
		}
	}

	if (usbport == -1) {
		// "���֤����������ʧ��!"
		MTLib_pIdCardReader->CloseComm();
		return -1;
	}

	return usbport;
}

int __stdcall Authenticate_Idcard()
{
	if (NULL == MTLib_pIdCardReader)
		return -1;

	int li_ret = MTLib_pIdCardReader->Authenticate();
	if (li_ret <= 0) {
		// "���֤��֤ʧ��!"
		return -1;
	}

	return 1;
}

int __stdcall Authenticate_Content(int Active)
{
	if (NULL == MTLib_pIdCardReader)
		return -1;

	if (1 != MTLib_pIdCardReader->Read_Content(Active)) {
		// "���֤��Ϣ��ȡʧ��!"
		return -1;
	}

	return 1;
}

int __stdcall IDCardReader_GetPeopleIDCode(char* pData, unsigned int iDataLen)
{
	if (NULL == MTLib_pIdCardReader)
		return -1;

	return MTLib_pIdCardReader->GetPeopleIDCode(pData, iDataLen);
}

int __stdcall IDCardReader_GetStartDate(char* pData, unsigned int iDataLen)
{
	if (NULL == MTLib_pIdCardReader)
		return -1;

	return MTLib_pIdCardReader->GetStartDate(pData, iDataLen);
}

int __stdcall IDCardReader_GetPhotoBMP(char* pData, unsigned int iDataLen)
{
	if (NULL == MTLib_pIdCardReader)
		return -1;

	return MTLib_pIdCardReader->GetPhotoBMP(pData, iDataLen);
}

int __stdcall CloseIDCardReader()
{
	if (NULL == MTLib_pIdCardReader)
		return -1;

	return MTLib_pIdCardReader->CloseComm();
}

int __stdcall UnloadIDCardReader()
{
	if (NULL != MTLib_pIdCardReader) {
		delete MTLib_pIdCardReader;
		MTLib_pIdCardReader = NULL;
	}

	return 0;
}
