
// IDCardFdvDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "IDCardFdvDlg.h"
#include "afxdialogex.h"

#include "MTLibIDCardReader.h"
#include "LocalMac.h"
#include "utility_funcs.h"
#include "base64.h"

#include <fstream>

using namespace std;
using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define OPENCV_CAPTURE 1

#define CLEAR_INFOIMG_TIMER 1


UINT CameraShowThread(LPVOID lpParam);
UINT FdvThread(LPVOID lpParam);
std::string callverify(std::string url, std::string appId, std::string apiKey, std::string secretKey,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	std::vector<uchar> idcardPhoto, std::vector<uchar> verifyPhotos[], int verifyPhotoNum);



#if OPENCV_CAPTURE
#else
static VOID __stdcall FaceImageCB(HWND hWnd, BSTR imgBase64, ULONG_PTR userdata)
{
	CIDCardFdvDlg* handle = (CIDCardFdvDlg*)userdata;

	_bstr_t bstr_t(imgBase64);
	handle->m_sCaptureBase64 = bstr_t;
	handle->m_bFaceGot = true;
	ZZReleaseString(imgBase64);
}

static VOID __stdcall FaceResultCB(HWND hWnd, LONG result, BSTR feature, ULONG_PTR userdata)
{
	CIDCardFdvDlg* handle = (CIDCardFdvDlg*)userdata;
	if (0 == result) {
		handle->m_bIsAliveSample = true;
		handle->ProcessCapture();
	}
	else if (-3 == result) {
		handle->m_bIsAliveSample = false;
		handle->ProcessCapture();
	}
	else {
		handle->m_bFaceGot = false;
	}
	ZZReleaseString(feature);
}
#endif


// CIDCardFdvDlg 对话框



CIDCardFdvDlg::CIDCardFdvDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IDCARDFDV_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	camdevid = 0;
	m_bCameraRun = false;
	m_thCamera = NULL;
	ResetEvent(m_eCameraEnd);
	m_iplImgDisplay = NULL;
	m_bFlip = true;

	m_bCmdCapture = false;

	m_bFaceGot = false;
	m_bIsAliveSample = false;
	m_bFdvRun = false;

	memset(m_IdCardId, 0, sizeof(m_IdCardId));
	memset(m_IdCardIssuedate, 0, sizeof(m_IdCardIssuedate));
	memset(m_IdCardPhoto, 0, sizeof(m_IdCardPhoto));
	m_iplImgPhoto = NULL;

	m_pInfoDlg = NULL;

	m_hBIconCamera = NULL;
	m_iplImgCameraImg = NULL;
	m_iplImgResultIconRight = NULL;
	m_iplImgResultIconWrong = NULL;
	m_dThreshold = 77.0;
}

void CIDCardFdvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIDCardFdvDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_SYSCOMMAND()
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

// CIDCardFdvDlg 消息处理程序

BOOL CIDCardFdvDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MAXIMIZE);

	// TODO: 在此添加额外的初始化代码

	//获得电脑显示器的像素宽度和像素高度
	//int ax = GetDC()->GetDeviceCaps(HORZRES);
	//int ay = GetDC()->GetDeviceCaps(VERTRES);

	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	m_strModulePath = ExtractFilePath(szPath);

	
	// 调整控件大小位置
	CRect rect;
	GetClientRect(&rect);		// 获取整个对话框大小
	int rw = rect.right - rect.left;
	//int rh = rect.bottom - rect.top;
	int rh = rw * 720 / 1280;
	//rw = (int)(rw * 0.6);
	int offsetX = rect.left;
	int offsetY = rect.top;
	GetDlgItem(IDC_PREVIEW_IMG)->MoveWindow(offsetX, offsetY, rw, rh, false);

	// 创建信息窗口
	ClientToScreen(&rect);
	float IPRATE = 0.15f;
	rw = (int)((rect.right - rect.left) * IPRATE); // 信息窗口宽
	rh = rect.bottom - rect.top;					// 信息窗口高
	offsetX = rect.left + (int)((rect.right - rect.left) * (1 - IPRATE));
	offsetY = rect.top;
	if (NULL == m_pInfoDlg) {
		m_pInfoDlg = new CInfoDlg(offsetX, offsetY, rw, rh);
		m_pInfoDlg->Create(IDD_INFO_DIALOG, this);
	}
	m_pInfoDlg->ShowWindow(SW_SHOW);

	/*
	m_hBIconCamera = (HBITMAP)LoadImage(AfxGetInstanceHandle(),
	MAKEINTRESOURCE(IDB_ICON_CAMERA),
	IMAGE_BITMAP, 0, 0,
	LR_LOADMAP3DCOLORS);
	CStatic* m_cs = (CStatic*)GetDlgItem(IDC_CAMERA_IMAGE);
	m_cs->SetBitmap(m_hBIconCamera);
	*/

	//std::string fn = m_strModulePath;
	//fn += "camera.png";
	//m_iplImgCameraImg = cvLoadImage(fn.c_str(), -1);
	std::string fn = m_strModulePath + "right.png";
	m_iplImgResultIconRight = cvLoadImage(fn.c_str(), -1);
	fn = m_strModulePath + "wrong.png";
	m_iplImgResultIconWrong = cvLoadImage(fn.c_str(), -1);

	// IDCardReader
	LoadIDCardReader();

	// Mac ID
	char mac[64];
	GetLocalMAC(mac, sizeof(mac));
	m_macId = mac;

	// config
	m_cfgAppId = "10022245";
	m_cfgApiKey = "MGRhNjEyYWExOTdhYzYxNTkx";
	m_cfgSecretKey = "NzQyNTg0YmZmNDg3OWFjMTU1MDQ2YzIw";
	m_cfgUrl = "http://192.168.1.201:8004/idcardfdv";
	m_cfgRegisteredNo = "0";
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
				if (key == "registeredNo")
					m_cfgRegisteredNo = value;
			}
		}
	}
	confFile.close();

	
	m_pInfoDlg->setResultText("");
	m_pInfoDlg->setThresholdText("");
	


#if OPENCV_CAPTURE
	ResetEvent(m_eCameraEnd);
	startCameraThread();
#else
	ZZInitFaceMgr();
	ZZOpenDevice(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd, 2);
	ZZOpenHideDevice(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd, 4);
	ZZOpenVideo(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd);
	ZZOpenHideVideo(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd);
	ZZGetFaceFeature(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd, FaceImageCB, (ULONG_PTR)this, 2, FaceResultCB, (ULONG_PTR)this);
#endif

	// 显示初始值
	CString thstr;
	thstr.Format("%.2f%%", m_dThreshold);
	m_pInfoDlg->setThresholdText(thstr.GetString());

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

HBRUSH CIDCardFdvDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_DLG)  //对话框颜色
	{
		return (HBRUSH)::GetStockObject(WHITE_BRUSH);//返回自己设置的刷子
	}

	

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIDCardFdvDlg::OnPaint()
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
		//CPaintDC dc(this);
		//CRect rect;
		//GetClientRect(&rect);
		//dc.FillSolidRect(rect, RGB(255, 255, 255));	//白色填充客户区

		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CIDCardFdvDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIDCardFdvDlg::showPreview(IplImage* img)	
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
	//SetRect(rect, 0, 0, rw, rh);	// 铺满控件
	
	int displayW = rw;					// 保持宽高比和填满控件宽
	int displayH = rw * ih / iw;
	int displayX = 0;
	int displayY = (int)(rh - displayH) / 2;	// 垂直居中
	SetRect(rect, displayX, displayY, displayX + displayW, displayY + displayH);

	CvSize ImgSize;
	ImgSize.height = displayW;
	ImgSize.width = displayH;
	CvSize ImgOrgSize;
	ImgOrgSize.width = img->width;
	ImgOrgSize.height = img->height;
	if (m_iplImgDisplay == NULL)
		m_iplImgDisplay = cvCreateImage(ImgSize, IPL_DEPTH_8U, 3);
	
	if (m_iplImgTemp == NULL)
		m_iplImgTemp = cvCreateImage(ImgOrgSize, IPL_DEPTH_8U, 3);

	if (m_bFlip)
		cvFlip(img, m_iplImgTemp, 1);
	else
		cvCopy(img, m_iplImgTemp);
	

	cvResize(m_iplImgTemp, m_iplImgDisplay, INTER_CUBIC);

	CvvImage cimg;
	cimg.CopyOf(m_iplImgDisplay, -1);							// 复制图片
	cimg.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内

	ReleaseDC(pDC);
}

void CIDCardFdvDlg::drawCameraImage(IplImage* img)
{
	UINT ID = 0;

	CDC* pDC = GetDlgItem(ID)->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作

	CRect rect;

	GetDlgItem(ID)->GetClientRect(&rect);
	int rw = rect.right - rect.left;			// 求出图片控件的宽和高
	int rh = rect.bottom - rect.top;

	SetRect(rect, 0, 0, rw, rh);	// 铺满控件

	CvvImage cimg;
	cimg.CopyOf(img, -1);							// 复制图片
	cimg.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内

	ReleaseDC(pDC);
}

void CIDCardFdvDlg::startCameraThread()
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

void CIDCardFdvDlg::stopCameraThread()
{
	m_bCameraRun = false;
	if (m_thCamera) {
		WaitForSingleObject(m_eCameraEnd, INFINITE);
		ResetEvent(m_eCameraEnd);
	}

	m_thCamera = NULL;
}


void CIDCardFdvDlg::ProcessCapture()
{

#if OPENCV_CAPTURE
	if (!m_bCameraRun)
		return;
#else
	if (!m_bFaceGot)
		return;
#endif

	if (m_bFdvRun)
		return;

	// clear
	KillTimer(CLEAR_INFOIMG_TIMER);
	//m_pInfoDlg->clearCameraImage();
	//m_pInfoDlg->clearIdcardImage();
	//m_pInfoDlg->clearResultIcon();
	//m_pInfoDlg->setResultText("");

	m_thFdv = AfxBeginThread(FdvThread, this);

	if (NULL == m_thFdv)
	{
		TRACE("创建新的线程出错！\n");
		return;
	}
}

UINT CameraShowThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;
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

	pDlg->m_bCameraRun = true;
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
		if (pDlg->m_bCmdCapture) {
			pDlg->m_CaptureImage = cvCloneImage(cFrame);
			pDlg->m_bCmdCapture = false;
			SetEvent(pDlg->m_eCaptureEnd);
		}
		//pDlg->drawCameraImage(pDlg->m_iplImgCameraImg);
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

// capture thread
#include <cpprest/json.h>
UINT FdvThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;
	pDlg->m_bFdvRun = true;

	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	std::string strModulePath = ExtractFilePath(szPath);

	
#if OPENCV_CAPTURE
	pDlg->m_bCmdCapture = true;
	ResetEvent(pDlg->m_eCaptureEnd);
	WaitForSingleObject(pDlg->m_eCaptureEnd, INFINITE);
	pDlg->m_bIsAliveSample = true;
#else
	// 获取摄像头人脸
	std::string facedatastr;
	Base64::Decode(pDlg->m_sCaptureBase64, &facedatastr);
	std::vector<uchar> facebuff(facedatastr.begin(), facedatastr.end());
	cv::Mat facemat = cv::imdecode(facebuff, CV_LOAD_IMAGE_UNCHANGED);
	IplImage imgTmp = facemat;
	pDlg->m_CaptureImage = cvCloneImage(&imgTmp);
#endif

	// read idcard
	int iopen = OpenIDCardReader();
	if (iopen < 0) {
		pDlg->m_bFdvRun = false;
		return 0;
	}
	else
	{
		// 获取身份证号
		long len = IDCardReader_GetPeopleIDCode(pDlg->m_IdCardId, sizeof(pDlg->m_IdCardId));
		// 获取身份证有效期
		len = IDCardReader_GetStartDate(pDlg->m_IdCardIssuedate, sizeof(pDlg->m_IdCardIssuedate));

		// 获取照片
		long lenbmp = IDCardReader_GetPhotoBMP(pDlg->m_IdCardPhoto, sizeof(pDlg->m_IdCardPhoto));
		CloseIDCardReader();

		pDlg->m_iplImgPhoto = BMP2Ipl((unsigned char*)pDlg->m_IdCardPhoto, lenbmp);
		pDlg->m_pInfoDlg->clearResultIcon();  // 验证结果图标清空
		pDlg->m_pInfoDlg->drawCameraImage(pDlg->m_CaptureImage);
		pDlg->m_pInfoDlg->drawIdcardImage(pDlg->m_iplImgPhoto);
		if (false == pDlg->m_bIsAliveSample) {
			// 非活体警告
			pDlg->m_pInfoDlg->setResultTextSize(60);
			pDlg->m_pInfoDlg->setResultText("非活体！");
			pDlg->setClearTimer();
			pDlg->m_bFdvRun = false;
			return 0;
		}
		pDlg->m_pInfoDlg->setResultTextSize(60);
		pDlg->m_pInfoDlg->setResultText("--%");

		std::vector<uchar> idcardPhoto(pDlg->m_IdCardPhoto, pDlg->m_IdCardPhoto + lenbmp);
		std::vector<uchar> verifyPhotos[1];
#if OPENCV_CAPTURE
		vector<int> param = vector<int>(2);
		//param.push_back(CV_IMWRITE_JPEG_QUALITY);
		//param.push_back(9); //image quality
		//					 //param[0] = CV_IMWRITE_PNG_COMPRESSION;
		//					 //param[1] = 3; //default(3)  0-9.

		Mat frame(pDlg->m_CaptureImage, false);
		cv::imencode(".jpg", frame, verifyPhotos[0], param);
#else
		verifyPhotos[0] = facebuff;
#endif
		std::string retstr = callverify(pDlg->m_cfgUrl,
			pDlg->m_cfgAppId, pDlg->m_cfgApiKey, pDlg->m_cfgSecretKey,
			pDlg->m_macId, pDlg->m_cfgRegisteredNo,
			pDlg->m_IdCardId, pDlg->m_IdCardIssuedate, idcardPhoto, verifyPhotos, 1);

		
		// 返回结果处理
		double retsim = atof(retstr.c_str());
		if (retsim >= pDlg->m_dThreshold ) {
			pDlg->m_pInfoDlg->setResultTextSize(60);
			pDlg->m_pInfoDlg->setResultText(retstr + "%");
			pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconRight);
		}
		else if (retsim > 0.0) {
			pDlg->m_pInfoDlg->setResultTextSize(60);
			pDlg->m_pInfoDlg->setResultText(retstr + "%");
			pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconWrong);
		}
		else {
			// erro msg
			pDlg->m_pInfoDlg->setResultTextSize(20);
			pDlg->m_pInfoDlg->setResultText(retstr);
			pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconWrong);
		}

		pDlg->setClearTimer();
	}

	pDlg->m_bFdvRun = false;
	return 0;
}

void CIDCardFdvDlg::setClearTimer()
{
	KillTimer(CLEAR_INFOIMG_TIMER);
	SetTimer(CLEAR_INFOIMG_TIMER, 1000 * 10, NULL);
}

//==============================================
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <openssl/sha.h>
using namespace web;
using namespace web::http;
using namespace web::http::client;
std::string callverifySub(utility::string_t& url, web::json::value& postParameters);

// ---- sha256摘要哈希 ---- //    
void sha256(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr)
{
	// 调用sha256哈希    
	unsigned char mdStr[33] = { 0 };
	SHA256((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);

	// 哈希后的字符串    
	encodedStr = std::string((const char *)mdStr);
	// 哈希后的十六进制串 32字节    
	char buf[65] = { 0 };
	char tmp[3] = { 0 };
	for (int i = 0; i < 32; i++)
	{
		sprintf_s(tmp, "%02x", mdStr[i]);
		strcat_s(buf, tmp);
	}
	buf[32] = '\0'; // 后面都是0，从32字节截断    
	encodedHexStr = std::string(buf);
}

std::string callverify(std::string url, std::string appId, std::string apiKey, std::string secretKey,
				std::string macId, std::string registeredNo,
				std::string idcardId, std::string idcardIssuedate,
				std::vector<uchar> idcardPhoto, std::vector<uchar> verifyPhotos[], int verifyPhotoNum)
{
	json::value verify_json = json::value::object();

	std::string shastr="";
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

	verify_json[U("appId")] = json::value::string(utility::conversions::to_string_t(appId));
	verify_json[U("apiKey")] = json::value::string(utility::conversions::to_string_t(apiKey));
	verify_json[U("secretKey")] = json::value::string(utility::conversions::to_string_t(secretKey));
	verify_json[U("uuid")] = json::value::string(utility::conversions::to_string_t(std::string(struuid)));
	shastr += appId;
	shastr += apiKey;
	shastr += secretKey;
	shastr += struuid;

	//std::string macId = "00:09:4c:53:78:2c"; // test
	verify_json[U("MacId")] = json::value::string(utility::conversions::to_string_t(macId));
	shastr += macId;

	//std::string registerno = "9081"; // test
	verify_json[U("RegisteredNo")] = json::value::string(utility::conversions::to_string_t(registeredNo));
	shastr += registeredNo;

	verify_json[U("idcard_id")] = json::value::string(utility::conversions::to_string_t(idcardId));
	verify_json[U("idcard_issuedate")] = json::value::string(utility::conversions::to_string_t(idcardIssuedate));
	shastr += idcardId;
	shastr += idcardIssuedate;

	utility::string_t b64str = U("data:image/bmp;base64,") + utility::conversions::to_base64(idcardPhoto);
	verify_json[U("idcard_photo")] = json::value::string(b64str);
	shastr += utility::conversions::to_utf8string(b64str);

	verify_json[U("verify_photos")] = json::value::array();
	for (int i = 0; i < verifyPhotoNum; i++) {
		utility::string_t b64str = U("data:image/jpeg;base64,") + utility::conversions::to_base64(verifyPhotos[i]);
		verify_json[U("verify_photos")][i] = json::value::string(b64str);
		shastr += utility::conversions::to_utf8string(b64str);
	}

	std::string shaEncoded;
	std::string shaEncodedHex;
	sha256(shastr, shaEncoded, shaEncodedHex);
	verify_json[U("checksum")] = json::value::string(utility::conversions::to_string_t(shaEncodedHex));

	return callverifySub(utility::conversions::to_string_t(url), verify_json);
}
std::string callverifySub(utility::string_t& url, web::json::value& postParameters)
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
				//AfxMessageBox(err_msg.c_str());
				//printf("%ls\n", err_msg.c_str());
			}
			else {
				double sim = jobj.at(L"Similarity").as_double() * 100;
				CString csTemp;
				csTemp.Format("%.2f", sim);
				ret = csTemp.GetString();
			}
		}
	}
	catch (...) {
		ret = "network error!";
		//AfxMessageBox(_T("network error!"));
		//printf("network error!\n");
	}

	return ret;
}

//==============================================


BOOL CIDCardFdvDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
#if OPENCV_CAPTURE
	stopCameraThread();
	Sleep(20);
#else
	m_pInfoDlg->ShowWindow(SW_HIDE);
	ZZStopGetFace(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd);
	ZZCloseHideVideo(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd);
	ZZCloseVideo(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd);
	ZZCloseHideDevice(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd);
	ZZCloseDevice(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd);
	ZZDeinitFaceMgr();
#endif
	// IDCardReader
	UnloadIDCardReader();

	if (m_iplImgDisplay != NULL) {
		cvReleaseImage(&m_iplImgDisplay);
		m_iplImgDisplay = NULL;
	}

	if (m_iplImgTemp != NULL) {
		cvReleaseImage(&m_iplImgTemp);
		m_iplImgTemp = NULL;
	}
	
	if (m_hBIconCamera != NULL) {
		DeleteObject(m_hBIconCamera);
		m_hBIconCamera = NULL;
	}

	if (m_iplImgCameraImg != NULL) {
		cvReleaseImage(&m_iplImgCameraImg);
		m_iplImgCameraImg = NULL;
	}

	if (m_iplImgResultIconRight != NULL) {
		cvReleaseImage(&m_iplImgResultIconRight);
		m_iplImgResultIconRight = NULL;
	}

	if (m_iplImgResultIconWrong != NULL) {
		cvReleaseImage(&m_iplImgResultIconWrong);
		m_iplImgResultIconWrong = NULL;
	}
	
	if (m_CaptureImage != NULL) {
		cvReleaseImage(&m_CaptureImage);
		m_CaptureImage = NULL;
	}

	if (m_iplImgPhoto != NULL) {
		cvReleaseImage(&m_iplImgPhoto);
		m_iplImgPhoto = NULL;
	}

	if (m_pInfoDlg) {
		delete m_pInfoDlg;
		m_pInfoDlg = NULL;
	}

	PostQuitMessage(0);
	return CDialogEx::DestroyWindow();
}

void CIDCardFdvDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
}




void CIDCardFdvDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// test code
	ProcessCapture();
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CIDCardFdvDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case CLEAR_INFOIMG_TIMER:
		m_pInfoDlg->clearCameraImage();
		m_pInfoDlg->clearIdcardImage();
		m_pInfoDlg->clearResultIcon();
		m_pInfoDlg->setResultText("");
		KillTimer(CLEAR_INFOIMG_TIMER);
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CIDCardFdvDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((nID & 0xFFF0) == SC_MOVE)
		return;

	CDialogEx::OnSysCommand(nID, lParam);
}
