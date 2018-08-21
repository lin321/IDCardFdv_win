
// IDCardFdvRegisterDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IDCardFdvRegister.h"
#include "IDCardFdvRegisterDlg.h"
#include "afxdialogex.h"

#include "LocalMac.h"

#include <fstream>
#include <sstream>

#include <cpprest/http_client.h> 
#include <cpprest/json.h>
#include <openssl/sha.h>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;

std::string callregister(HANDLE handle, std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string productsn, std::string macId);
std::string callregisterSub(HANDLE handle, utility::string_t& url, web::json::value& postParameters);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CIDCardFdvRegisterDlg �Ի���



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

// CIDCardFdvRegisterDlg ��Ϣ�������

BOOL CIDCardFdvRegisterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
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

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIDCardFdvRegisterDlg::OnPaint()
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
HCURSOR CIDCardFdvRegisterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIDCardFdvRegisterDlg::OnBnClickedBtnTest()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString str;
	GetDlgItem(IDC_URL_EDIT)->GetWindowText(str);
	std::string urlstr = str.GetString();

	utility::string_t url = utility::conversions::to_string_t(urlstr);
	http::uri uri = http::uri(url);
	http_client client(uri);
	web::http::http_request postRequest;
	postRequest.set_method(methods::POST);
	try {
		Concurrency::task<web::http::http_response> getTask = client.request(postRequest);
		http_response resp = getTask.get();
		if (resp.status_code() == 200) {
			AfxMessageBox(_T("URL ����!"));
		}
		else {
			AfxMessageBox(_T("URL ������!"));
		}
	}
	catch (...) {
		AfxMessageBox(_T("URL ������!"));
		//printf("network error!\n");
	}
}


void CIDCardFdvRegisterDlg::OnBnClickedBtnReg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	std::string urlstr = editstr.GetString();

	char mac[64];
	GetLocalMAC(mac,sizeof(mac));
	std::string macId = mac;
	callregister(this,urlstr, m_cfgAppId, m_cfgApiKey, m_cfgSecretKey, productsn, macId);

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


// ---- sha256ժҪ��ϣ ---- //    
void sha256(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr)
{
	// ����sha256��ϣ    
	unsigned char mdStr[33] = { 0 };
	SHA256((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);

	// ��ϣ����ַ���    
	encodedStr = std::string((const char *)mdStr);
	// ��ϣ���ʮ�����ƴ� 32�ֽ�    
	char buf[65] = { 0 };
	char tmp[3] = { 0 };
	for (int i = 0; i < 32; i++)
	{
		sprintf_s(tmp, "%02x", mdStr[i]);
		strcat_s(buf, tmp);
	}
	buf[32] = '\0'; // ���涼��0����32�ֽڽض�    
	encodedHexStr = std::string(buf);
}

std::string callregister(HANDLE handle, std::string url, std::string appId, std::string apiKey, std::string secretKey,
						std::string productsn, std::string macId)
{
	json::value reg_json = json::value::object();

	std::string shastr = "";
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

	reg_json[U("appId")] = json::value::string(utility::conversions::to_string_t(appId));
	reg_json[U("apiKey")] = json::value::string(utility::conversions::to_string_t(apiKey));
	//reg_json[U("secretKey")] = json::value::string(utility::conversions::to_string_t(secretKey));
	reg_json[U("uuid")] = json::value::string(utility::conversions::to_string_t(std::string(struuid)));
	shastr += appId;
	shastr += apiKey;
	shastr += secretKey;
	shastr += struuid;

	reg_json[U("MacId")] = json::value::string(utility::conversions::to_string_t(macId));
	shastr += macId;

	reg_json[U("productsn")] = json::value::string(utility::conversions::to_string_t(productsn));
	shastr += productsn;

	std::string shaEncoded;
	std::string shaEncodedHex;
	sha256(shastr, shaEncoded, shaEncodedHex);
	reg_json[U("checksum")] = json::value::string(utility::conversions::to_string_t(shaEncodedHex));

	return callregisterSub(handle,utility::conversions::to_string_t(url), reg_json);
}
std::string callregisterSub(HANDLE handle, utility::string_t& url, web::json::value& postParameters)
{
	std::string ret;

	http::uri uri = http::uri(url);
	http_client client(uri);
	web::http::http_request postRequest;
	postRequest.set_method(methods::POST);
	postRequest.set_body(postParameters);

	try {
		Concurrency::task<web::http::http_response> getTask = client.request(postRequest);
		http_response resp = getTask.get();
		const json::value& jval = resp.extract_json().get();
		const web::json::object& jobj = jval.as_object();
		if (jval.has_field(U("Err_no"))) {
			int err_no = jobj.at(L"Err_no").as_integer();
			if (err_no != 0) {
				utility::string_t err_msg = jobj.at(L"Err_msg").as_string();
				ret = utility::conversions::to_utf8string(err_msg);
				//AfxMessageBox(ret.c_str());
				//printf("%ls\n", err_msg.c_str());
				if (404 == err_no) {
					AfxMessageBox(_T("�����к��ѱ�ע��!"));
				}
				else if (403 == err_no) {
					AfxMessageBox(_T("��Ч�����кţ�"));
				}
				else
					AfxMessageBox(_T("ע��ʧ�ܣ�"));
			}
			else {
				utility::string_t RegisteredNo = jobj.at(L"RegisteredNo").as_string();
				ret = utility::conversions::to_utf8string(RegisteredNo);
				AfxMessageBox(_T("ע��ɹ�!"));

				// save
				CIDCardFdvRegisterDlg* pDlg = (CIDCardFdvRegisterDlg*)handle;
				pDlg->m_cfgRegisteredNo = ret;
				pDlg->saveConfig();				
			}
		}
	}
	catch (...) {
		ret = "network error!";
		AfxMessageBox(_T("network error!"));
		//printf("network error!\n");
	}

	return ret;
}
