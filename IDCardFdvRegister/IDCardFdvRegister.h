
// IDCardFdvRegister.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CIDCardFdvRegisterApp: 
// �йش����ʵ�֣������ IDCardFdvRegister.cpp
//

class CIDCardFdvRegisterApp : public CWinApp
{
public:
	CIDCardFdvRegisterApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()

private:
	HANDLE m_hMutex;
};

extern CIDCardFdvRegisterApp theApp;