
// IDCardFdvDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "IDCardFdvDlg.h"
#include "afxdialogex.h"

#include "MTLibIDCardReader.h"
#include "MTLibNetwork.h"
#include "LocalMac.h"
#include "utility_funcs.h"
#include "base64.h"

#include <fstream>
#include <exception>

using namespace std;
using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define OPENCV_CAPTURE 1

#define CLEAR_INFOIMG_TIMER 1

clock_t time_net=0;

CCriticalSection g_CriticalSection;
UINT FaceDetectThread(LPVOID lpParam);
UINT CameraShowThread(LPVOID lpParam);
UINT FdvThread(LPVOID lpParam);

#if OPENCV_CAPTURE
#else
#include "MTLibCameraLib.h"

static void __stdcall FaceImageCB(HWND hWnd, BSTR imgBase64, ULONG_PTR userdata)
{
	CIDCardFdvDlg* handle = (CIDCardFdvDlg*)userdata;

	_bstr_t bstr_t(imgBase64);
	handle->m_sCaptureBase64 = bstr_t;
	handle->m_bFaceGot = true;
}

static void __stdcall FaceResultCB(HWND hWnd, LONG result, ULONG_PTR userdata)
{
	CIDCardFdvDlg* handle = (CIDCardFdvDlg*)userdata;
	if (0 == result) {
		handle->m_bIsAliveSample = true;
		handle->startFdvThread();
	}
	else if (-3 == result) {
		handle->m_bIsAliveSample = false;
		handle->startFdvThread();
	}
	else {
		handle->m_bFaceGot = false;
	}
}
#endif

// 识别callback
static void __stdcall VerifyCB(int err_no, std::string err_msg, double similarity, MTLIBPTR userdata)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)userdata;

	if (MTLIBNETWORK_NETWORK_ERROR == err_no) {
		// 超时或其他网络错误处理
#ifdef NDEBUG
		double sim = 0.0;
		int simret = pDlg->m_pfrmwrap->ai_fdr_similarity(pDlg->m_photoFaceFeat, pDlg->m_frameFaceFeats[0], sim);
		VerifyCB(0, "", sim, userdata);
		return;
#endif
	}

	double retsim = similarity * 100;
	CString csTemp;
	csTemp.Format("%.2f", retsim);
	std::string retstr = csTemp.GetString();
	//AfxMessageBox(csTemp);
	if (retsim >= pDlg->m_dThreshold) {
		pDlg->m_pInfoDlg->setResultTextSize(60);
		pDlg->m_pInfoDlg->setResultText(retstr + "%");
		pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconRight);
	}
	else if (retsim >= 0.0) {
		pDlg->m_pInfoDlg->setResultTextSize(60);
		pDlg->m_pInfoDlg->setResultText(retstr + "%");
		pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconWrong);
	}
	else {
		// error
		pDlg->m_pInfoDlg->setResultTextSize(20);
		pDlg->m_pInfoDlg->setResultText(err_msg);
		pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconWrong);
	}

	pDlg->setClearTimer();

	clock_t dt = clock() - time_net;
	CString csTemp1;
	csTemp1.Format("%d", dt);
//	AfxMessageBox(csTemp1);
	time_net = 0;
}

// CIDCardFdvDlg 对话框
CIDCardFdvDlg::CIDCardFdvDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IDCARDFDV_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//camdevid = 0;
	m_bCameraRun = false;
	m_thCamera = NULL;
	ResetEvent(m_eCameraEnd);
	m_iplImgDisplay = NULL;
	m_iplImgTemp = NULL;
	m_bFlip = false;

	m_bFaceDetectRun = false;
	m_thFaceDetect = NULL;
	ResetEvent(m_eFaceDetectEnd);
	m_bCmdDetect = false;
	m_faces.clear();

	m_bCmdCapture = false;
	m_CaptureImage = NULL;
	m_CaptureImageHide = NULL;
	m_bFaceGot = false;
	m_bIsAliveSample = false;
	m_bFdvRun = false;
	m_iplImgTestImage = NULL;
	m_iplImgTestImage2 = NULL;
#ifdef NDEBUG
	m_pfrmwrap = NULL;
#endif

	m_bMainWinClose = false;

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
	//fn = m_strModulePath + "testp.png";
	//m_iplImgTestImage = cvLoadImage(fn.c_str(), -1);

	// IDCardReader
	LoadIDCardReader();

	// Mac ID
	char mac[64];
	GetLocalMAC(mac, sizeof(mac));
	m_macId = mac;

	//ai_fdr
#if NDEBUG
	std::string modelpath = m_strModulePath;
	m_pfrmwrap = new fdr_model_wrap(modelpath);

#else
	// opencv face detect
	std::string faceCascadeFilename = m_strModulePath + "haarcascade_frontalface_default.xml";
	try {
		faceCascade.load(faceCascadeFilename);
	}
	catch (cv::Exception e) {}
	if (faceCascade.empty()) {
		AfxMessageBox("haarcascade_frontalface_default.xml加载出错！");
	}
#endif

	

	// config
	m_cfgCameraVid = "2AB8";
	m_cfgCameraPid = "A101";
	m_cfgCameraHideVid = "2AB8";
	m_cfgCameraHidePid = "C101";
	m_cfgAppId = "10022245";
	m_cfgApiKey = "MGRhNjEyYWExOTdhYzYxNTkx";
	m_cfgSecretKey = "NzQyNTg0YmZmNDg3OWFjMTU1MDQ2YzIw";
	m_cfgUrl = "http://192.168.1.201:8004/idcardfdv";
	m_cfgTimeOut = "15";
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
				if (key == "cameraVid")
					m_cfgCameraVid = value;
				if (key == "cameraPid")
					m_cfgCameraPid = value;
				if (key == "cameraHideVid")
					m_cfgCameraHideVid = value;
				if (key == "cameraHidePid")
					m_cfgCameraHidePid = value;
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

	ResetEvent(m_eFaceDetectEnd);
	startFaceDetectThread();
#else
	MTLibLoadCamera();
	MTLibOpenCamera(GetDlgItem(IDC_PREVIEW_IMG)->m_hWnd,
					FaceImageCB, (ULONG_PTR)this, 
					FaceResultCB, (ULONG_PTR)this);
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

	CvSize ImgOrgSize;
	ImgOrgSize.width = img->width;
	ImgOrgSize.height = img->height;
	if (m_iplImgDisplay == NULL)
		m_iplImgDisplay = cvCreateImage(ImgOrgSize, img->depth, img->nChannels);
	
	IplImage* pImg;
	if (m_bFlip) {
		cvFlip(img, m_iplImgDisplay, 1);
		pImg = m_iplImgDisplay;
	}
	else
		pImg = img;

	CvvImage cimg;
	cimg.CopyOf(pImg, -1);							// 复制图片
	cimg.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内

	//CBrush base(RGB(255, 255, 255));
	//CBrush color(RGB(0, 0, 255));
	//pDC->FillRect(CRect(100, 100, 200, 200), &base);

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

// function:阻塞并等待,但处理消息
void WaitObjectAndMsg(HANDLE hEvent, DWORD dwMilliseconds) {
	BOOL bWait = TRUE;
	DWORD dwResult = 0;

	while (bWait)
	{
		DWORD dwResult = ::MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwMilliseconds, QS_ALLINPUT);

		if (WAIT_OBJECT_0 == dwResult) {
			break;
		}
		else {
			MSG msg;
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			DispatchMessage(&msg);
		}
	}
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
	if (m_thCamera && m_bCameraRun) {
		g_CriticalSection.Lock();
		m_bCameraRun = false;
		g_CriticalSection.Unlock();

		WaitObjectAndMsg(m_eCameraEnd, INFINITE);
		ResetEvent(m_eCameraEnd);

		m_thCamera = NULL;
	}
}

void CIDCardFdvDlg::startFaceDetectThread()
{
	ResetEvent(m_eFaceDetectEnd);

	if (m_thFaceDetect == NULL) {
		m_thFaceDetect = AfxBeginThread(FaceDetectThread, this);
		if (NULL == m_thFaceDetect)
		{
			TRACE("创建新的线程出错！\n");
			return;
		}
	}
}

void CIDCardFdvDlg::stopFaceDetectThread()
{
	if (m_thFaceDetect && m_bFaceDetectRun) {
		g_CriticalSection.Lock();
		m_bFaceDetectRun = false;
		g_CriticalSection.Unlock();
		SetEvent(m_eCaptureForDetect); // 放弃等待截图
		WaitObjectAndMsg(m_eFaceDetectEnd, INFINITE);
		ResetEvent(m_eFaceDetectEnd);

		m_thFaceDetect = NULL;
	}
}


void CIDCardFdvDlg::startFdvThread()
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

#if OPENCV_CAPTURE
	m_bCmdCapture = true;	// 确保在预览线程中调用startFdvThread()后可立即处理截图
	ResetEvent(m_eCaptureEnd);
#endif
	m_bFdvRun = true;
	ResetEvent(m_eFdvEnd);
	m_thFdv = AfxBeginThread(FdvThread, this);

	if (NULL == m_thFdv)
	{
		TRACE("创建新的线程出错！\n");
		m_bFdvRun = false;
		return;
	}
}

void CIDCardFdvDlg::waitFdvThreadStopped()
{
	if (m_thFdv && m_bFdvRun) {
		WaitObjectAndMsg(m_eFdvEnd, INFINITE);
		ResetEvent(m_eFdvEnd);
	}

	m_thFdv = NULL;
}


//face detect thread
UINT FaceDetectThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	g_CriticalSection.Lock();
	pDlg->m_bFaceDetectRun = true;
	g_CriticalSection.Unlock();

	int times_per_sec = 5;
	clock_t time_s, detect_dt;
	while (pDlg->m_bFaceDetectRun)
	{
		time_s = clock();
		pDlg->m_bCmdDetect = true;
		ResetEvent(pDlg->m_eCaptureForDetect);
		WaitForSingleObject(pDlg->m_eCaptureForDetect, INFINITE);
		if (pDlg->m_bMainWinClose)
			break;

		//*/
		//clock_t time1 = clock();
		std::vector < std::vector<int>> face_rects;
		Mat matframe(cvarrToMat(pDlg->m_iplImgCameraImg));

		pDlg->m_pfrmwrap->dectect_faces(matframe, face_rects, -1, true);
		g_CriticalSection.Lock();
		pDlg->m_faces.clear();
		vector<Rect>().swap(pDlg->m_faces);
		g_CriticalSection.Unlock();
		if (face_rects.size() > 0) {
			Rect tmpRect;
			tmpRect.x = face_rects[0][0];
			tmpRect.y = face_rects[0][1];
			tmpRect.width = face_rects[0][2]- face_rects[0][0];
			tmpRect.height = face_rects[0][3]- face_rects[0][1];
			g_CriticalSection.Lock();
			pDlg->m_faces.push_back(tmpRect);
			g_CriticalSection.Unlock();
		}

		//clock_t dt = clock() - time1;
		//CString csTemp;
		//csTemp.Format("%d", dt);
		//AfxMessageBox(csTemp);
		//*/

		detect_dt = clock() - time_s;
		//CString csTemp;
		//csTemp.Format("%d", detect_dt);
		//AfxMessageBox(csTemp);
		if (detect_dt < 1000 / times_per_sec) {
			Sleep(1000 / times_per_sec - detect_dt);
		}
	}

	SetEvent(pDlg->m_eFaceDetectEnd);
	return 0;
}

//camera preview thread
UINT CameraShowThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	int mainDevIdx = getDeviceIndex(pDlg->m_cfgCameraVid, pDlg->m_cfgCameraPid);
	int hideDevIdx = getDeviceIndex(pDlg->m_cfgCameraHideVid, pDlg->m_cfgCameraHidePid);

	if (-1 == mainDevIdx)
		mainDevIdx = 0;

	Mat cFrame;
	VideoCapture captureMain;
	captureMain.open(mainDevIdx); // captureMain.open(mainDevIdx, CAP_DSHOW);
	if (!captureMain.isOpened()) {
		SetEvent(pDlg->m_eCameraEnd);
		return 0;
	}
	captureMain.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	captureMain.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	Mat cFrameHide;
	VideoCapture captureHide;
	if (hideDevIdx >= 0) {
		captureHide.open(hideDevIdx); // captureHide.open(hideDevIdx, CAP_DSHOW);
		if (captureHide.isOpened()) {
			captureHide.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
			captureHide.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
		}
	}

	Sleep(500);

	g_CriticalSection.Lock();
	pDlg->m_bCameraRun = true;
	g_CriticalSection.Unlock();

	while (pDlg->m_bCameraRun)
	{
		WINDOWPLACEMENT wpl;

		wpl.length = sizeof(WINDOWPLACEMENT);
		if (pDlg->GetWindowPlacement(&wpl) && (wpl.showCmd == SW_SHOWMINIMIZED)) {
			if (!captureMain.isOpened()) {
				captureMain.release();
			}
			if (!captureHide.isOpened()) {
				captureHide.release();
			}
			Sleep(5);
			continue;
		}

		if (!captureMain.isOpened()) {
			captureMain.open(mainDevIdx); // captureMain.open(mainDevIdx, CAP_DSHOW);
			if (!captureMain.isOpened()) {
				Sleep(2000);
				continue;
			}
			captureMain.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
			captureMain.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
			Sleep(200);
		}
		if (hideDevIdx >= 0 && !captureHide.isOpened()) {
			captureHide.open(hideDevIdx); // captureHide.open(hideDevIdx, CAP_DSHOW);
			if (!captureHide.isOpened()) {
				Sleep(2000);
				continue;
			}
			captureHide.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
			captureHide.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
			Sleep(200);
		}

		captureMain.read(cFrame);
		if (cFrame.empty()) {
			//AfxMessageBox("读取图像帧错误！");
			//captureMain.release();
			continue;
		}
		if (captureHide.isOpened()) {
			// 避免其他处理用时造成的误差，先在此读入
			captureHide.read(cFrameHide);
			if (cFrameHide.empty()) {
				//AfxMessageBox("红外摄像头读取图像帧错误！");
				//captureHide.release();
				continue;
			}
		}

		IplImage* newframe = &IplImage(cFrame);
		if (pDlg->m_bCmdDetect) {
			if (pDlg->m_iplImgCameraImg) {
				cvReleaseImage(&(pDlg->m_iplImgCameraImg));
				pDlg->m_iplImgCameraImg = NULL;
			}
			pDlg->m_iplImgCameraImg = cvCloneImage(newframe);
			pDlg->m_bCmdDetect = false;
			SetEvent(pDlg->m_eCaptureForDetect);
		}
		
		double scale = 1.0;

#ifdef NDEBUG
#else
//*		//clock_t time1 = clock();
		scale = 2.0;
		Mat imgGray, imgSamll;
		cvtColor(cFrame, imgGray, CV_RGB2GRAY);
		resize(imgGray, imgSamll, Size(), 1/scale, 1 / scale, INTER_NEAREST);
		equalizeHist(imgSamll, imgSamll);	// 直方图均衡化，提高图像质量
		pDlg->m_faces.clear();
		vector<Rect>().swap(pDlg->m_faces);
		if (!pDlg->faceCascade.empty()) {
			int flags = CASCADE_DO_CANNY_PRUNING |
						CASCADE_SCALE_IMAGE |
						CASCADE_FIND_BIGGEST_OBJECT |
						CASCADE_DO_ROUGH_SEARCH;
			pDlg->faceCascade.detectMultiScale(imgSamll, pDlg->m_faces, 2.0, 2, flags, Size(60, 60));    // 检测人脸
		}
		//clock_t dt = clock() - time1;
		//CString csTemp;
		//csTemp.Format("%d", dt);
		//AfxMessageBox(csTemp);
//*/
#endif

		int facex, facey;
		CvSize FaceImgSize;
		if (pDlg->m_faces.size()>0)
		{
			// 开启识别线程
			pDlg->startFdvThread();

			// 防止人脸框频繁抖动
			static Rect biggestface;
			if (abs(pDlg->m_faces[0].x - biggestface.x) < 5 && abs(pDlg->m_faces[0].y - biggestface.y) < 5
				&& abs((pDlg->m_faces[0].x + pDlg->m_faces[0].width) - (biggestface.x + biggestface.width)) < 5
				&& abs((pDlg->m_faces[0].y + pDlg->m_faces[0].height) - (biggestface.y + biggestface.height)) < 5) {
				pDlg->m_faces[0] = biggestface;
			}
			else
				biggestface = pDlg->m_faces[0];

			facex = (int)(pDlg->m_faces[0].x * scale - pDlg->m_faces[0].width * scale * 0.0);
			facey = (int)(pDlg->m_faces[0].y * scale - pDlg->m_faces[0].height * scale * 0.0);
			if (facex < 0) facex = 0;
			if (facey < 0) facey = 0;

			FaceImgSize.width = (int)(pDlg->m_faces[0].width * scale * 1.0);
			if (facex + FaceImgSize.width > 1280)
				FaceImgSize.width = 1280 - facex;
			FaceImgSize.height = (int)(pDlg->m_faces[0].height * scale * 1.0);
			if (facey + FaceImgSize.height > (720-0))
				FaceImgSize.height = (720-0) - facey;
		}


		
		if (pDlg->m_bCmdCapture && pDlg->m_faces.size()>0) {
			// 释放旧截图
			if (pDlg->m_CaptureImage) {
				cvReleaseImage(&(pDlg->m_CaptureImage));
				pDlg->m_CaptureImage = NULL;
			}
			if (pDlg->m_CaptureImageHide) {
				cvReleaseImage(&(pDlg->m_CaptureImageHide));
				pDlg->m_CaptureImageHide = NULL;
			}

			// 截出人脸图
			pDlg->m_CaptureImage = cvCreateImage(FaceImgSize, newframe->depth, newframe->nChannels);
			cvSetImageROI(newframe, cvRect(facex, facey, FaceImgSize.width, FaceImgSize.height));
			cvCopy(newframe, pDlg->m_CaptureImage);
			cvResetImageROI(newframe);
			//pDlg->m_CaptureImage = cvCloneImage(newframe);
			//string fn = pDlg->m_strModulePath + "frame0.png";
			//imwrite(fn.c_str(), cvarrToMat(pDlg->m_CaptureImage));

			if (captureHide.isOpened()) {
				//captureHide.read(cFrameHide);
				if (!cFrameHide.empty()) {
					IplImage* newframeHide = &IplImage(cFrameHide);
					pDlg->m_CaptureImageHide = cvCreateImage(FaceImgSize, newframeHide->depth, newframeHide->nChannels);
					cvSetImageROI(newframeHide, cvRect(facex, facey, FaceImgSize.width, FaceImgSize.height));
					cvCopy(newframeHide, pDlg->m_CaptureImageHide);
					cvResetImageROI(newframeHide);
					//pDlg->m_CaptureImageHide = cvCloneImage(&IplImage(cFrameHide));
					//string fn = pDlg->m_strModulePath + "frame1.png";
					//imwrite(fn.c_str(), cvarrToMat(pDlg->m_CaptureImageHide));
				}
			}

			pDlg->m_bCmdCapture = false;
			SetEvent(pDlg->m_eCaptureEnd);
		}

		// 画人脸框，需在截取人脸图后
		if (pDlg->m_faces.size() > 0) {
			//for (int i = 0; i<faces.size(); i++)
			{
				int i = 0;
				rectangle(cvarrToMat(newframe),
					Point(facex, facey),
					Point(facex + FaceImgSize.width, facey + FaceImgSize.height),
					Scalar(0, 255, 0), 2);    // 框出人脸
			}
		}
//*/
		//pDlg->drawCameraImage(pDlg->m_iplImgCameraImg);
		pDlg->showPreview(newframe);

		//Sleep(7);
		//		int c=cvWaitKey(33);   // not work in MFC proj

		//		if(c==27)break;
	}

	captureMain.release();
	captureHide.release();

	SetEvent(pDlg->m_eCameraEnd);
	return 0;
}

// capture thread
UINT FdvThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;
	//pDlg->m_bFdvRun = true;

	try {
		char szPath[1024] = { 0 };
		GetModuleFileName(NULL, szPath, MAX_PATH);
		std::string strModulePath = ExtractFilePath(szPath);


#if OPENCV_CAPTURE
		WaitForSingleObject(pDlg->m_eCaptureEnd, INFINITE);
		pDlg->m_bIsAliveSample = true;
#else
		// 获取摄像头人脸
		std::string facedatastr;
		Base64::Decode(pDlg->m_sCaptureBase64, &facedatastr);
		std::vector<uchar> facebuff(facedatastr.begin(), facedatastr.end());
		cv::Mat facemat = cv::imdecode(facebuff, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		IplImage imgTmp = facemat;
		pDlg->m_CaptureImage = cvCloneImage(&imgTmp);
#endif

		// read idcard
		int iopen = OpenIDCardReader();
		if (iopen < 0) {
			pDlg->m_bFdvRun = false;
			SetEvent(pDlg->m_eFdvEnd);
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
			{
				if (pDlg->m_iplImgPhoto)
					cvReleaseImage(&(pDlg->m_iplImgPhoto));	// 先释放

				vector<char>  vcBuf;
				vcBuf.insert(vcBuf.end(), pDlg->m_IdCardPhoto, pDlg->m_IdCardPhoto + lenbmp);
				Mat matphoto = cv::imdecode(vcBuf, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
				IplImage imgTmp = matphoto;

				pDlg->m_iplImgPhoto = cvCloneImage(&imgTmp);
				//pDlg->m_iplImgPhoto = BMP2Ipl((unsigned char*)pDlg->m_IdCardPhoto, lenbmp);
			}

			// mat
			Mat matframe = cvarrToMat(pDlg->m_CaptureImage);
			//Mat matframe(pDlg->m_iplImgTestImage, false);
			Mat matphoto = cvarrToMat(pDlg->m_iplImgPhoto);

#if OPENCV_CAPTURE
			Mat matframehide = cvarrToMat(pDlg->m_CaptureImageHide);
#ifdef NDEBUG
			// Ai_Fdr_SC 活体检测
			if (pDlg->m_CaptureImageHide) {
				std::vector<int> tmpfacerect;
				std::string tmpfacefeat;

				//clock_t time1 = clock();
				
				
				bool live = pDlg->m_pfrmwrap->livecheck(matframe, matframehide, tmpfacerect, tmpfacefeat);
				//clock_t dt = clock() - time1;
				//CString csTemp;
				//csTemp.Format("%d", dt);
				//AfxMessageBox(csTemp);
				/*
				CString csTemp;
				static int lcscnt = 316;
				csTemp.Format("lcs%04d", lcscnt);
				if (live) {
					string fn = pDlg->m_strModulePath + csTemp.GetString() + "_0_t.png";
					cv::imwrite(fn.c_str(), matframe);
					fn = pDlg->m_strModulePath + csTemp.GetString() + "_1_t.png";
					cv::imwrite(fn.c_str(), matframehide);
				}
				else {
					string fn = pDlg->m_strModulePath + csTemp.GetString() + "_0.png";
					cv::imwrite(fn.c_str(), matframe);
					fn = pDlg->m_strModulePath + csTemp.GetString() + "_1.png";
					cv::imwrite(fn.c_str(), matframehide);
				}
				lcscnt++;
				//*/
				pDlg->m_bIsAliveSample = live;

				pDlg->m_frameFaceFeats.clear();
				pDlg->m_frameFaceFeats.shrink_to_fit();
				pDlg->m_frameFaceFeats.push_back(tmpfacefeat);
			}
#endif
#endif

			// 显示
			pDlg->m_pInfoDlg->clearResultIcon();  // 验证结果图标清空
			pDlg->m_pInfoDlg->drawCameraImage(pDlg->m_CaptureImage);
			pDlg->m_pInfoDlg->drawIdcardImage(pDlg->m_iplImgPhoto);
			if (false == pDlg->m_bIsAliveSample) {
				// 非活体警告
				pDlg->m_pInfoDlg->setResultTextSize(60);
				pDlg->m_pInfoDlg->setResultText("非活体！");
				pDlg->setClearTimer();
				pDlg->m_bFdvRun = false;
				SetEvent(pDlg->m_eFdvEnd);
				return 0;
			}
			pDlg->m_pInfoDlg->setResultTextSize(60);
			pDlg->m_pInfoDlg->setResultText("--%");

			//std::vector<uchar> idcardPhoto;
			std::vector<uchar> idcardPhoto(pDlg->m_IdCardPhoto, pDlg->m_IdCardPhoto + lenbmp);
			std::vector<uchar> verifyPhotos[1];
#if OPENCV_CAPTURE
#ifdef NDEBUG
#else
			// debug
			verifyPhotos[0] = vector<uchar>(256000);
			vector<int> param = vector<int>(2);
			cv::imencode(".png", matframe, verifyPhotos[0], param);
#endif

#else
			verifyPhotos[0] = facebuff;
#endif
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

			
#ifdef NDEBUG
			//*	
			std::vector < std::vector<int>> face_rects, face_rects2;

			int photo_face_cnt = pDlg->m_pfrmwrap->dectect_faces(matphoto, face_rects, 1, true);
			if (photo_face_cnt < 1) {
				face_rects.push_back({ 10,0,92,110 }); // l t r b
				//face_rects.push_back({ 0,0,102,126 });
			}
			pDlg->m_photoFaceFeat.clear();
			pDlg->m_photoFaceFeat.shrink_to_fit();
			pDlg->m_pfrmwrap->ai_fdr_edge_descriptor(matphoto, face_rects[0], pDlg->m_photoFaceFeat);

			if (!pDlg->m_CaptureImageHide) {
				// 由于活体检测时可得到feat，只在没做活体检测情况下取feat
				pDlg->m_frameFaceFeats.clear();
				pDlg->m_frameFaceFeats.shrink_to_fit();
				int frame_face_cnt = pDlg->m_pfrmwrap->ai_fdr_edge_descriptors(matframe, face_rects2, pDlg->m_frameFaceFeats, 0, true);
			}

			//std::ofstream logfile(strModulePath + "des_photo.txt");
			//logfile << pDlg->m_photoFaceFeat;
			//logfile.close();
			//std::ofstream logfile2(strModulePath + "des_frame.txt");
			//for (auto ltrb : pDlg->m_frameFaceFeats)
			//	logfile2 << ltrb << endl;
			//logfile2.close();

			time_net = clock();
			MTLibCallVerify(pDlg->m_cfgUrl,
				pDlg->m_cfgAppId, pDlg->m_cfgApiKey, pDlg->m_cfgSecretKey, uuid,
				pDlg->m_macId, pDlg->m_cfgRegisteredNo,
				pDlg->m_IdCardId, pDlg->m_IdCardIssuedate, pDlg->m_photoFaceFeat, pDlg->m_frameFaceFeats, 1,
				VerifyCB, (MTLIBPTR)pDlg, ::stoi(pDlg->m_cfgTimeOut));
			//*/
#else
			//*
			MTLibCallVerify_Image("http://192.168.1.201:8004/idcardfdv",//pDlg->m_cfgUrl,
				pDlg->m_cfgAppId, pDlg->m_cfgApiKey, pDlg->m_cfgSecretKey, uuid,
				pDlg->m_macId, pDlg->m_cfgRegisteredNo,
				pDlg->m_IdCardId, pDlg->m_IdCardIssuedate, idcardPhoto, verifyPhotos, 1,
				VerifyCB, (MTLIBPTR)pDlg, ::stoi(pDlg->m_cfgTimeOut));
			/**/
#endif

		}
		pDlg->m_bFdvRun = false;
		SetEvent(pDlg->m_eFdvEnd);
	}
	catch (std::exception e) {
		pDlg->m_bFdvRun = false;
		SetEvent(pDlg->m_eFdvEnd);
	}
	
	return 0;
}

void CIDCardFdvDlg::setClearTimer()
{
	KillTimer(CLEAR_INFOIMG_TIMER);
	SetTimer(CLEAR_INFOIMG_TIMER, 1000 * 10, NULL);
}


BOOL CIDCardFdvDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class

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

	if (m_iplImgTestImage != NULL) {
		cvReleaseImage(&m_iplImgTestImage);
		m_iplImgTestImage = NULL;
	}
	if (m_iplImgTestImage2 != NULL) {
		cvReleaseImage(&m_iplImgTestImage2);
		m_iplImgTestImage2 = NULL;
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

	if (m_CaptureImageHide != NULL) {
		cvReleaseImage(&m_CaptureImageHide);
		m_CaptureImageHide = NULL;
	}

	if (m_iplImgPhoto != NULL) {
		cvReleaseImage(&m_iplImgPhoto);
		m_iplImgPhoto = NULL;
	}

	if (m_pInfoDlg) {
		delete m_pInfoDlg;
		m_pInfoDlg = NULL;
	}

#ifdef NDEBUG
	if (m_pfrmwrap) {
		delete m_pfrmwrap;
		m_pfrmwrap = NULL;
	}
#endif

	return CDialogEx::DestroyWindow();
}

void CIDCardFdvDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (true == m_bMainWinClose)
		return;
	m_bMainWinClose = true;	// 关闭主窗口->全部退出，只允许处理一次

#if OPENCV_CAPTURE
	stopFaceDetectThread(); // 停止预览线程前先停止检测
	stopCameraThread();
	Sleep(20);
#else
	m_pInfoDlg->ShowWindow(SW_HIDE);
	MTLibCloseCamera();
	MTLibUnloadCamera();
#endif
	waitFdvThreadStopped();

	// IDCardReader
	UnloadIDCardReader();

	CDialogEx::OnClose();
}




void CIDCardFdvDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// test code
#if OPENCV_CAPTURE
//	startFdvThread();
#endif
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
