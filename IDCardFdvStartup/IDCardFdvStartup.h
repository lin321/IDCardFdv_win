
// IDCardFdvStartup.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CIDCardFdvStartupApp: 
// �йش����ʵ�֣������ IDCardFdvStartup.cpp
//

class CIDCardFdvStartupApp : public CWinApp
{
public:
	CIDCardFdvStartupApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()

private:
	HANDLE m_hMutex;
};

extern CIDCardFdvStartupApp theApp;