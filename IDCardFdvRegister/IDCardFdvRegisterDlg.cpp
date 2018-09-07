
// IDCardFdvRegisterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdvRegister.h"
#include "IDCardFdvRegisterDlg.h"
#include "afxdialogex.h"

#include "LocalMac.h"
#include "MTLibNetwork.h"
#include "QRFuncs.h"

#include <fstream>
#include <sstream>
#include <string>
#include <regex> 

using namespace std;
using namespace cv;

CCriticalSection g_CriticalSection;
UINT CameraShowThread(LPVOID lpParam);

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

	camdevid = 0;
	m_bCameraRun = false;
	m_iQRbtnState = 0;
	m_thCamera = NULL;
	ResetEvent(m_eCameraEnd);
	m_iplImgDisplay = NULL;
	m_iplImgTemp = NULL;
	m_bFlip = true;

	qrScanCnt = 0;
	qrScanCntMax = 2;
	m_iplImgQRimg = NULL;
	m_iplImgQRimgGray = NULL;
	m_pScanMaskDlg = NULL;
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
	ON_BN_CLICKED(IDC_BTN_QRSCAN, &CIDCardFdvRegisterDlg::OnBnClickedBtnQrscan)
	ON_WM_SHOWWINDOW()
	ON_WM_MOVE()
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

	// scan mask
	if (NULL == m_pScanMaskDlg) {
		m_pScanMaskDlg = new CQRScanMaskDlg();
		m_pScanMaskDlg->Create(IDD_QRSCAN_MASK, this);
	}

	// config
	m_cfgAppId = "10022245";
	m_cfgApiKey = "MGRhNjEyYWExOTdhYzYxNTkx";
	m_cfgSecretKey = "NzQyNTg0YmZmNDg3OWFjMTU1MDQ2YzIw";
	m_cfgUrl = "http://192.168.1.201:8004/idcardfdv";
	m_cfgTimeOut = "15";
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
				if (key == "timeout")
					m_cfgTimeOut = value;
			}
		}
	}
	confFile.close();
	m_cfgRegisteredNo = "";

	int idx = m_cfgUrl.find_last_of('/');
	std::string regurl = m_cfgUrl.substr(0, idx);
	regurl = regurl + "/registerproduct";
	GetDlgItem(IDC_URL_EDIT)->SetWindowText(regurl.c_str());

	CButton* check_reg = (CButton*)GetDlgItem(IDC_CHECK_REG);
	check_reg->SetCheck(1);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CIDCardFdvRegisterDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	// scan mask
	if (m_pScanMaskDlg) {
		CRect rect;
		GetDlgItem(IDC_PREVIEW_IMG)->GetWindowRect(&rect);
		m_pScanMaskDlg->MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, false);
		//m_pScanMaskDlg->ShowWindow(SW_SHOW);
	}
}


void CIDCardFdvRegisterDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	// TODO: 在此处添加消息处理程序代码
	if (m_pScanMaskDlg) {
		CRect rect;
		GetDlgItem(IDC_PREVIEW_IMG)->GetWindowRect(&rect);
		m_pScanMaskDlg->MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, false);
	}
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

	MTLibTestUrl(url, TestUrlCB, (unsigned long)this, ::stoi(m_cfgTimeOut));

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

	MTLibCallRegister(url, m_cfgAppId, m_cfgApiKey, m_cfgSecretKey, uuid, productsn, macId,RegisterCB,(unsigned long)this, ::stoi(m_cfgTimeOut));

	btn_reg->EnableWindow(true);
}

void CIDCardFdvRegisterDlg::showPreview(IplImage* img)
{
	UINT ID = IDC_PREVIEW_IMG;				// ID 是Picture Control控件的ID号

	CDC* pDC = GetDlgItem(ID)->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作

	CRect rect;

	GetDlgItem(ID)->GetClientRect(&rect);
	int rw = rect.right - rect.left;			// 求出图片控件的宽和高
	int rh = rect.bottom - rect.top;

	int iw = img->width;						// 读取图片的宽和高
	int ih = img->height;
	//TRACE("rect:w,h %d, %d: %d, %d", rw, rh, iw, ih);
	//int tx = (int)(rw - iw)/2;					// 使图片的显示位置正好在控件的正中
	//int ty = (int)(rh - ih)/2;
	//SetRect( rect, tx, ty, tx+iw, ty+ih );
	SetRect(rect, 0, 0, rw, rh);	// 铺满控件
	
	float dRate = 1.5;
	int displayW = (int)(rw * dRate);					// 在图片里取控件大小的区域 * dRate
	int displayH = (int)(rh * dRate);
	int displayX = (iw - displayW) / 2;		// 取中间区域
	int displayY = (ih - displayH) / 2;

	// 显示区域裁剪
	CvSize ImgSize;
	ImgSize.width = displayW;
	ImgSize.height = displayH;
	if (m_iplImgTemp == NULL)
		m_iplImgTemp = cvCreateImage(ImgSize, img->depth, img->nChannels);

	cvSetImageROI(img, cvRect(displayX, displayY, displayW, displayH));
	cvCopy(img, m_iplImgTemp);
	cvResetImageROI(img);

	// 二维码识别
	qrScanCnt++;
	if (qrScanCnt >= qrScanCntMax) {
		qrScanCnt = 0;

		CvSize ScanSize;
		ScanSize.width = (int)(m_pScanMaskDlg->getScanAreaSize() * dRate);
		ScanSize.height = ScanSize.width;
		if (m_iplImgQRimg == NULL)
			m_iplImgQRimg = cvCreateImage(ScanSize, img->depth, img->nChannels);
		cvSetImageROI(m_iplImgTemp, cvRect((ImgSize.width - ScanSize.width)/2, (ImgSize.height - ScanSize.height)/2, ScanSize.width, ScanSize.height));
		cvCopy(m_iplImgTemp, m_iplImgQRimg);		// 识别区域裁剪
		cvResetImageROI(m_iplImgTemp);

		if (m_iplImgQRimgGray == NULL)
			m_iplImgQRimgGray = cvCreateImage(ScanSize, img->depth, 1);
		cvCvtColor(m_iplImgQRimg, m_iplImgQRimgGray, CV_BGR2GRAY);	// 转灰度图
		string qrstr = GetQR(m_iplImgQRimgGray);
		if (!qrstr.empty()) {
			const std::tr1::regex pattern("^[A-Fa-f0-9]{4}(-[A-Fa-f0-9]{4}){3}$");
			bool match = std::regex_search(qrstr, pattern);			// 正则判断格式
			if (match) {
				setProductSnText(qrstr);
				CButton* check_reg = (CButton*)GetDlgItem(IDC_CHECK_REG);
				if (check_reg->GetCheck())
					OnBnClickedBtnReg();
			}
		}
	}

	// flip
	IplImage *pImg;
	if (m_iplImgDisplay == NULL)
		m_iplImgDisplay = cvCreateImage(ImgSize, img->depth, img->nChannels);
	if (m_bFlip) {
		cvFlip(m_iplImgTemp, m_iplImgDisplay, 1);
		pImg = m_iplImgDisplay;
	}
	else
		pImg = m_iplImgTemp;

	CvvImage cimg;
	cimg.CopyOf(pImg, -1);							// 复制图片
	cimg.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内

	ReleaseDC(pDC);
}

void CIDCardFdvRegisterDlg::startCameraThread()
{
	ResetEvent(m_eCameraEnd);

	if (m_thCamera == NULL) {
		m_thCamera = AfxBeginThread(CameraShowThread, this);
		if (NULL == m_thCamera)
		{
			TRACE("创建新的线程出错！\n");
			return;
		}
	}
}

void CIDCardFdvRegisterDlg::stopCameraThread()
{
	g_CriticalSection.Lock();
	m_bCameraRun = false;
	g_CriticalSection.Unlock();
	if (m_thCamera) {
		WaitForSingleObject(m_eCameraEnd, INFINITE);
		ResetEvent(m_eCameraEnd);
	}

	m_thCamera = NULL;
}

void CIDCardFdvRegisterDlg::setProductSnText(std::string str)
{
	string strbuf = str;
	string text;
	text = strbuf.substr(0, strbuf.find('-'));
	GetDlgItem(IDC_REG_EDIT1)->SetWindowText(text.c_str());
	strbuf = strbuf.substr(strbuf.find('-')+1);

	text = strbuf.substr(0, strbuf.find('-'));
	GetDlgItem(IDC_REG_EDIT2)->SetWindowText(text.c_str());
	strbuf = strbuf.substr(strbuf.find('-') + 1);

	text = strbuf.substr(0, strbuf.find('-'));
	GetDlgItem(IDC_REG_EDIT3)->SetWindowText(text.c_str());
	strbuf = strbuf.substr(strbuf.find('-') + 1);

	GetDlgItem(IDC_REG_EDIT4)->SetWindowText(strbuf.c_str());
}

void CIDCardFdvRegisterDlg::saveConfig()
{
	std::ofstream confFile(m_strModulePath + "config.txt");
	confFile << "appId=" << m_cfgAppId << endl;
	confFile << "apiKey=" << m_cfgApiKey << endl;
	confFile << "secretKey=" << m_cfgSecretKey << endl;
	confFile << "url=" << m_cfgUrl << endl;
	confFile << "timeout=" << m_cfgTimeOut << endl;
	confFile << "registeredNo=" << m_cfgRegisteredNo << endl;
	confFile.close();
}

UINT CameraShowThread(LPVOID lpParam)
{
	CIDCardFdvRegisterDlg* pDlg = (CIDCardFdvRegisterDlg*)lpParam;
	IplImage* cFrame = NULL;
	CvCapture* pCapture = cvCreateCameraCapture(pDlg->camdevid);
	//int frameW = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH, 1280);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT, 720);

	Sleep(500);

	if (pCapture == NULL) {
		SetEvent(pDlg->m_eCameraEnd);
		return 0;
	}

	g_CriticalSection.Lock();
	pDlg->m_bCameraRun = true;
	g_CriticalSection.Unlock();

	while (pDlg->m_bCameraRun)
	{
		WINDOWPLACEMENT wpl;

		wpl.length = sizeof(WINDOWPLACEMENT);
		if (pDlg->GetWindowPlacement(&wpl) && (wpl.showCmd == SW_SHOWMINIMIZED)) {
			if (pCapture) {
				cvReleaseCapture(&pCapture);
				pCapture = NULL;
			}
			Sleep(5);
			continue;
		}

		if (pCapture == NULL) {
			pCapture = cvCreateCameraCapture(pDlg->camdevid);
			if (pCapture == NULL) {
				Sleep(2000);
				continue;
			}
			Sleep(200);
		}
		cFrame = cvQueryFrame(pCapture);
		if (!cFrame) {
			//pDlg->MessageBox("读取图像帧错误！", "出错信息：", MB_ICONERROR | MB_OK);
			continue;
		}


		IplImage* newframe = cvCloneImage(cFrame);
		//if (pDlg->m_bCmdCapture) {
		//	pDlg->m_CaptureImage = cvCloneImage(cFrame);
		//	pDlg->m_bCmdCapture = false;
		//	SetEvent(pDlg->m_eCaptureEnd);
		//}

		pDlg->showPreview(newframe);


		Sleep(7);
		//		int c=cvWaitKey(33);   // not work in MFC proj
		cvReleaseImage(&newframe);
		//		if(c==27)break;
	}

	if (pCapture) {
		cvReleaseCapture(&pCapture);
		pCapture = NULL;
	}

	SetEvent(pDlg->m_eCameraEnd);
	return 0;
}





BOOL CIDCardFdvRegisterDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	stopCameraThread();
	Sleep(20);

	if (m_iplImgDisplay != NULL) {
		cvReleaseImage(&m_iplImgDisplay);
		m_iplImgDisplay = NULL;
	}

	if (m_iplImgTemp != NULL) {
		cvReleaseImage(&m_iplImgTemp);
		m_iplImgTemp = NULL;
	}

	if (m_iplImgQRimg != NULL) {
		cvReleaseImage(&m_iplImgQRimg);
		m_iplImgQRimg = NULL;
	}

	if (m_iplImgQRimgGray != NULL) {
		cvReleaseImage(&m_iplImgQRimgGray);
		m_iplImgQRimgGray = NULL;
	}

	if (m_pScanMaskDlg) {
		delete m_pScanMaskDlg;
		m_pScanMaskDlg = NULL;
	}

	return CDialogEx::DestroyWindow();
}


void CIDCardFdvRegisterDlg::OnBnClickedBtnQrscan()
{
	// TODO: 在此添加控件通知处理程序代码
	CButton* btn = (CButton*)GetDlgItem(IDC_BTN_QRSCAN);
	if (0 == m_iQRbtnState) {
		btn->SetWindowText(_T("停止"));
		m_iQRbtnState = 1;
		m_pScanMaskDlg->setScanAreaSize(200);
		m_pScanMaskDlg->ShowWindow(SW_SHOW);
		startCameraThread();
	}
	else {
		btn->EnableWindow(false);
		stopCameraThread();
		m_pScanMaskDlg->ShowWindow(SW_HIDE);
		Invalidate();	// clear preview
		btn->SetWindowText(_T("启动"));
		btn->EnableWindow(true);
		m_iQRbtnState = 0;
	}
}






