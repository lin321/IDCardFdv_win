
// IDCardFdvRegisterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdvRegister.h"
#include "IDCardFdvRegisterDlg.h"
#include "afxdialogex.h"

#include "LocalMac.h"
#include "MTLibNetwork.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

static void __stdcall RegisterCB(int err_no, std::string err_msg, std::string RegisteredNo, unsigned long userdata)
{
	if (-1 == err_no) {
		AfxMessageBox(_T("network error!")); 
		return;
	}

	if (0 != err_no) {
		if (404 == err_no) {
			AfxMessageBox(_T("该序列号已被注册!"));
		}
		else if (403 == err_no) {
			AfxMessageBox(_T("无效的序列号！"));
		}
		else
			AfxMessageBox(_T("注册失败！"));
	}
	else {
		AfxMessageBox(_T("注册成功!"));

		// save
		CIDCardFdvRegisterDlg* pDlg = (CIDCardFdvRegisterDlg*)userdata;
		pDlg->m_cfgRegisteredNo = RegisteredNo;
		pDlg->saveConfig();
	}
}

static void __stdcall TestUrlCB(int err_no, unsigned long userdata)
{
	if(0 == err_no)
		AfxMessageBox(_T("URL 可用!"));
	else
		AfxMessageBox(_T("URL 不可用!"));
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIDCardFdvRegisterDlg 对话框



CIDCardFdvRegisterDlg::CIDCardFdvRegisterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IDCARDFDVREGISTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIDCardFdvRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIDCardFdvRegisterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CIDCardFdvRegisterDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_REG, &CIDCardFdvRegisterDlg::OnBnClickedBtnReg)
END_MESSAGE_MAP()

string ExtractFilePath(const string& szFile)
{
	if (szFile == "")
		return "";

	size_t idx = szFile.find_last_of("\\:");

	if (-1 == idx)
		return "";
	return string(szFile.begin(), szFile.begin() + idx + 1);
}

// CIDCardFdvRegisterDlg 消息处理程序

BOOL CIDCardFdvRegisterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	m_strModulePath = ExtractFilePath(szPath);

	// config
	m_cfgAppId = "10022245";
	m_cfgApiKey = "MGRhNjEyYWExOTdhYzYxNTkx";
	m_cfgSecretKey = "NzQyNTg0YmZmNDg3OWFjMTU1MDQ2YzIw";
	m_cfgUrl = "http://192.168.1.201:8004/idcardfdv";
	std::ifstream confFile(m_strModulePath + "config.txt");
	std::string line;
	while (std::getline(confFile, line))
	{
		std::istringstream is_line(line);

		std::string key;
		if (std::getline(is_line, key, '='))
		{
			std::string value;
			if (std::getline(is_line, value)) {
				if (key == "appId")
					m_cfgAppId = value;
				if (key == "apiKey")
					m_cfgApiKey = value;
				if (key == "secretKey")
					m_cfgSecretKey = value;
				if (key == "url")
					m_cfgUrl = value;
			}
		}
	}
	confFile.close();
	m_cfgRegisteredNo = "";

	int idx = m_cfgUrl.find_last_of('/');
	std::string regurl = m_cfgUrl.substr(0, idx);
	regurl = regurl + "/registerproduct";
	GetDlgItem(IDC_URL_EDIT)->SetWindowText(regurl.c_str());

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CIDCardFdvRegisterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIDCardFdvRegisterDlg::OnPaint()
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
HCURSOR CIDCardFdvRegisterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIDCardFdvRegisterDlg::OnBnClickedBtnTest()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	GetDlgItem(IDC_URL_EDIT)->GetWindowText(str);
	std::string url = str.GetString();

	MTLibTestUrl(url, TestUrlCB, (unsigned long)this);

}


void CIDCardFdvRegisterDlg::OnBnClickedBtnReg()
{
	// TODO: 在此添加控件通知处理程序代码
	CButton* btn_reg = (CButton*)GetDlgItem(IDC_BTN_REG);
	btn_reg->EnableWindow(false);

	CString editstr;
	GetDlgItem(IDC_REG_EDIT1)->GetWindowText(editstr);
	editstr.Remove(' ');
	std::string productsn = editstr.GetString();

	GetDlgItem(IDC_REG_EDIT2)->GetWindowText(editstr);
	editstr.Remove(' ');
	productsn = productsn + "-" + editstr.GetString();

	GetDlgItem(IDC_REG_EDIT3)->GetWindowText(editstr);
	editstr.Remove(' ');
	productsn = productsn + "-" + editstr.GetString();

	GetDlgItem(IDC_REG_EDIT4)->GetWindowText(editstr);
	editstr.Remove(' ');
	productsn = productsn + "-" + editstr.GetString();

	GetDlgItem(IDC_URL_EDIT)->GetWindowText(editstr);
	std::string url = editstr.GetString();

	char mac[64];
	GetLocalMAC(mac,sizeof(mac));
	std::string macId = mac;

	// uuid
	CString struuid(L"error");
	RPC_CSTR guidStr;
	GUID guid;
	if (UuidCreateSequential(&guid) == RPC_S_OK)
	{
		if (UuidToString(&guid, &guidStr) == RPC_S_OK)
		{
			struuid = (LPTSTR)guidStr;
			RpcStringFree(&guidStr);
		}
	}
	std::string uuid = std::string(struuid);

	MTLibCallRegister(url, m_cfgAppId, m_cfgApiKey, m_cfgSecretKey, uuid, productsn, macId,RegisterCB,(unsigned long)this);

	btn_reg->EnableWindow(true);
}

void CIDCardFdvRegisterDlg::saveConfig()
{
	std::ofstream confFile(m_strModulePath + "config.txt");
	confFile << "appId=" << m_cfgAppId << endl;
	confFile << "apiKey=" << m_cfgApiKey << endl;
	confFile << "secretKey=" << m_cfgSecretKey << endl;
	confFile << "url=" << m_cfgUrl << endl;
	confFile << "registeredNo=" << m_cfgRegisteredNo << endl;
	confFile.close();
}


