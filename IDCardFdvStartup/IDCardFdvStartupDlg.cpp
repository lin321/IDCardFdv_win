
// IDCardFdvStartupDlg.cpp : 实现文件
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



// CIDCardFdvStartupDlg 对话框

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


// CIDCardFdvStartupDlg 消息处理程序

BOOL CIDCardFdvStartupDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	// TODO: 在此添加额外的初始化代码
	m_pStaticMsg = (CStatic*)GetDlgItem(IDC_STATIC_MSG);

	if (m_thWork == NULL) {
		m_thWork = AfxBeginThread(MainWorkThread, this);
		if (NULL == m_thWork)
		{
			TRACE("创建新的线程出错！\n");
			PostMessage(WM_CLOSE);
			return TRUE;
		}
	}
	

	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	m_strModulePath = ExtractFilePath(szPath);
	



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIDCardFdvStartupDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CIDCardFdvStartupDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HBRUSH CIDCardFdvStartupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_STATIC_MSG)
	{
		CFont font;
		font.CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
			"微软雅黑");
		pDC->SelectObject(&font);
		pDC->SetTextColor(RGB(74, 144, 226));
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CIDCardFdvStartupDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 0:
		string msg = "发现新版本，正在更新";
		static int msgstep = 1;
		if (1 == msgstep)
			msg += "◆◇◇";
		else if (2 == msgstep)
			msg += "◆◆◇";
		else if (3 == msgstep)
			msg += "◆◆◆";
		else
			msg += "◇◇◇";

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

	// 结束可能已打开的IDCardFdv
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
			pDlg->errorExit("启动失败，请关闭软件后重试!");
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
		// 检查新版本
		string verUpgrade;
		std::getline(verFileUpgrade, verUpgrade);

		string verRunnable;
		std::getline(verFileRunnable, verRunnable);

		// verStr.empty(): 读取出错，文件被锁
		if (!verRunnable.empty() && (CompareVersion(verRunnable, verUpgrade)<0)){
			// 发现新版本
			pDlg->m_pStaticMsg->SetWindowText("发现新版本，正在更新◇◇◇");
			pDlg->SetTimer(0, 200, NULL);
			verFileRunnable.close();
			verFileUpgrade.close();

			// 关闭idfdvcomplete.exe
			string spt = pDlg->m_strModulePath + "Runnable\\idfdvcomplete_stop.bat";
			WinExec(spt.c_str(), SW_HIDE);

			// 关闭可能打开的IDCardFdvRegister
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

			// 等待idfdvcomplete.exe关闭
			int timeout = 1000;
			while (GetProcessPID("idfdvcomplete.exe") != 0 && timeout > 0) {
				Sleep(100);
				timeout -= 100;
			}

			// 拷贝新版本所有文件
			string fromFolder = pDlg->m_strModulePath + "Upgrade\\*.*";
			string toFolder = pDlg->m_strModulePath + "Runnable";
			BOOL copyRet = CopyFolder(fromFolder.c_str(), toFolder.c_str());

			if (!copyRet) {	// 拷贝失败
				// 尝试重启idfdvcomplete.exe
				string spt = pDlg->m_strModulePath + "Runnable\\idfdvcomplete_run.bat";
				WinExec(spt.c_str(), SW_HIDE);

				// 回滚版本号
				std::ofstream verFileRunnable2(pDlg->m_strModulePath + "Runnable\\version.txt");
				if (verFileRunnable2) {
					verFileRunnable2 << verRunnable;	
					verFileRunnable2.close();
				}

				pDlg->errorExit("更新失败，请稍候重试!");
				return 0;
			}
			else { // 拷贝成功
				// 重新检查Upgrade的版本号，避免拷贝过程中执行了更新。
				bool err = false;
				ifstream verFileUpgrade2(pDlg->m_strModulePath + "Upgrade\\version.txt");
				if (verFileUpgrade2) {
					string verUpgrade2;
					std::getline(verFileUpgrade2, verUpgrade2);
					if (verUpgrade2.empty()) {
						err = true;	// 被锁定,可能是upgrade_idfdv在执行拷贝操作
					}
					else if (CompareVersion(verUpgrade, verUpgrade2) != 0) {
						err = true;	// 版本号不一致，拷贝过程中有了新的更新
					}
					verFileUpgrade2.close();
				}
				else {
					err = true;	// 被锁定
				}
			
				if (err) {
					pDlg->errorExit("更新失败，请稍候重试!");
					return 0;
				}

				// 重启idfdvcomplete.exe
				string spt = pDlg->m_strModulePath + "Runnable\\idfdvcomplete_run.bat";
				WinExec(spt.c_str(), SW_HIDE);

				// 重新处理upgrade_idfdv.exe计划任务
				spt = pDlg->m_strModulePath + "Runnable\\upgrade_idfdv_add_task.bat";
				WinExec(spt.c_str(), SW_HIDE);

				pDlg->KillTimer(0);
				pDlg->m_pStaticMsg->SetWindowText("更新完成，正在启动...");
			} // if (!copyRet) 
		}
		
	}

	verFileRunnable.close();
	verFileUpgrade.close();

	//==========================================
	// 启动IDCardFdv.exe
	STARTUPINFO si;
	::memset(&si, 0, sizeof(STARTUPINFO));//初始化si在内存块中的值（详见memset函数）
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION pi;//必备参数设置结束
	string exePath = pDlg->m_strModulePath + "Runnable\\IDCardFdv.exe";
	if(!CreateProcess(exePath.c_str(),
		NULL,NULL,NULL,FALSE,0,NULL,NULL,
		&si,&pi)
		){
		pDlg->errorExit("启动失败，请重试!");
		return 0;
	}

	// 隐藏窗口，进入后台运行
	Sleep(3 * 1000);
	pDlg->ShowWindow(SW_HIDE);

	// 读upgradestart.txt确认是否需要更新自己
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
		// 1则马上执行重启更新自己
		upgrademyself = true;

		// 写回避免二次触发
		std::ofstream outFile(pDlg->m_strModulePath + "Runnable\\upgradestart.txt");
		outFile << "upgradeStart=" << "0" << endl;
		outFile.close();
	}

	// 取重启的分钟随机数
	::srand((unsigned)time(NULL));
	int rebootM = (::rand() % 58); // 0-57
	while(1){
		Sleep(10*1000);
		//Sleep(1 * 1000);

		// 用互斥量检测IDCardFdv.exe是否还在运行
		HANDLE hMutex = CreateMutex(NULL, FALSE, "IDCardFdv");
		// 检查错误代码
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			CloseHandle(hMutex);

			// 定点重启
			CTime time = CTime::GetCurrentTime();
			int hour = time.GetHour();
			if (hour == 5 || upgrademyself) {
				int minute = time.GetMinute();
				if (minute >= rebootM || upgrademyself) {
					// 重启
					// 执行添加重启的计划任务bat
					string spt = pDlg->m_strModulePath + "Runnable\\AddRestartTask.bat";
					if (WinExec(spt.c_str(), SW_HIDE) > 31) {
						// 关闭IDCardFdv.exe
						HWND hWnd = ::FindWindow(NULL, "IDCardFdv");
						if (hWnd) {
							::SendMessage(hWnd, WM_UPGRADE_CLOSE, 0, 0);
						}

						// 退出本程序
						pDlg->SendMessage(WM_CLOSE, 0, 0);
						return 0;
					}
				}
			}	// if (hour == **)
		}
		else {
			// IDCardFdv.exe已经退出
			
			//不使用的句柄最好关掉
			CloseHandle(hMutex);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			pDlg->SendMessage(WM_CLOSE, 0, 0);
			return 0;
		}
		
	}
	
	
	//不使用的句柄最好关掉
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 0;
}


