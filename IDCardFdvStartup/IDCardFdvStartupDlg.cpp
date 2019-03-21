
// IDCardFdvStartupDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IDCardFdvStartup.h"
#include "IDCardFdvStartupDlg.h"
#include "afxdialogex.h"
#include "utility_funcs.h"
#include <string>
#include <sstream>

using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPGRADE_CLOSE			WM_USER+1001

UINT MainWorkThread(LPVOID lpParam);



// CIDCardFdvStartupDlg �Ի���

CIDCardFdvStartupDlg::CIDCardFdvStartupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IDCARDFDVSTARTUP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_thWork = NULL;
}

void CIDCardFdvStartupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIDCardFdvStartupDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CIDCardFdvStartupDlg ��Ϣ�������

BOOL CIDCardFdvStartupDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��


	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_pStaticMsg = (CStatic*)GetDlgItem(IDC_STATIC_MSG);

	if (m_thWork == NULL) {
		m_thWork = AfxBeginThread(MainWorkThread, this);
		if (NULL == m_thWork)
		{
			TRACE("�����µ��̳߳���\n");
			PostMessage(WM_CLOSE);
			return TRUE;
		}
	}
	

	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	m_strModulePath = ExtractFilePath(szPath);
	



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIDCardFdvStartupDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CIDCardFdvStartupDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HBRUSH CIDCardFdvStartupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_STATIC_MSG)
	{
		CFont font;
		font.CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
			"΢���ź�");
		pDC->SelectObject(&font);
		pDC->SetTextColor(RGB(74, 144, 226));
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void CIDCardFdvStartupDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (nIDEvent)
	{
	case 0:
		string msg = "�����°汾�����ڸ���";
		static int msgstep = 1;
		if (1 == msgstep)
			msg += "�����";
		else if (2 == msgstep)
			msg += "������";
		else if (3 == msgstep)
			msg += "������";
		else
			msg += "����";

		msgstep = (msgstep + 1) % 4;
		m_pStaticMsg->SetWindowText(msg.c_str());
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CIDCardFdvStartupDlg::errorExit(std::string msg)
{
	KillTimer(0);
	m_pStaticMsg->SetWindowText(msg.c_str());
	Sleep(2000);
	SendMessage(WM_CLOSE, 0, 0);
}


UINT MainWorkThread(LPVOID lpParam)
{
	CIDCardFdvStartupDlg* pDlg = (CIDCardFdvStartupDlg*)lpParam;

	// ���������Ѵ򿪵�IDCardFdv
	HWND hWnd = ::FindWindow("CIDCardFdvDlg", "IDCardFdv");
	if (hWnd) {
		::SendMessage(hWnd, WM_CLOSE, 0, 0);
		int timeout = 2000;	
		do {
			Sleep(100);
			timeout -= 100;
			hWnd = ::FindWindow("CIDCardFdvDlg", "IDCardFdv");
		} while (hWnd && timeout > 0);

		if (hWnd) {
			pDlg->errorExit("����ʧ�ܣ���ر����������!");
			return 0;
		}
	}	

	// 
	/*
	// lock test
	string lf = m_strModulePath + "Upgrade\\version.txt";
	HANDLE hFile;
	hFile = CreateFile(lf.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD fsize = GetFileSize(hFile, NULL);
	BOOL lret = LockFile(hFile, 0, 0, fsize, 0);
	*/
	ifstream verFileUpgrade(pDlg->m_strModulePath + "Upgrade\\version.txt");
	ifstream verFileRunnable(pDlg->m_strModulePath + "Runnable\\version.txt");
	if (verFileUpgrade && verFileRunnable) {
		// ����°汾
		string verUpgrade;
		std::getline(verFileUpgrade, verUpgrade);

		string verRunnable;
		std::getline(verFileRunnable, verRunnable);

		// verStr.empty(): ��ȡ�����ļ�����
		if (!verRunnable.empty() && (CompareVersion(verRunnable, verUpgrade)<0)){
			// �����°汾
			pDlg->m_pStaticMsg->SetWindowText("�����°汾�����ڸ��¡���");
			pDlg->SetTimer(0, 200, NULL);
			verFileRunnable.close();
			verFileUpgrade.close();

			// �ر�idfdvcomplete.exe
			string spt = pDlg->m_strModulePath + "Runnable\\idfdvcomplete_stop.bat";
			WinExec(spt.c_str(), SW_HIDE);

			// �رտ��ܴ򿪵�IDCardFdvRegister
			HWND hWnd = ::FindWindow(NULL, "IDCardFdvRegister");
			if (hWnd) {
				::SendMessage(hWnd, WM_CLOSE, 0, 0);
				int timeout = 1000;
				do {
					Sleep(100);
					timeout -= 100;
					hWnd = ::FindWindow(NULL, "IDCardFdvRegister");
				} while (hWnd && timeout > 0);
			}

			// �ȴ�idfdvcomplete.exe�ر�
			int timeout = 1000;
			while (GetProcessPID("idfdvcomplete.exe") != 0 && timeout > 0) {
				Sleep(100);
				timeout -= 100;
			}

			// �����°汾�����ļ�
			string fromFolder = pDlg->m_strModulePath + "Upgrade\\*.*";
			string toFolder = pDlg->m_strModulePath + "Runnable";
			BOOL copyRet = CopyFolder(fromFolder.c_str(), toFolder.c_str());

			if (!copyRet) {	// ����ʧ��
				// ��������idfdvcomplete.exe
				string spt = pDlg->m_strModulePath + "Runnable\\idfdvcomplete_run.bat";
				WinExec(spt.c_str(), SW_HIDE);

				// �ع��汾��
				std::ofstream verFileRunnable2(pDlg->m_strModulePath + "Runnable\\version.txt");
				if (verFileRunnable2) {
					verFileRunnable2 << verRunnable;	
					verFileRunnable2.close();
				}

				pDlg->errorExit("����ʧ�ܣ����Ժ�����!");
				return 0;
			}
			else { // �����ɹ�
				// ���¼��Upgrade�İ汾�ţ����⿽��������ִ���˸��¡�
				bool err = false;
				ifstream verFileUpgrade2(pDlg->m_strModulePath + "Upgrade\\version.txt");
				if (verFileUpgrade2) {
					string verUpgrade2;
					std::getline(verFileUpgrade2, verUpgrade2);
					if (verUpgrade2.empty()) {
						err = true;	// ������,������upgrade_idfdv��ִ�п�������
					}
					else if (CompareVersion(verUpgrade, verUpgrade2) != 0) {
						err = true;	// �汾�Ų�һ�£����������������µĸ���
					}
					verFileUpgrade2.close();
				}
				else {
					err = true;	// ������
				}
			
				if (err) {
					pDlg->errorExit("����ʧ�ܣ����Ժ�����!");
					return 0;
				}

				// ����idfdvcomplete.exe
				string spt = pDlg->m_strModulePath + "Runnable\\idfdvcomplete_run.bat";
				WinExec(spt.c_str(), SW_HIDE);

				// ���´���upgrade_idfdv.exe�ƻ�����
				spt = pDlg->m_strModulePath + "Runnable\\upgrade_idfdv_add_task.bat";
				WinExec(spt.c_str(), SW_HIDE);

				pDlg->KillTimer(0);
				pDlg->m_pStaticMsg->SetWindowText("������ɣ���������...");
			} // if (!copyRet) 
		}
		
	}

	verFileRunnable.close();
	verFileUpgrade.close();

	//==========================================
	// ����IDCardFdv.exe
	STARTUPINFO si;
	::memset(&si, 0, sizeof(STARTUPINFO));//��ʼ��si���ڴ���е�ֵ�����memset������
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION pi;//�ر��������ý���
	string exePath = pDlg->m_strModulePath + "Runnable\\IDCardFdv.exe";
	if(!CreateProcess(exePath.c_str(),
		NULL,NULL,NULL,FALSE,0,NULL,NULL,
		&si,&pi)
		){
		pDlg->errorExit("����ʧ�ܣ�������!");
		return 0;
	}

	// ���ش��ڣ������̨����
	Sleep(3 * 1000);
	pDlg->ShowWindow(SW_HIDE);

	// ��upgradestart.txtȷ���Ƿ���Ҫ�����Լ�
	string upgradevalue="0";
	std::ifstream usFile(pDlg->m_strModulePath + "Runnable\\upgradestart.txt");
	std::string line;
	while (std::getline(usFile, line)) {
		std::istringstream is_line(line);
		std::string key;
		if (std::getline(is_line, key, '=')) {
			std::string value;
			if (std::getline(is_line, value)) {
				if (key == "upgradeStart")
					upgradevalue = value;
			}
		}
	}
	usFile.close();


	bool upgrademyself = false;
	if (::stoi(upgradevalue) == 1) { 
		// 1������ִ�����������Լ�
		upgrademyself = true;

		// д�ر�����δ���
		std::ofstream outFile(pDlg->m_strModulePath + "Runnable\\upgradestart.txt");
		outFile << "upgradeStart=" << "0" << endl;
		outFile.close();
	}

	// ȡ�����ķ��������
	::srand((unsigned)time(NULL));
	int rebootM = (::rand() % 58); // 0-57
	while(1){
		Sleep(10*1000);
		//Sleep(1 * 1000);

		// �û��������IDCardFdv.exe�Ƿ�������
		HANDLE hMutex = CreateMutex(NULL, FALSE, "IDCardFdv");
		// ���������
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			CloseHandle(hMutex);

			// ��������
			CTime time = CTime::GetCurrentTime();
			int hour = time.GetHour();
			if (hour == 5 || upgrademyself) {
				int minute = time.GetMinute();
				if (minute >= rebootM || upgrademyself) {
					// ����
					// ִ����������ļƻ�����bat
					string spt = pDlg->m_strModulePath + "Runnable\\AddRestartTask.bat";
					if (WinExec(spt.c_str(), SW_HIDE) > 31) {
						// �ر�IDCardFdv.exe
						HWND hWnd = ::FindWindow(NULL, "IDCardFdv");
						if (hWnd) {
							::SendMessage(hWnd, WM_UPGRADE_CLOSE, 0, 0);
						}

						// �˳�������
						pDlg->SendMessage(WM_CLOSE, 0, 0);
						return 0;
					}
				}
			}	// if (hour == **)
		}
		else {
			// IDCardFdv.exe�Ѿ��˳�
			
			//��ʹ�õľ����ùص�
			CloseHandle(hMutex);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			pDlg->SendMessage(WM_CLOSE, 0, 0);
			return 0;
		}
		
	}
	
	
	//��ʹ�õľ����ùص�
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 0;
}


