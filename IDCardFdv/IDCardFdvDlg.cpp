
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
clock_t time_global = 0;
clock_t time_global2 = 0;

CCriticalSection g_CriticalSection;
CCriticalSection g_CriticalSectionAiFdr;
CCriticalSection g_CriticalSectionIdCard;
UINT DataLoadingThread(LPVOID lpParam);
UINT IdcardDetectThread(LPVOID lpParam);
UINT IdcardReadThread(LPVOID lpParam);
UINT FaceDetectThread(LPVOID lpParam);
UINT CameraShowThread(LPVOID lpParam);
UINT FdvThread(LPVOID lpParam);
UINT ImgUploadThread(LPVOID lpParam);

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
static void __stdcall VerifyCB(int err_no, std::string err_msg, double similarity, std::string serial_no, MTLIBPTR userdata)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)userdata;

	if (MTLIBNETWORK_NETWORK_ERROR == err_no) {
		// 超时或其他网络错误处理
#ifdef NDEBUG
		double sim = 0.0;
		g_CriticalSectionAiFdr.Lock();
		int simret = pDlg->m_pfrmwrap->ai_fdr_similarity(pDlg->m_photoFaceFeat, pDlg->m_frameFaceFeats[0], sim);
		g_CriticalSectionAiFdr.Unlock();
		VerifyCB(0, "", sim, serial_no, userdata);
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
		//pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconRight);
		pDlg->m_bDrawResultIconRight = true;
		alSourcePlay(pDlg->m_sndRightSource);
		ALint state;
		do {
			alGetSourcei(pDlg->m_sndRightSource, AL_SOURCE_STATE, &state);
		} while (state == AL_PLAYING);
	}
	else if (retsim >= 0.0) {
		pDlg->m_pInfoDlg->setResultTextSize(60);
		pDlg->m_pInfoDlg->setResultText(retstr + "%");
		//pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconWrong);
		pDlg->m_bDrawResultIconWrong = true;
		alSourcePlay(pDlg->m_sndWrongSource);
		ALint state;
		do {
			alGetSourcei(pDlg->m_sndWrongSource, AL_SOURCE_STATE, &state);
		} while (state == AL_PLAYING);
	}
	else {
		// error
		pDlg->m_pInfoDlg->setResultTextSize(20);
		pDlg->m_pInfoDlg->setResultText(err_msg);
		//pDlg->m_pInfoDlg->drawResultIcon(pDlg->m_iplImgResultIconWrong);
		pDlg->m_bDrawResultIconWrong = true;
		alSourcePlay(pDlg->m_sndWrongSource);
		ALint state;
		do {
			alGetSourcei(pDlg->m_sndWrongSource, AL_SOURCE_STATE, &state);
		} while (state == AL_PLAYING);
	}

	pDlg->m_bFirstFdv = false;
	pDlg->setClearTimer();


	// 对比图插入上传队列
	if (serial_no != "") {
		bool bInsert = pDlg->m_imgUploadMgt->insert(serial_no, 
								pDlg->m_iplImgUploadCopyFrame, 
								pDlg->m_iplImgUploadCopyFrameHide,
								pDlg->m_iplImgUploadCopyPhoto,
								pDlg->m_IdCardId, 
								pDlg->m_IdCardIssuedate);
		if (bInsert) {
			pDlg->m_iplImgUploadCopyFrame = NULL;
			pDlg->m_iplImgUploadCopyFrameHide = NULL;
			pDlg->m_iplImgUploadCopyPhoto = NULL;
		}
	}

	// 重启数据上传 
	g_CriticalSection.Lock();
	pDlg->m_bImgUploadPause = false;
	g_CriticalSection.Unlock();
	SetEvent(pDlg->m_eImgUploadResume);

#if DEBUG_LOG_FILE
	g_CriticalSection.Lock();
	pDlg->m_logfile << "end call Verify! " << clock() - time_global << "ms" << endl;
	time_global = clock();
	pDlg->m_logfile << "total time: " << clock() - time_global2 << "ms" << endl;
	g_CriticalSection.Unlock();
#endif

	clock_t dt = clock() - time_net;
	CString csTemp1;
	csTemp1.Format("%d", dt);
//	AfxMessageBox(csTemp1);
	time_net = 0;
}

// CIDCardFdvDlg 对话框
CIDCardFdvDlg::CIDCardFdvDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IDCARDFDV_DIALOG, pParent), 
	m_eCameraResume(FALSE,TRUE,0,0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// camera preview
	//camdevid = 0;
	m_bCameraRun = false;
	m_thCamera = NULL;
	ResetEvent(m_eCameraEnd);
	m_iMainDevIdx = -1;
	m_iHideDevIdx = -1;
	m_iplImgDisplay = NULL;
	m_iplImgTemp = NULL;
	m_bFlip = false;
	m_iPreviewX = 0;
	m_bDrawScan = true;
	m_bMainFrameSuccess = false;
	m_bHideFrameSuccess = false;

	// help scene and idcard detect
	m_bIdcardDetectRun = false;
	m_thIdcardDetect = NULL;
	ResetEvent(m_eIdcardDetectEnd);
	m_iplImgHelpImg = NULL;

	// idcard read and get feature thread
	m_bIdcardReadRun = false;
	m_thIdcardRead = NULL;
	ResetEvent(m_eIdcardReadEnd);

	m_bFaceDetectRun = false;
	m_thFaceDetect = NULL;
	ResetEvent(m_eFaceDetectEnd);
	m_bCmdDetect = false;
	m_faces.clear();
	m_iplImgCameraImg = NULL;
	m_iplImgCameraImgHide = NULL;

	m_bImgUploadRun = false;
	m_thImgUpload = NULL;
	ResetEvent(m_eImgUploadEnd);
	m_imgUploadMgt = NULL;
	m_iplImgUploadCopyFrame = NULL;
	m_iplImgUploadCopyFrameHide = NULL;
	m_iplImgUploadCopyPhoto = NULL;

	m_CaptureImage = NULL;
	m_CaptureImageHide = NULL;
	m_bFaceGot = false;
	m_bIsAliveSample = false;
	m_bFdvRun = false;
	m_bCmdFdvStop = false;
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
	m_bIdCardNoChange = false;

	m_pAttentionDlg = NULL;

	m_pInfoDlg = NULL;

	m_hBIconCamera = NULL;
	m_bDrawResultIconRight = false;
	m_bDrawResultIconWrong = false;
	m_dThreshold = 77.0;
}

CIDCardFdvDlg::~CIDCardFdvDlg() 
{
	if (!alutExit()) {
		fprintf(stderr, "> ALUT error: %s\n",
			alutGetErrorString(alutGetError()));
	}

#if DEBUG_LOG_FILE
	m_logfile.close();
#endif
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
	//char sbuf[1024];
	//GetSystemDirectory(sbuf,1024);

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
	m_iPreviewWidth = rw;
	m_iPreviewHeight = rh;

	// 创建提醒信息窗口
	ClientToScreen(&rect);
	float ADRATE = 0.4f;
	rw = (int)((rect.right - rect.left) * ADRATE); // 信息窗口宽
	rh = rect.bottom - rect.top;					// 信息窗口高
	offsetX = rect.left;
	offsetY = rect.top;
	if (NULL == m_pAttentionDlg) {
		m_pAttentionDlg = new CAttentionDlg(offsetX, offsetY, rw, rh);
		m_pAttentionDlg->Create(IDD_ATTENTION_DIALOG, this);
		m_pAttentionDlg->setFontRate(m_iPreviewWidth * 1.0f / 1366);
	}
	m_pAttentionDlg->setVisible(false);
	m_iPreviewX = offsetX + rw / 2;

	// 创建信息窗口
//	ClientToScreen(&rect);
	float IPRATE = 0.15f;
	rw = (int)((rect.right - rect.left) * IPRATE); // 信息窗口宽
	rh = rect.bottom - rect.top;					// 信息窗口高
	offsetX = rect.left + (int)((rect.right - rect.left) * (1 - IPRATE));
	offsetY = rect.top;
	if (NULL == m_pInfoDlg) {
		m_pInfoDlg = new CInfoDlg(offsetX, offsetY, rw, rh);
		m_pInfoDlg->Create(IDD_INFO_DIALOG, this);
	}
	m_pInfoDlg->ShowWindow(SW_HIDE);//(SW_SHOW);

	/*
	m_hBIconCamera = (HBITMAP)LoadImage(AfxGetInstanceHandle(),
	MAKEINTRESOURCE(IDB_ICON_CAMERA),
	IMAGE_BITMAP, 0, 0,
	LR_LOADMAP3DCOLORS);
	CStatic* m_cs = (CStatic*)GetDlgItem(IDC_CAMERA_IMAGE);
	m_cs->SetBitmap(m_hBIconCamera);
	*/

	// 帮助画面单独先读入
	std::string fn;
	fn = m_strModulePath + "help.jpg";
	m_iplImgHelpImg = cvLoadImage(fn.c_str(), -1);

	/*
	CFile file;
	fn = m_strModulePath + "right.wav";
	if (file.Open(fn.c_str(), CFile::modeRead) && 
		file.GetLength() < sizeof(m_sndRight)) {
		file.Read(m_sndRight, (UINT)file.GetLength());
		file.Close();
	}
	fn = m_strModulePath + "wrong.wav";
	if (file.Open(fn.c_str(), CFile::modeRead) &&
		file.GetLength() < sizeof(m_sndWrong)) {
		file.Read(m_sndWrong, (UINT)file.GetLength());
		file.Close();
	}*/

	

#if NDEBUG

	/* livecheck test
	std::vector < std::vector<int>> face_rects;
	std::vector<int> tmpfacerect;
	std::string tmpfacefeat;
	string tmpfn = m_strModulePath + "dump/lcs20181102100405323_0.png";
	Mat matframe = imread(tmpfn.c_str(), CV_LOAD_IMAGE_UNCHANGED);
	cvtColor(matframe, matframe, cv::COLOR_BGR2RGB);
	tmpfn = m_strModulePath + "dump/lcs20181102100405323_1.png";
	Mat matframehide = imread(tmpfn.c_str(), CV_LOAD_IMAGE_UNCHANGED);
	cvtColor(matframehide, matframehide, cv::COLOR_BGR2RGB);

	bool live = m_pfrmwrap->livecheck(matframe, matframehide, tmpfacerect, tmpfacefeat);
	//*/
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
	m_cfgUploadUrl = "http://192.168.1.201:8008/idfdv_complete";
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
				if (key == "uploadurl")
					m_cfgUploadUrl = value;
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
	startDataLoadingThread();
	startIdcardDetectThread();
	startCameraThread();
	startIdcardReadThread();
	startFaceDetectThread();
	startImgUploadThread();
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

#if DEBUG_LOG_FILE
	m_logfile.open(m_strModulePath + "log.txt");
#endif

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

void CIDCardFdvDlg::showPreview(IplImage* img, int dX)	
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
	
	
	int displayX = dX;
	int displayW = rw;
	int displayH = rw * ih / iw;
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

void CIDCardFdvDlg::drawHelpImage(IplImage* img)
{
	UINT ID = IDC_PREVIEW_IMG;

	CDC* pDC = GetDlgItem(ID)->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作

	CRect rect;

	GetDlgItem(ID)->GetClientRect(&rect);
	int rw = rect.right - rect.left;			// 求出图片控件的宽和高
	int rh = rect.bottom - rect.top;

	SetRect(rect, 0, 0, rw, rh);	// 铺满控件

	if (NULL == img) {
		pDC->FillSolidRect(rect, RGB(255, 255, 255));
	}
	else {
		CvvImage cimg;
		cimg.CopyOf(img, -1);							// 复制图片
		cimg.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内
	}

	ReleaseDC(pDC);
}

void CIDCardFdvDlg::drawScanRect(cv::Mat frame)
{
	cv::Rect info_dlg_rect, camera_img_rect;
	m_pInfoDlg->getInfoDlgScreenRect(info_dlg_rect);
	info_dlg_rect.x -= m_iPreviewX;
	m_pInfoDlg->getCameraImageRect(camera_img_rect);

	int sMin = camera_img_rect.y;
	sMin = sMin * frame.rows / m_iPreviewHeight;
	int sMax = camera_img_rect.y + camera_img_rect.height;
	sMax = sMax * frame.rows / m_iPreviewHeight - m_MatScan.rows;
	int scanX = info_dlg_rect.x + camera_img_rect.x;
	scanX = scanX * frame.cols / m_iPreviewWidth;
	scanX = scanX + (camera_img_rect.width * frame.cols / m_iPreviewWidth - m_MatScan.cols) / 2; // 居中
	static int scanY = sMax;
	scanY = ((scanY - sMin) + 10) % (sMax - sMin + 1) + sMin;
	Mat newframe_scan(frame, cvRect(scanX, scanY, m_MatScan.cols, m_MatScan.rows));
	MatAlphaBlend(newframe_scan, m_MatScan);

	int b_x = scanX - 0;
	Mat newframe_btl(frame, cvRect(b_x, sMin, m_MatScanBorder.cols, m_MatScanBorder.rows));
	MatAlphaBlend(newframe_btl, m_MatScanBorder);
	Mat bbl;
	cv::flip(m_MatScanBorder, bbl, 0);
	int b_y = sMax + m_MatScan.rows - bbl.rows;
	Mat newframe_bbl(frame, cvRect(b_x, b_y, bbl.cols, bbl.rows));
	MatAlphaBlend(newframe_bbl, bbl);
	b_x = scanX + m_MatScan.cols + 0 - m_MatScanBorder.cols;
	Mat btr;
	cv::flip(m_MatScanBorder, btr, 1);
	Mat newframe_btr(frame, cvRect(b_x, sMin, btr.cols, btr.rows));
	MatAlphaBlend(newframe_btr, btr);
	Mat bbr;
	cv::flip(m_MatScanBorder, bbr, -1);
	Mat newframe_bbr(frame, cvRect(b_x, b_y, btr.cols, btr.rows));
	MatAlphaBlend(newframe_bbr, bbr);
}

void CIDCardFdvDlg::drawResultIcon(cv::Mat frame, cv::Mat icon)
{
	cv::Rect info_dlg_rect, result_icon_rect;
	m_pInfoDlg->getInfoDlgScreenRect(info_dlg_rect);
	info_dlg_rect.x -= m_iPreviewX;
	m_pInfoDlg->getResultIconRect(result_icon_rect);

	int sX = info_dlg_rect.x + result_icon_rect.x;
	sX = sX * frame.cols / m_iPreviewWidth;
	sX = sX + (result_icon_rect.width * frame.cols / m_iPreviewWidth - icon.cols) / 2; // 居中
	int sY = result_icon_rect.y;
	sY = sY * frame.rows / m_iPreviewHeight;

	Mat newframe(frame, cvRect(sX, sY, icon.cols, icon.rows));
	MatAlphaBlend(newframe, icon);
}

void CIDCardFdvDlg::startDataLoadingThread()
{
	m_bDataReady = false;
	m_bStopLoading = false;
	m_bsndReady = false;
	ResetEvent(m_eDataloadingEnd);
	if (m_thDataLoading == NULL) {
		m_thDataLoading = AfxBeginThread(DataLoadingThread, this);
		if (NULL == m_thDataLoading)
		{
			TRACE("创建新的线程出错！\n");
			return;
		}
	}
}

void CIDCardFdvDlg::stopDataLoadingThread()
{
	g_CriticalSection.Lock();
	m_bStopLoading = true;
	g_CriticalSection.Unlock();

	if(m_thDataLoading)
		WaitObjectAndMsg(m_eDataloadingEnd, INFINITE);
}

void CIDCardFdvDlg::startIdcardDetectThread()
{
	ResetEvent(m_eIdcardDetectEnd);

	if (m_thIdcardDetect == NULL) {
		m_thIdcardDetect = AfxBeginThread(IdcardDetectThread, this);
		if (NULL == m_thIdcardDetect)
		{
			TRACE("创建新的线程出错！\n");
			return;
		}
	}
}

void CIDCardFdvDlg::stopIdcardDetectThread()
{
	if (m_thIdcardDetect && m_bIdcardDetectRun) {
		g_CriticalSection.Lock();
		m_bIdcardDetectRun = false;
		g_CriticalSection.Unlock();
		SetEvent(m_eIdcardDetectResume);
		WaitObjectAndMsg(m_eIdcardDetectEnd, INFINITE);
		ResetEvent(m_eIdcardDetectEnd);

		m_thIdcardDetect = NULL;
	}
}

void CIDCardFdvDlg::startIdcardReadThread()
{
	ResetEvent(m_eIdcardReadEnd);

	if (m_thIdcardRead == NULL) {
		m_thIdcardRead = AfxBeginThread(IdcardReadThread, this);
		if (NULL == m_thIdcardRead)
		{
			TRACE("创建新的线程出错！\n");
			return;
		}
	}
}

void CIDCardFdvDlg::stopIdcardReadThread()
{
	if (m_thIdcardRead && m_bIdcardReadRun) {
		g_CriticalSection.Lock();
		m_bIdcardReadRun = false;
		g_CriticalSection.Unlock();
		SetEvent(m_eIdcardReadResume);
		WaitObjectAndMsg(m_eIdcardReadEnd, INFINITE);
		ResetEvent(m_eIdcardReadEnd);

		m_thIdcardRead = NULL;
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

		SetEvent(m_eCameraResume);
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
		SetEvent(m_eCaptureForDetect); // 释放截图等待
		SetEvent(m_eGetIdCardFeat);	   // 释放提取feat等待
		SetEvent(m_eFaceDetectResume); // 释放检测结束等待
		WaitObjectAndMsg(m_eFaceDetectEnd, INFINITE);
		ResetEvent(m_eFaceDetectEnd);

		m_thFaceDetect = NULL;
	}
}


bool CIDCardFdvDlg::startFdvThread(std::string feat, bool live)
{
#if OPENCV_CAPTURE
	if (!m_bCameraRun)
		return false;
#else
	if (!m_bFaceGot)
		return;
#endif

	if (m_bFdvRun)
		return false;

	//if(m_bFirstFdv && !m_bFirstFdvIdcardReaded)
	//	return false;
	if (m_bFirstFdv) {
		while (!m_bFirstFdvIdcardReaded) { // wait until idcard photo ready
			Sleep(10);
		}
	}

#if OPENCV_CAPTURE
	m_frameFaceFeats.clear();
	m_frameFaceFeats.shrink_to_fit();
	m_frameFaceFeats.push_back(feat);
	m_bIsAliveSample = live;
	if(NULL == m_iplImgUploadCopyFrame)
		m_iplImgUploadCopyFrame = cvCloneImage(m_iplImgCameraImg);
	if (NULL == m_iplImgUploadCopyFrameHide)
		m_iplImgUploadCopyFrameHide = cvCloneImage(m_iplImgCameraImgHide);
	

	// 截出人脸
	int facex, facey;
	CvSize FaceImgSize;
	double scale = 1.0;
	if (m_faces.size()>0)
	{
		facex = (int)(m_faces[0].x * scale - m_faces[0].width * scale * 0.0);
		facey = (int)(m_faces[0].y * scale - m_faces[0].height * scale * 0.0);
		if (facex < 0) facex = 0;
		if (facey < 0) facey = 0;

		FaceImgSize.width = (int)(m_faces[0].width * scale * 1.0);
		if (facex + FaceImgSize.width > 1280)
			FaceImgSize.width = 1280 - facex;
		FaceImgSize.height = (int)(m_faces[0].height * scale * 1.0);
		if (facey + FaceImgSize.height >(720 - 0))
			FaceImgSize.height = (720 - 0) - facey;

		// 释放旧截图
		if (m_CaptureImage) {
			cvReleaseImage(&(m_CaptureImage));
			m_CaptureImage = NULL;
		}

		// 截出人脸图
		m_CaptureImage = cvCreateImage(FaceImgSize, m_iplImgCameraImg->depth, m_iplImgCameraImg->nChannels);
		cvSetImageROI(m_iplImgCameraImg, cvRect(facex, facey, FaceImgSize.width, FaceImgSize.height));
		cvCopy(m_iplImgCameraImg, m_CaptureImage);
		cvResetImageROI(m_iplImgCameraImg);
		//pDlg->m_CaptureImage = cvCloneImage(newframe);
		//string fn = m_strModulePath + "face0.png";
		//imwrite(fn.c_str(), cvarrToMat(m_CaptureImage));
	}
#endif
	m_bFdvRun = true;
	ResetEvent(m_eFdvEnd);
	m_thFdv = AfxBeginThread(FdvThread, this);

	if (NULL == m_thFdv)
	{
		TRACE("创建新的线程出错！\n");
		m_bFdvRun = false;
		return false;
	}

	return true;
}

void CIDCardFdvDlg::waitFdvThreadStopped()
{
	if (m_thFdv && m_bFdvRun) {
		m_bCmdFdvStop = true;
		WaitObjectAndMsg(m_eFdvEnd, INFINITE);
		ResetEvent(m_eFdvEnd);
		m_bCmdFdvStop = false;
		m_thFdv = NULL;
	}	
}

void CIDCardFdvDlg::startImgUploadThread()
{
	ResetEvent(m_eImgUploadEnd);

	if (m_thImgUpload == NULL) {
		m_thImgUpload = AfxBeginThread(ImgUploadThread, this);
		if (NULL == m_thImgUpload)
		{
			TRACE("创建新的线程出错！\n");
			return;
		}
	}
}

void CIDCardFdvDlg::stopImgUploadThread()
{
	if (m_thImgUpload && m_bImgUploadRun) {
		g_CriticalSection.Lock();
		m_bImgUploadRun = false;
		g_CriticalSection.Unlock();
		SetEvent(m_eImgUploadResume);	// 释放线程内等待
		WaitObjectAndMsg(m_eImgUploadEnd, INFINITE);
		ResetEvent(m_eImgUploadEnd);

		m_thImgUpload = NULL;
	}
}

// data loading
UINT DataLoadingThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	while (1) {
		// 图
		pDlg->m_pAttentionDlg->loadAllData();
		if (pDlg->m_bStopLoading) break;

		std::string fn = pDlg->m_strModulePath + "right.png";
		pDlg->m_ResultIconRight = imread(fn.c_str(), CV_LOAD_IMAGE_UNCHANGED);
		if (pDlg->m_bStopLoading) break;

		fn = pDlg->m_strModulePath + "wrong.png";
		pDlg->m_ResultIconWrong = imread(fn.c_str(), CV_LOAD_IMAGE_UNCHANGED);
		if (pDlg->m_bStopLoading) break;

		fn = pDlg->m_strModulePath + "scan00.png";
		pDlg->m_MatScan = imread(fn.c_str(), CV_LOAD_IMAGE_UNCHANGED);
		if (pDlg->m_bStopLoading) break;

		fn = pDlg->m_strModulePath + "scan01.png";
		pDlg->m_MatScanBorder = imread(fn.c_str(), CV_LOAD_IMAGE_UNCHANGED);
		if (pDlg->m_bStopLoading) break;

		// openal,alut以及声音准备
		//*
		alutInit(NULL, NULL);
		fn = pDlg->m_strModulePath + "right.wav";
		pDlg->m_sndRightBuffer = alutCreateBufferFromFile(fn.c_str());
		alGenSources(1, &pDlg->m_sndRightSource);
		alSourcei(pDlg->m_sndRightSource, AL_BUFFER, pDlg->m_sndRightBuffer);

		fn = pDlg->m_strModulePath + "wrong.wav";
		pDlg->m_sndWrongBuffer = alutCreateBufferFromFile(fn.c_str());
		alGenSources(1, &pDlg->m_sndWrongSource);
		alSourcei(pDlg->m_sndWrongSource, AL_BUFFER, pDlg->m_sndWrongBuffer);
		pDlg->m_bsndReady = true;
		if (pDlg->m_bStopLoading) break;
		//*/

		// IDCardReader
		LoadIDCardReader();
		if (pDlg->m_bStopLoading) break;

		// Mac ID
		char mac[64];
		GetLocalMAC(mac, sizeof(mac));
		pDlg->m_macId = mac;
		if (pDlg->m_bStopLoading) break;

		std::string modelpath = pDlg->m_strModulePath;
		pDlg->m_pfrmwrap = new fdr_model_wrap(modelpath);
		if (pDlg->m_bStopLoading) break;

		// 进行一次网络测试以提高后面速度。	
		Sleep(10);
		MTLibTestUrlIgnoreResp(pDlg->m_cfgUrl, NULL, (MTLIBPTR)pDlg, ::stoi(pDlg->m_cfgTimeOut));

		g_CriticalSection.Lock();
		pDlg->m_bDataReady = true;
		g_CriticalSection.Unlock();

		break;
	}

	SetEvent(pDlg->m_eDataloadingEnd);
	return 0;
}

// idcard detect thread
UINT IdcardDetectThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	g_CriticalSection.Lock();
	pDlg->m_bIdcardDetectRun = true;
	g_CriticalSection.Unlock();

	while (!pDlg->m_bDataReady) {
		pDlg->drawHelpImage(pDlg->m_iplImgHelpImg);
		Sleep(100);
	}

	int times_per_sec = 2;
	clock_t time_s, detect_dt;
	int iopen = OpenIDCardReader();
	if (iopen < 0) {
		AfxMessageBox("身份证读卡器连接失败！");
		g_CriticalSection.Lock();
		pDlg->m_bIdcardDetectRun = false;
		g_CriticalSection.Unlock();
	}

	bool close_att_dlg = false;
	int camera_delay_cnt = 4;	// 显示帮忙画面后再打开摄像头
	while (pDlg->m_bIdcardDetectRun)
	{
		time_s = clock();

		int check = Authenticate_Idcard(); //Authenticate_Content(2);
		
		if (check > 0) {		
			//pDlg->drawHelpImage(NULL); // clear
			//pDlg->m_pAttentionDlg->setVisible(true);
			//UpdateWindow(pDlg->m_pAttentionDlg->m_hWnd);

			g_CriticalSection.Lock();
			pDlg->m_bImgUploadPause = true; // 暂停数据上传
			pDlg->m_bFirstFdv = true;		// 第一次预读优化
			pDlg->m_bFirstFdvIdcardReaded = false;
			g_CriticalSection.Unlock();

			ResetEvent(pDlg->m_eGetIdCardFeat);
			SetEvent(pDlg->m_eCameraResume);//pDlg->startCameraThread();
			pDlg->idcardPreRead();			// 数据预读
			pDlg->m_pInfoDlg->ShowWindow(SW_SHOW);
			pDlg->setClearTimer(20);

			ResetEvent(pDlg->m_eIdcardDetectResume);
			WaitForSingleObject(pDlg->m_eIdcardDetectResume, INFINITE);
			if (!pDlg->m_bIdcardDetectRun)
				break;	// 已退出
			close_att_dlg = true;
		}
		else if (-5 == check) {
			pDlg->drawHelpImage(pDlg->m_iplImgHelpImg);
			CloseIDCardReader();
			int iopen = OpenIDCardReader();
			if (iopen < 0)
				AfxMessageBox("身份证读卡器连接失败！");
		}
		else {
			pDlg->drawHelpImage(pDlg->m_iplImgHelpImg);
			if (close_att_dlg) {
				pDlg->m_pAttentionDlg->setVisible(false);
				close_att_dlg = false;
			}
		}

		// 打开并监控摄像头
		if (camera_delay_cnt <= 0) {
			pDlg->checkAndOpenAllCamera();	// 在帮忙画面循环检查
		}
		else
			camera_delay_cnt--;

		detect_dt = clock() - time_s;
		if (detect_dt < 1000 / times_per_sec) {
			Sleep(1000 / times_per_sec - detect_dt);
		}
	}

	CloseIDCardReader();
	SetEvent(pDlg->m_eIdcardDetectEnd);
	return 0;
}

// idcard read and get feature thread
UINT IdcardReadThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	g_CriticalSection.Lock();
	pDlg->m_bIdcardReadRun = true;
	g_CriticalSection.Unlock();

	while (pDlg->m_bIdcardReadRun)
	{
		// 等待读卡请求
		ResetEvent(pDlg->m_eIdcardReadResume);
		WaitForSingleObject(pDlg->m_eIdcardReadResume, INFINITE);

		if (pDlg->m_bFirstFdv) {
			SetEvent(pDlg->m_eIdcardReadDone);
			continue;
		}

		g_CriticalSectionIdCard.Lock();
		Authenticate_Idcard();
		int check = Authenticate_Content();
		g_CriticalSectionIdCard.Unlock();
		if (check > 0) {
			pDlg->m_bIdcardReadFindCard = true;
			// 获取身份证号
			char idcardid[256];
			long len = IDCardReader_GetPeopleIDCode(idcardid, sizeof(idcardid));
			if (strcmp(pDlg->m_IdCardId, idcardid) == 0) {
				// 身份证无变化
			}
			else {
				// 获取身份证号
				long len = IDCardReader_GetPeopleIDCode(pDlg->m_IdCardId, sizeof(pDlg->m_IdCardId));
				// 获取身份证有效期
				len = IDCardReader_GetStartDate(pDlg->m_IdCardIssuedate, sizeof(pDlg->m_IdCardIssuedate));

				Mat matphoto;
				pDlg->getIdcardMatPhoto(matphoto);
				//pDlg->getIdcardFeat(matphoto);
			}
		}
		else if (-6 == check) {
			CloseIDCardReader();
			int iopen = OpenIDCardReader();
			if (iopen < 0)
				AfxMessageBox("身份证读卡器连接失败！");
			pDlg->m_bIdcardReadFindCard = false;
		}
		else
			pDlg->m_bIdcardReadFindCard = false;

		SetEvent(pDlg->m_eIdcardReadDone);
	}

	SetEvent(pDlg->m_eIdcardReadEnd);
	return 0;
}

//face detect thread
UINT FaceDetectThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	g_CriticalSection.Lock();
	pDlg->m_bFaceDetectRun = true;
	g_CriticalSection.Unlock();

	int times_per_sec = 3;
	clock_t time_s, detect_dt;
	while (pDlg->m_bFaceDetectRun)
	{
		time_s = clock();
		pDlg->m_bCmdDetect = true;
		ResetEvent(pDlg->m_eCaptureForDetect);
		WaitForSingleObject(pDlg->m_eCaptureForDetect, INFINITE);
		if (!pDlg->m_bFaceDetectRun)
			break;

		//*/
		//clock_t time1 = clock();
		std::vector < std::vector<int>> face_rects;
		std::vector<int> tmpfacerect;
		std::string tmpfacefeat;
		Mat matframe(cvarrToMat(pDlg->m_iplImgCameraImg));
		Mat matframehide(cvarrToMat(pDlg->m_iplImgCameraImgHide));

		bool live = false;
		if (pDlg->m_bFirstFdv &&	// 只进行一次验证
			!matframe.empty() && !matframehide.empty()
			) {
			ResetEvent(pDlg->m_eIdcardReadDone);
			SetEvent(pDlg->m_eIdcardReadResume);

#if DEBUG_LOG_FILE
			g_CriticalSection.Lock();
			pDlg->m_logfile << "start livecheck! " << clock() - time_global << "ms" << endl;
			time_global = clock();
			g_CriticalSection.Unlock();
#endif

			g_CriticalSectionAiFdr.Lock();
			live = pDlg->m_pfrmwrap->livecheck(matframe, matframehide, tmpfacerect, tmpfacefeat);
			g_CriticalSectionAiFdr.Unlock();

#if DEBUG_LOG_FILE
			g_CriticalSection.Lock();
			pDlg->m_logfile << "end livecheck! " << clock() - time_global << "ms" << endl;
			pDlg->m_logfile << "livecheck:" << ((live) ? "true" : "false") << endl;
			time_global = clock();
			g_CriticalSection.Unlock();
#endif
			if (live) {
				// 提取身份证照片feat,仅在这里提取
#if DEBUG_LOG_FILE
				clock_t stime2 = clock();
				g_CriticalSection.Lock();
				pDlg->m_logfile << "start get photo feat!" << endl;
				g_CriticalSection.Unlock();
#endif
				WaitForSingleObject(pDlg->m_eGetIdCardFeat, INFINITE);
				if (!pDlg->m_bFaceDetectRun)
					break;
				if (!pDlg->m_bIdCardNoChange) {
					// 仅身份证有变化后提取
					Mat matphoto = cvarrToMat(pDlg->m_iplImgPhoto);
					pDlg->getIdcardFeat(matphoto);
				}

#if DEBUG_LOG_FILE
				g_CriticalSection.Lock();
				pDlg->m_logfile << "end get photo feat!" << clock() - stime2 << "ms" << endl;
				g_CriticalSection.Unlock();
#endif
			}
		}
		//clock_t dt = clock() - time1;
		//CString csTemp0;
		//csTemp0.Format("%d", dt);
		//AfxMessageBox(csTemp0);
		//*/

#if	DEBUG_LIVECHECK_OUTPUT
		//*
		SYSTEMTIME st;
		GetLocalTime(&st);
		CString fcsTemp;
		fcsTemp.Format("/dump/lcs%4d%02d%02d%02d%02d%02d%03d", 
						st.wYear, st.wMonth, st.wDay,
						st.wHour, st.wMinute, st.wSecond,
						st.wMilliseconds);
		if (live) {
		string fn = pDlg->m_strModulePath + fcsTemp.GetString() + "_0_t.png";
		cv::imwrite(fn.c_str(), matframe);
		fn = pDlg->m_strModulePath + fcsTemp.GetString() + "_1_t.png";
		cv::imwrite(fn.c_str(), matframehide);
		}
		else {
		string fn = pDlg->m_strModulePath + fcsTemp.GetString() + "_0.png";
		cv::imwrite(fn.c_str(), matframe);
		fn = pDlg->m_strModulePath + fcsTemp.GetString() + "_1.png";
		cv::imwrite(fn.c_str(), matframehide);
		}
		//*/
#endif
		//tmpfacerect.push_back(0);
		//tmpfacerect.push_back(0);
		//tmpfacerect.push_back(1280);
		//tmpfacerect.push_back(720);

		//pDlg->m_pfrmwrap->dectect_faces(matframe, face_rects, -1, true);
		g_CriticalSection.Lock();
		pDlg->m_faces.clear();
		vector<Rect>().swap(pDlg->m_faces);
		g_CriticalSection.Unlock();
		if (tmpfacerect.size() > 0) {
			Rect tmpRect;
			tmpRect.x = tmpfacerect[0];
			tmpRect.y = tmpfacerect[1];
			tmpRect.width = tmpfacerect[2]- tmpfacerect[0];
			tmpRect.height = tmpfacerect[3]- tmpfacerect[1];
			g_CriticalSection.Lock();
			pDlg->m_faces.push_back(tmpRect);
			g_CriticalSection.Unlock();

			// 开启识别线程
			bool success = pDlg->startFdvThread(tmpfacefeat, live);
			if (success) {
				ResetEvent(pDlg->m_eFaceDetectResume);
				WaitForSingleObject(pDlg->m_eFaceDetectResume, INFINITE);
				if (!pDlg->m_bFaceDetectRun)
					break;
			}
		}	

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

//====================================
// camera preview
void CIDCardFdvDlg::checkAndOpenAllCamera()
{
	clock_t time_s,time_dt;
	if (-1 == m_iMainDevIdx)
		m_iMainDevIdx = 0;// getDeviceIndex(m_cfgCameraVid, m_cfgCameraPid);
	if (-1 == m_iHideDevIdx)
		m_iHideDevIdx = getDeviceIndex(m_cfgCameraHideVid, m_cfgCameraHidePid);

//	m_iMainDevIdx = 0;	// test
	time_s = clock();
	if (!m_vcapMain.isOpened()) {
		m_vcapMain.open(m_iMainDevIdx); // m_vcapMain.open(m_iMainDevIdx, CAP_DSHOW);	
		if (m_vcapMain.isOpened()) {
			m_vcapMain.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
			m_vcapMain.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
		}
	}

	if (m_iHideDevIdx >= 0){
		if (!m_vcapHide.isOpened()) {
			m_vcapHide.open(m_iHideDevIdx);// m_vcapHide.open(m_iHideDevIdx, CAP_DSHOW);
			if (m_vcapHide.isOpened()) {
				m_vcapHide.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
				m_vcapHide.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
			}
		}
	}
	time_dt = clock() - time_s;
}

void CIDCardFdvDlg::closeAllCamera()
{
	if (m_vcapMain.isOpened()) {
		m_vcapMain.release();
	}
	if (m_vcapHide.isOpened()) {
		m_vcapHide.release();
	}
}

//camera preview thread
UINT CameraShowThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	g_CriticalSection.Lock();
	pDlg->m_bCameraRun = true;
	g_CriticalSection.Unlock();
	pDlg->m_bMainFrameSuccess = false;
	pDlg->m_bHideFrameSuccess = false;
	Mat cFrame, cFrameHide;
	while (pDlg->m_bCameraRun)
	{
		WaitForSingleObject(pDlg->m_eCameraResume, INFINITE);
		if (!pDlg->m_bCameraRun)
			break;

		// 如摄像头意外关闭，尝试重新打开
		pDlg->checkAndOpenAllCamera();

		pDlg->m_vcapMain.read(cFrame);
		if (cFrame.empty()) {
			//AfxMessageBox("读取图像帧错误！");
			if (pDlg->m_bMainFrameSuccess){
				// 已打开的情况下出错，关闭后重新打开
				pDlg->m_vcapMain.release();
				pDlg->m_bMainFrameSuccess = false;
				pDlg->m_iMainDevIdx = -1;
			}
			continue;
		}
		else
			pDlg->m_bMainFrameSuccess = true;

		if (pDlg->m_iHideDevIdx >= 0) {
			// 避免其他处理用时造成的误差，先在此读入
			pDlg->m_vcapHide.read(cFrameHide);
			if (cFrameHide.empty()) {
				//AfxMessageBox("红外摄像头读取图像帧错误！");
				if (pDlg->m_bHideFrameSuccess){
					// 已打开的情况下出错，关闭后重新打开
					pDlg->m_vcapHide.release();
					pDlg->m_bHideFrameSuccess = false;
					pDlg->m_iHideDevIdx = -1;
				}
				continue;
			}
			else
				pDlg->m_bHideFrameSuccess = true;
		}

		IplImage* newframe = &IplImage(cFrame);
		if (pDlg->m_bCmdDetect) {
#if DEBUG_LOG_FILE
			g_CriticalSection.Lock();
			pDlg->m_logfile << "start capture for detect! " << clock()-time_global<<"ms"<<endl;
			time_global = clock();
			g_CriticalSection.Unlock();
#endif
			if (pDlg->m_iplImgCameraImg) {
				cvReleaseImage(&(pDlg->m_iplImgCameraImg));
				pDlg->m_iplImgCameraImg = NULL;
			}
			pDlg->m_iplImgCameraImg = cvCloneImage(newframe);


			if (pDlg->m_iplImgCameraImgHide) {
				cvReleaseImage(&(pDlg->m_iplImgCameraImgHide));
				pDlg->m_iplImgCameraImgHide = NULL;
			}
			if (pDlg->m_vcapHide.isOpened()) {
				if (!cFrameHide.empty()) {
					IplImage* newframeHide = &IplImage(cFrameHide);					
					pDlg->m_iplImgCameraImgHide = cvCloneImage(newframeHide);
				}
			}

			pDlg->m_bCmdDetect = false;
			SetEvent(pDlg->m_eCaptureForDetect);

#if DEBUG_LOG_FILE
			g_CriticalSection.Lock();
			pDlg->m_logfile << "end capture for detect! " << clock() - time_global << "ms" << endl;
			time_global = clock();
			g_CriticalSection.Unlock();
#endif
		}

		// 画人脸框，需在截取人脸图后
		//if (pDlg->m_faces.size() > 0) {
		//	//for (int i = 0; i<faces.size(); i++)
		//	{
		//		int i = 0;
		//		rectangle(cFrame,
		//			Point(facex, facey),
		//			Point(facex + FaceImgSize.width, facey + FaceImgSize.height),
		//			Scalar(0, 255, 0), 2);    // 框出人脸
		//	}
		//}

		
		// 扫描框
		if(pDlg->m_bDrawScan)
			pDlg->drawScanRect(cFrame);

		// 结果图标
		if (pDlg->m_bDrawResultIconRight)
			pDlg->drawResultIcon(cFrame, pDlg->m_ResultIconRight);
		if (pDlg->m_bDrawResultIconWrong)
			pDlg->drawResultIcon(cFrame, pDlg->m_ResultIconWrong);

		pDlg->showPreview(newframe, pDlg->m_iPreviewX);
		Sleep(1);
	}

	//pDlg->closeAllCamera();
	SetEvent(pDlg->m_eCameraEnd);
	return 0;
}

//====================================

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
		if (pDlg->m_bCmdFdvStop) {
			pDlg->m_bFdvRun = false;
			SetEvent(pDlg->m_eFdvEnd);
			SetEvent(pDlg->m_eFaceDetectResume);
			return 0;
		}
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
		//int iopen = OpenIDCardReader();
		WaitForSingleObject(pDlg->m_eIdcardReadDone, INFINITE);
		if (NULL == pDlg->m_iplImgUploadCopyPhoto)
			pDlg->m_iplImgUploadCopyPhoto = cvCloneImage(pDlg->m_iplImgPhoto);
		if (!pDlg->m_bFirstFdv && !pDlg->m_bIdcardReadFindCard) {
			pDlg->m_bFdvRun = false;
			SetEvent(pDlg->m_eFdvEnd);
			SetEvent(pDlg->m_eFaceDetectResume);
			return 0;
		}
		else
		{
			KillTimer(pDlg->m_hWnd, CLEAR_INFOIMG_TIMER);

			// mat
			Mat matframe = cvarrToMat(pDlg->m_CaptureImage);
			//Mat matphoto = cvarrToMat(pDlg->m_iplImgPhoto);

			// 显示
			//clock_t t1 = clock();
			//pDlg->m_pInfoDlg->clearResultIcon();  // 验证结果图标清空
			pDlg->m_bDrawResultIconRight = pDlg->m_bDrawResultIconWrong = false;
			pDlg->m_pInfoDlg->drawCameraImage(pDlg->m_CaptureImage);
			pDlg->m_bDrawScan = false;
			pDlg->m_pInfoDlg->drawIdcardImage(pDlg->m_iplImgPhoto);
			//clock_t t2 = clock() - t1;
			if (false == pDlg->m_bIsAliveSample) {
				// 非活体警告
				pDlg->m_pInfoDlg->setResultTextSize(60);
				pDlg->m_pInfoDlg->setResultText("非活体！");
				pDlg->setClearTimer();
				pDlg->m_bFdvRun = false;
				SetEvent(pDlg->m_eFdvEnd);
				SetEvent(pDlg->m_eFaceDetectResume);
				return 0;
			}
			pDlg->m_pInfoDlg->setResultTextSize(60);
			pDlg->m_pInfoDlg->setResultText("--%");

#if OPENCV_CAPTURE
#ifdef NDEBUG
#else
			std::vector<uchar> idcardPhoto(pDlg->m_IdCardPhoto, pDlg->m_IdCardPhoto + lenbmp);
			std::vector<uchar> verifyPhotos[1];

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
			//clock_t t1 = clock();
			if (NULL == pDlg->m_iplImgCameraImgHide) {
				// 由于活体检测时可得到feat，只在没做活体检测情况下取feat
				std::vector < std::vector<int>> face_rects2;
				pDlg->m_frameFaceFeats.clear();
				pDlg->m_frameFaceFeats.shrink_to_fit();
				g_CriticalSectionAiFdr.Lock();
				int frame_face_cnt = pDlg->m_pfrmwrap->ai_fdr_edge_descriptors(matframe, face_rects2, pDlg->m_frameFaceFeats, 0, true);
				g_CriticalSectionAiFdr.Unlock();
			}
			//std::ofstream logfile(strModulePath + "des_photo.txt");
			//logfile << pDlg->m_photoFaceFeat;
			//logfile.close();
			//std::ofstream logfile2(strModulePath + "des_frame.txt");
			//for (auto ltrb : pDlg->m_frameFaceFeats)
			//	logfile2 << ltrb << endl;
			//logfile2.close();
			//clock_t t2 = clock() - t1;

#if DEBUG_LOG_FILE
			g_CriticalSection.Lock();
			pDlg->m_logfile << "start call Verify! " << clock() - time_global << "ms" << endl;
			time_global = clock();
			g_CriticalSection.Unlock();
#endif
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
		SetEvent(pDlg->m_eFaceDetectResume);
	}
	catch (std::exception e) {
		pDlg->m_bFdvRun = false;
		SetEvent(pDlg->m_eFdvEnd);
		SetEvent(pDlg->m_eFaceDetectResume);
	}
	
	return 0;
}

// idcard detect thread
UINT ImgUploadThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;

	g_CriticalSection.Lock();
	pDlg->m_bImgUploadRun = true;
	g_CriticalSection.Unlock();	

	pDlg->m_imgUploadMgt = new CImgUploadMgt();
	CImgUploadMgt* mgt = pDlg->m_imgUploadMgt;

	while (pDlg->m_bImgUploadRun)
	{
		if (pDlg->m_bImgUploadPause) {
			// 预览开始后避免干扰验证，暂停上传
			// 回到帮忙画面后重新开启。
			ResetEvent(pDlg->m_eImgUploadResume);
			WaitForSingleObject(pDlg->m_eImgUploadResume, INFINITE);
			if (!pDlg->m_bImgUploadRun)
				break;
		}

		if (!mgt->isQueueEmpty()) {
			mgt->upload(pDlg->m_cfgUploadUrl,
				pDlg->m_cfgAppId, pDlg->m_cfgApiKey, pDlg->m_cfgSecretKey, 
				pDlg->m_macId, pDlg->m_cfgRegisteredNo, 
				::stoi(pDlg->m_cfgTimeOut));
		}
		Sleep(1);
	}

	delete pDlg->m_imgUploadMgt;
	pDlg->m_imgUploadMgt = NULL;
	SetEvent(pDlg->m_eImgUploadEnd);
	return 0;
}



void CIDCardFdvDlg::setClearTimer(int sec)
{
	KillTimer(CLEAR_INFOIMG_TIMER);
	SetTimer(CLEAR_INFOIMG_TIMER, 1000 * sec, NULL);
}

bool CIDCardFdvDlg::idcardPreRead()
{
#if DEBUG_LOG_FILE
	g_CriticalSection.Lock();
	time_global = clock();
	time_global2 = time_global;
	clock_t stime = time_global;
	m_logfile << endl << endl;
	m_logfile << "start read idcard!" << endl;
	g_CriticalSection.Unlock();
#endif
	Authenticate_Content();

	char idcardid[256];
	long len = IDCardReader_GetPeopleIDCode(idcardid, sizeof(idcardid));
	if (strcmp(m_IdCardId, idcardid) == 0) {
		// 身份证无变化
		m_bIdCardNoChange = true;
		SetEvent(m_eGetIdCardFeat);	// ready to get feat
	}
	else {
		m_bIdCardNoChange = false;
		// 获取身份证号
		long len = IDCardReader_GetPeopleIDCode(m_IdCardId, sizeof(m_IdCardId));
		// 获取身份证有效期
		len = IDCardReader_GetStartDate(m_IdCardIssuedate, sizeof(m_IdCardIssuedate));

		Mat matphoto;
		getIdcardMatPhoto(matphoto);

		//getIdcardFeat(matphoto);
		SetEvent(m_eGetIdCardFeat);	// ready to get feat
	}

	g_CriticalSection.Lock();
	m_bFirstFdvIdcardReaded = true;
	g_CriticalSection.Unlock();

#if DEBUG_LOG_FILE
	g_CriticalSection.Lock();
	m_logfile << "end read idcard! time:" << clock() - stime << "ms" << endl;
	g_CriticalSection.Unlock();
#endif

	return true;
}
void CIDCardFdvDlg::getIdcardMatPhoto(cv::Mat &matphoto)
{
	// 获取照片
	long lenbmp = IDCardReader_GetPhotoBMP(m_IdCardPhoto, sizeof(m_IdCardPhoto));

	if (m_iplImgPhoto)
		cvReleaseImage(&(m_iplImgPhoto));	// 先释放

	vector<char>  vcBuf;
	vcBuf.insert(vcBuf.end(), m_IdCardPhoto, m_IdCardPhoto + lenbmp);
	matphoto = cv::imdecode(vcBuf, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	IplImage imgTmp = matphoto;

	m_iplImgPhoto = cvCloneImage(&imgTmp);
	//m_iplImgPhoto = BMP2Ipl((unsigned char*)m_IdCardPhoto, lenbmp);
}

void CIDCardFdvDlg::getIdcardFeat(cv::Mat &matphoto)
{
	std::vector < std::vector<int>> face_rects;

	g_CriticalSectionAiFdr.Lock();
	int photo_face_cnt = m_pfrmwrap->dectect_faces(matphoto, face_rects, 0, true);
	g_CriticalSectionAiFdr.Unlock();
	if (photo_face_cnt < 1) {
		face_rects.push_back({ 10,0,92,110 }); // l t r b
											   //face_rects.push_back({ 0,0,102,126 });
	}
	m_photoFaceFeat.clear();
	m_photoFaceFeat.shrink_to_fit();
	g_CriticalSectionAiFdr.Lock();
	m_pfrmwrap->ai_fdr_edge_descriptor(matphoto, face_rects[0], m_photoFaceFeat);
	g_CriticalSectionAiFdr.Unlock();
}


BOOL CIDCardFdvDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	stopIdcardReadThread();
	stopImgUploadThread();
	stopDataLoadingThread();

	if (m_iplImgHelpImg != NULL) {
		cvReleaseImage(&m_iplImgHelpImg);
		m_iplImgHelpImg = NULL;
	}

	if (m_iplImgDisplay != NULL) {
		cvReleaseImage(&m_iplImgDisplay);
		m_iplImgDisplay = NULL;
	}

	if (m_iplImgTemp != NULL) {
		cvReleaseImage(&m_iplImgTemp);
		m_iplImgTemp = NULL;
	}

	if (!m_MatScan.empty()) {
		m_MatScan.release();
	}

	if (!m_MatScanBorder.empty()) {
		m_MatScanBorder.release();
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

	if (m_iplImgCameraImgHide != NULL) {
		cvReleaseImage(&m_iplImgCameraImgHide);
		m_iplImgCameraImgHide = NULL;
	}

	if (!m_ResultIconRight.empty()) {
		m_ResultIconRight.release();
	}

	if (!m_ResultIconWrong.empty()) {
		m_ResultIconWrong.release();
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

	if (m_pAttentionDlg) {
		delete m_pAttentionDlg;
		m_pAttentionDlg = NULL;
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

	// openal alut release
	if (m_bsndReady) {
		alDeleteSources(1, &m_sndRightSource);
		alDeleteBuffers(1, &m_sndRightBuffer);
		alDeleteSources(1, &m_sndWrongSource);
		alDeleteBuffers(1, &m_sndWrongBuffer);
		//alutSleep(1.5);
	}

	return CDialogEx::DestroyWindow();
}

void CIDCardFdvDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (true == m_bMainWinClose)
		return;
	m_bMainWinClose = true;	// 关闭主窗口->全部退出，只允许处理一次

	m_pInfoDlg->ShowWindow(SW_HIDE);
	
	KillTimer(CLEAR_INFOIMG_TIMER);

#if OPENCV_CAPTURE
	stopFaceDetectThread(); // 停止预览线程前先停止检测
	stopCameraThread();
	m_pAttentionDlg->setVisible(false);
	closeAllCamera();
	Sleep(20);
#else
	m_pInfoDlg->ShowWindow(SW_HIDE);
	MTLibCloseCamera();
	MTLibUnloadCamera();
#endif
	waitFdvThreadStopped();
	stopIdcardDetectThread();

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
		m_bDrawScan = true;
		m_pInfoDlg->clearIdcardImage();
		m_bDrawResultIconRight = m_bDrawResultIconWrong = false;//m_pInfoDlg->clearResultIcon();
		m_pInfoDlg->setResultText("");
		KillTimer(CLEAR_INFOIMG_TIMER);
		//int chk1, chk2;
		//g_CriticalSectionIdCard.Lock();
		//chk1 = Authenticate_Idcard();
		//chk2 = Authenticate_Content(2);
		//g_CriticalSectionIdCard.Unlock();
		//if (chk1 <= 0 && chk2 < 0) {
		if(true){
			ResetEvent(m_eCameraResume);
			waitFdvThreadStopped();

			m_pInfoDlg->ShowWindow(SW_HIDE);

			// 重启帮忙画面
			SetEvent(m_eIdcardDetectResume);
			// 重启数据上传 
			//g_CriticalSection.Lock();
			//m_bImgUploadPause = false;
			//g_CriticalSection.Unlock();
			//SetEvent(m_eImgUploadResume);
		}
		else
			setClearTimer();
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
