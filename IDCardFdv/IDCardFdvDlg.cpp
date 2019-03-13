
// IDCardFdvDlg.cpp : ʵ���ļ�
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

#define PREVIEW_DEBUG	0
#define TEST_FILE_DEBUG	0
#define SHOW_ADV		0
#define SHOW_ATTENTION	1

#define CLEAR_INFOIMG_TIMER 1
#define ADV_AND_BACK_TIMER	3

clock_t time_net=0;
clock_t time_global = 0;
clock_t time_global2 = 0;

CCriticalSection g_CriticalSection;
CCriticalSection g_CriticalSectionAiFdr;
CCriticalSection g_CriticalSectionIDCard;
UINT DataLoadingThread(LPVOID lpParam);
UINT IdcardDetectThread(LPVOID lpParam);
UINT FaceDetectThread(LPVOID lpParam);
UINT CameraShowThread(LPVOID lpParam);
UINT FdvThread(LPVOID lpParam);
UINT ImgUploadThread(LPVOID lpParam);

// ʶ��callback
static void __stdcall VerifyCB(int err_no, std::string err_msg, double similarity, std::string serial_no, MTLIBPTR userdata)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)userdata;

	if (MTLIBNETWORK_NETWORK_ERROR == err_no) {
		// ��ʱ���������������
#ifdef NDEBUG
		//double sim = 0.0;
		//g_CriticalSectionAiFdr.Lock();
		//int simret = pDlg->m_pfrmwrap->ai_fdr_similarity(pDlg->m_photoFaceFeat, pDlg->m_frameFaceFeats[0], sim);
		//g_CriticalSectionAiFdr.Unlock();
		//VerifyCB(0, "", sim, serial_no, userdata);
		VerifyCB(200, _T("�����������!"), -1, serial_no, userdata);
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

#if SHOW_ADV
	pDlg->m_bShowAdv = true;
#else
	pDlg->m_bShowAdv = false;
#endif
	pDlg->m_HtmlView.SetUrl(pDlg->m_cfgAdvUrl.c_str());
	pDlg->setClearTimer();


	// �Ա�ͼ�����ϴ�����
	if ( serial_no != "") {
		bool bInsert = false;
		/*
		bInsert = pDlg->m_imgUploadMgt->insert(serial_no, 
								pDlg->m_iplImgUploadCopyFrame, 
								pDlg->m_iplImgUploadCopyFrameHide,
								pDlg->m_iplImgUploadCopyPhoto,
								pDlg->m_IDCardId, 
								pDlg->m_IDCardIssuedate);
		*/
		string path = pDlg->m_strModulePath + pDlg->m_cfgUploadLoc;
		bInsert = saveToLocal(path,
								pDlg->m_IDCardId,
								serial_no,
								similarity * 1000,
								pDlg->m_iplImgUploadCopyFrame,
								pDlg->m_iplImgUploadCopyFrameHide,
								pDlg->m_iplImgUploadCopyPhoto);
		//���汾��ʱ��Ҫ�ͷ�
		if (pDlg->m_iplImgUploadCopyFrame)
			cvReleaseImage(&pDlg->m_iplImgUploadCopyFrame);
		if (pDlg->m_iplImgUploadCopyFrameHide)
			cvReleaseImage(&pDlg->m_iplImgUploadCopyFrameHide);
		if(pDlg->m_iplImgUploadCopyPhoto)
			cvReleaseImage(&pDlg->m_iplImgUploadCopyPhoto);

		// ���۳ɹ��������ΪNULL
		pDlg->m_iplImgUploadCopyFrame = NULL;
		pDlg->m_iplImgUploadCopyFrameHide = NULL;
		pDlg->m_iplImgUploadCopyPhoto = NULL;

	}

	// ���������ϴ� 
	g_CriticalSection.Lock();
	pDlg->m_bImgUploadPause = false;
	g_CriticalSection.Unlock();
	SetEvent(pDlg->m_eImgUploadResume);

	bool bNoIDCard = pDlg->m_pOperationDlg->isNoIDCardCheckIn();
	if (bNoIDCard) {
		// ���������Ա����´�ʹ�����֤����������
		memset(pDlg->m_IDCardId, 0, sizeof(pDlg->m_IDCardId));
	}

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

// CIDCardFdvDlg �Ի���
CIDCardFdvDlg::CIDCardFdvDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IDCARDFDV_DIALOG, pParent), 
	m_eCameraResume(FALSE,TRUE,0,0), m_eGetIDCardFeat(FALSE, TRUE, 0, 0)
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
	m_iplImgTestPhoto = NULL;
	m_iplImgTestImage = NULL;
	m_iplImgTestImage2 = NULL;
#ifdef NDEBUG
	m_pfrmwrap = NULL;
#endif

	m_bMainWinClose = false;

	memset(m_IDCardId, 0, sizeof(m_IDCardId));
	memset(m_IDCardIssuedate, 0, sizeof(m_IDCardIssuedate));
	memset(m_IDCardPhoto, 0, sizeof(m_IDCardPhoto));
	m_iplImgPhoto = NULL;
	m_bIDCardReadOK = false;
	m_bIDCardNoChange = false;

	m_pMsgDlg = NULL;
	m_pOperationDlg = NULL;
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


// CIDCardFdvDlg ��Ϣ�������

BOOL CIDCardFdvDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_MAXIMIZE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//char sbuf[1024];
	//GetSystemDirectory(sbuf,1024);

	//��õ�����ʾ�������ؿ�Ⱥ����ظ߶�
	//int ax = GetDC()->GetDeviceCaps(HORZRES);
	//int ay = GetDC()->GetDeviceCaps(VERTRES);

	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	m_strModulePath = ExtractFilePath(szPath);

	// �����ؼ���Сλ��
	CRect rect;
	GetClientRect(&rect);		// ��ȡ�����Ի����С
	int rw = rect.right - rect.left;
	//int rh = rect.bottom - rect.top;
	int rh = rw * 720 / 1280;
	//rw = (int)(rw * 0.6);
	int offsetX = rect.left;
	int offsetY = rect.top;
	GetDlgItem(IDC_PREVIEW_IMG)->MoveWindow(offsetX, offsetY, rw, rh, false);
	m_iPreviewWidth = rw;
	m_iPreviewHeight = rh;

	GetDlgItem(IDC_STATIC_HTML)->MoveWindow(offsetX, offsetY, rw, rh, false);
	GetDlgItem(IDC_STATIC_HTML)->ShowWindow(SW_HIDE);
	m_HtmlView.CreateFromStatic(IDC_STATIC_HTML, this);
	m_HtmlView.ShowWindow(SW_HIDE);

	ClientToScreen(&rect);
	if (NULL == m_pMsgDlg) {
		int cx = (rect.right - rect.left) / 2;
		int cy = (rect.bottom - rect.top) / 2;
		m_pMsgDlg = new CMsgDlg(cx,cy);
		m_pMsgDlg->setFontRate(m_iPreviewWidth * 1.0f / 1366);// ����ʱʹ�õ���Ļ��1366��Ϊ��׼
		m_pMsgDlg->Create(IDD_MSG_DIALOG, this);
	}
	
	m_pMsgDlg->hide();

	// ����������崰��
	rw = (int)((rect.right - rect.left) * 0.1f);
	rh = (int)((rect.bottom - rect.top) * 0.08f);
	offsetX = (int)((rect.right - rect.left - rw)/2);
	offsetY = (int)(rect.bottom - (rect.bottom - rect.top) * 0.15f);
	if (NULL == m_pOperationDlg) {
		m_pOperationDlg = new COperationDlg(offsetX, offsetY, rw, rh);
		m_pOperationDlg->setFontRate(m_iPreviewWidth * 1.0f / 1366);// ����ʱʹ�õ���Ļ��1366��Ϊ��׼
		m_pOperationDlg->Create(IDD_OPERATION_DIALOG, this);	
	}
	m_pOperationDlg->ShowWindow(SW_SHOW);

	// ����������Ϣ����
	//ClientToScreen(&rect);
	float ADRATE = 0.4f;
	rw = (int)((rect.right - rect.left) * ADRATE); // ��Ϣ���ڿ�
	rh = rect.bottom - rect.top;					// ��Ϣ���ڸ�
	offsetX = rect.left;
	offsetY = rect.top;
	if (NULL == m_pAttentionDlg) {
		m_pAttentionDlg = new CAttentionDlg(offsetX, offsetY, rw, rh, "");
		m_pAttentionDlg->Create(IDD_ATTENTION_DIALOG, this);
		m_pAttentionDlg->setFontRate(m_iPreviewWidth * 1.0f / 1366);
	}
	m_pAttentionDlg->setVisible(false);
	m_iPreviewX = offsetX + rw / 2;

	// ������Ϣ����
//	ClientToScreen(&rect);
	float IPRATE = 0.15f;
	rw = (int)((rect.right - rect.left) * IPRATE); // ��Ϣ���ڿ�
	rh = rect.bottom - rect.top;					// ��Ϣ���ڸ�
	offsetX = rect.left + (int)((rect.right - rect.left) * (1 - IPRATE));
	offsetY = rect.top;
	if (NULL == m_pInfoDlg) {
		m_pInfoDlg = new CInfoDlg(offsetX, offsetY, rw, rh);
		m_pInfoDlg->setFontRate(m_iPreviewWidth * 1.0f / 1366);
		m_pInfoDlg->Create(IDD_INFO_DIALOG, this);
		m_pInfoDlg->setFdvDlgPtr((WORK_PTR)this);
		m_pInfoDlg->resetIDCardNoInput();
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

	// �������浥���ȶ���
	std::string fn;
	fn = m_strModulePath + "help.jpg";
	m_iplImgHelpImg = cvLoadImage(fn.c_str(), -1);

	// test files
#if TEST_FILE_DEBUG
	{
		fn = m_strModulePath + "Tphoto.bmp";
		m_iplImgTestPhoto = cvLoadImage(fn.c_str(), -1);
		cvtColor(cvarrToMat(m_iplImgTestPhoto), cvarrToMat(m_iplImgTestPhoto), CV_BGR2RGB);

		fn = m_strModulePath + "TF01_0.jpg";
		m_iplImgTestImage = cvLoadImage(fn.c_str(), -1);
		cvtColor(cvarrToMat(m_iplImgTestImage), cvarrToMat(m_iplImgTestImage), CV_BGR2RGB);

		fn = m_strModulePath + "TF01_1.jpg";
		m_iplImgTestImage2 = cvLoadImage(fn.c_str(), -1);
		cvtColor(cvarrToMat(m_iplImgTestImage2), cvarrToMat(m_iplImgTestImage2), CV_BGR2RGB);

	}
#endif
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
		AfxMessageBox("haarcascade_frontalface_default.xml���س���");
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
	m_cfgUploadLoc = "UPLOAD";
	m_cfgAdvUrl = "http://www.baidu.com";
	m_cfgTimeOut = "15";
	m_cfgAttStrMain = "������";
	std::ifstream confFile(m_strModulePath + "config.txt");
	std::string line;
	while (std::getline(confFile, line)){
		std::istringstream is_line(line);
		std::string key;
		if (std::getline(is_line, key, '=')){
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
				if (key == "uploadLoc")
					m_cfgUploadLoc = value;
				if (key == "advurl")
					m_cfgAdvUrl = value;
				if (key == "timeout")
					m_cfgTimeOut = value;
				if (key == "AttStrMain")
					m_cfgAttStrMain = value;
			}
		}
	}
	confFile.close();
	m_pAttentionDlg->setDepartment(m_cfgAttStrMain);
	
	m_cfgRegisteredNo = "0";
	std::ifstream RegFile(m_strModulePath + "configReg.txt");
	while (std::getline(RegFile, line)){
		std::istringstream is_line(line);
		std::string key;
		if (std::getline(is_line, key, '=')){
			std::string value;
			if (std::getline(is_line, value)) {
				if (key == "registeredNo")
					m_cfgRegisteredNo = value;
			}
		}
	}
	RegFile.close();

	m_pInfoDlg->setResultText("");
	m_pInfoDlg->setThresholdText("");
	
	// threads
	startDataLoadingThread();
	startIdcardDetectThread();
	startCameraThread();
	startFaceDetectThread();
	//startImgUploadThread();

	// ��ʾ��ʼֵ
	CString thstr;
	thstr.Format("%.2f%%", m_dThreshold);
	m_pInfoDlg->setThresholdText(thstr.GetString());

#if DEBUG_LOG_FILE
	m_logfile.open(m_strModulePath + "log.txt");
#endif

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

HBRUSH CIDCardFdvDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (nCtlColor == CTLCOLOR_DLG)  //�Ի�����ɫ
	{
		return (HBRUSH)::GetStockObject(WHITE_BRUSH);//�����Լ����õ�ˢ��
	}

	

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIDCardFdvDlg::OnPaint()
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
		//CPaintDC dc(this);
		//CRect rect;
		//GetClientRect(&rect);
		//dc.FillSolidRect(rect, RGB(255, 255, 255));	//��ɫ���ͻ���

		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CIDCardFdvDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIDCardFdvDlg::showPreview(IplImage* img, int dX)	
{
	UINT ID = IDC_PREVIEW_IMG;				// ID ��Picture Control�ؼ���ID��

	CDC* pDC = GetDlgItem(ID)->GetDC();		// �����ʾ�ؼ��� DC
	HDC hDC = pDC->GetSafeHdc();				// ��ȡ HDC(�豸���) �����л�ͼ����

	CRect rect;

	GetDlgItem(ID)->GetClientRect(&rect);
	int rw = rect.right - rect.left;			// ���ͼƬ�ؼ��Ŀ�͸�
	int rh = rect.bottom - rect.top;

	int iw = img->width;						// ��ȡͼƬ�Ŀ�͸�
	int ih = img->height;
	//TRACE("rect:w,h %d, %d: %d, %d", rw, rh, iw, ih);
	//int tx = (int)(rw - iw)/2;					// ʹͼƬ����ʾλ�������ڿؼ�������
	//int ty = (int)(rh - ih)/2;
	//SetRect( rect, tx, ty, tx+iw, ty+ih );
	//SetRect(rect, 0, 0, rw, rh);	// �����ؼ�
	
	
	int displayX = dX;
	int displayW = rw;
	int displayH = rw * ih / iw;
	int displayY = (int)(rh - displayH) / 2;	// ��ֱ����
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
	cimg.CopyOf(pImg, -1);							// ����ͼƬ
	cimg.DrawToHDC(hDC, &rect);				// ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������

	//CBrush base(RGB(255, 255, 255));
	//CBrush color(RGB(0, 0, 255));
	//pDC->FillRect(CRect(100, 100, 200, 200), &base);

	ReleaseDC(pDC);
}

void CIDCardFdvDlg::drawHelpImage(IplImage* img)
{
	UINT ID = IDC_PREVIEW_IMG;

	CDC* pDC = GetDlgItem(ID)->GetDC();		// �����ʾ�ؼ��� DC
	HDC hDC = pDC->GetSafeHdc();				// ��ȡ HDC(�豸���) �����л�ͼ����

	CRect rect;

	GetDlgItem(ID)->GetClientRect(&rect);
	int rw = rect.right - rect.left;			// ���ͼƬ�ؼ��Ŀ�͸�
	int rh = rect.bottom - rect.top;

	SetRect(rect, 0, 0, rw, rh);	// �����ؼ�

	if (NULL == img) {
		pDC->FillSolidRect(rect, RGB(74, 144, 226));
	}
	else {
		CvvImage cimg;
		cimg.CopyOf(img, -1);							// ����ͼƬ
		cimg.DrawToHDC(hDC, &rect);				// ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������
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
	scanX = scanX + (camera_img_rect.width * frame.cols / m_iPreviewWidth - m_MatScan.cols) / 2; // ����
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
	sX = sX + (result_icon_rect.width * frame.cols / m_iPreviewWidth - icon.cols) / 2; // ����
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
			TRACE("�����µ��̳߳���\n");
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
			TRACE("�����µ��̳߳���\n");
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

void CIDCardFdvDlg::startCameraThread()
{
	ResetEvent(m_eCameraEnd);

	if (m_thCamera == NULL) {
		m_thCamera = AfxBeginThread(CameraShowThread, this);
		if (NULL == m_thCamera)
		{
			TRACE("�����µ��̳߳���\n");
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
			TRACE("�����µ��̳߳���\n");
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
		SetEvent(m_eCaptureForDetect); // �ͷŽ�ͼ�ȴ�
		SetEvent(m_eGetIDCardFeat);	   // �ͷ���ȡfeat�ȴ�
		SetEvent(m_eFaceDetectResume); // �ͷż������ȴ�
		WaitObjectAndMsg(m_eFaceDetectEnd, INFINITE);
		ResetEvent(m_eFaceDetectEnd);

		m_thFaceDetect = NULL;
	}
}


bool CIDCardFdvDlg::startFdvThread(std::string feat, bool live)
{
	if (!m_bCameraRun)
		return false;

	if (m_bFdvRun)
		return false;

	bool bNoIDCard = m_pOperationDlg->isNoIDCardCheckIn();
	if (!bNoIDCard && NULL == m_iplImgPhoto)
		return false;	// ���֤��ȡʧ��ʱ������

	m_frameFaceFeats.clear();
	m_frameFaceFeats.shrink_to_fit();
	m_frameFaceFeats.push_back(feat);
	m_bIsAliveSample = live;

	// ʶ��ͼ����
	if (m_iplImgUploadCopyFrame)
		cvReleaseImage(&m_iplImgUploadCopyFrame);
	if (m_iplImgUploadCopyFrameHide)
		cvReleaseImage(&m_iplImgUploadCopyFrameHide);
	if (m_iplImgUploadCopyPhoto)
		cvReleaseImage(&m_iplImgUploadCopyPhoto);

	m_iplImgUploadCopyFrame = cvCloneImage(m_iplImgCameraImg);
	m_iplImgUploadCopyFrameHide = cvCloneImage(m_iplImgCameraImgHide);
	if (!bNoIDCard && m_iplImgPhoto){
		m_iplImgUploadCopyPhoto = cvCloneImage(m_iplImgPhoto);
	}


	// �س�����
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

		// �ͷžɽ�ͼ
		if (m_CaptureImage) {
			cvReleaseImage(&(m_CaptureImage));
			m_CaptureImage = NULL;
		}

		// �س�����ͼ
		m_CaptureImage = cvCreateImage(FaceImgSize, m_iplImgCameraImg->depth, m_iplImgCameraImg->nChannels);
		cvSetImageROI(m_iplImgCameraImg, cvRect(facex, facey, FaceImgSize.width, FaceImgSize.height));
		cvCopy(m_iplImgCameraImg, m_CaptureImage);
		cvResetImageROI(m_iplImgCameraImg);
		//pDlg->m_CaptureImage = cvCloneImage(newframe);
		//string fn = m_strModulePath + "face0.png";
		//imwrite(fn.c_str(), cvarrToMat(m_CaptureImage));
	}

	m_bFdvRun = true;
	ResetEvent(m_eFdvEnd);
	m_thFdv = AfxBeginThread(FdvThread, this);

	if (NULL == m_thFdv)
	{
		TRACE("�����µ��̳߳���\n");
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
			TRACE("�����µ��̳߳���\n");
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
		SetEvent(m_eImgUploadResume);	// �ͷ��߳��ڵȴ�
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
		// ͼ
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

		// openal,alut�Լ�����׼��
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

		// ����һ�������������ߺ����ٶȡ�	
		Sleep(10);
		MTLibTestUrlIgnoreResp(pDlg->m_cfgUrl, NULL, (MTLIBPTR)pDlg, ::stoi(pDlg->m_cfgTimeOut));

		// �޸�ע���ȷ��CHtmlViewʹ�õ�IE�ں˰汾
		RaisePrivileges();
		SetIECoreVersion();

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
#if PREVIEW_DEBUG
	int iopen = 1;
#else
	int iopen = OpenIDCardReader();
#endif
	if (iopen < 0) {
		pDlg->m_pMsgDlg->setMessage("���֤����������ʧ�ܣ�");
		g_CriticalSection.Lock();
		//pDlg->m_bIdcardDetectRun = false;
		g_CriticalSection.Unlock();
	}

	bool reset_screen = false;
	while (pDlg->m_bIdcardDetectRun)
	{
		time_s = clock();

		bool bNoIDCard = pDlg->m_pOperationDlg->isNoIDCardCheckIn();
#if PREVIEW_DEBUG
		bNoIDCard = true;//false;
		int check = 1;
#else
		int check = Authenticate_Idcard(); //Authenticate_Content(2);
#endif		
		if (check > 0 || bNoIDCard) {
			//pDlg->drawHelpImage(NULL); // clear
			pDlg->m_pOperationDlg->ShowWindow(SW_HIDE);			
			pDlg->m_bShowAdv = false;

			g_CriticalSection.Lock();
			pDlg->m_bImgUploadPause = true; // ��ͣ�����ϴ�
			g_CriticalSection.Unlock();

			pDlg->m_bIDCardReadOK = false;
			ResetEvent(pDlg->m_eGetIDCardFeat);
			
			SetEvent(pDlg->m_eCameraResume);//pDlg->startCameraThread();
			SetEvent(pDlg->m_eFaceDetectResume);
			if (bNoIDCard) {
				pDlg->afterIDCardReadOK();
				pDlg->m_pInfoDlg->setMode(CInfoDlg::INFO_MODE_INCARDNO);
				pDlg->m_pInfoDlg->ShowWindow(SW_SHOW);
				pDlg->m_pInfoDlg->UpdateWindow();
				pDlg->setClearTimer(FDVDLG_DEFAULT_CLEAR_TIME);
			}
			else {
				pDlg->m_pInfoDlg->setMode(CInfoDlg::INFO_MODE_PHOTO);
				pDlg->idcardPreRead();			// ����Ԥ��
			}			

			ResetEvent(pDlg->m_eIdcardDetectResume);
			WaitForSingleObject(pDlg->m_eIdcardDetectResume, INFINITE);
			if (!pDlg->m_bIdcardDetectRun)
				break;	// ���˳�
			reset_screen = true;	// �ػ��������������������ù���
		}
		/*else if (-5 == check) {
			pDlg->drawHelpImage(pDlg->m_iplImgHelpImg);
			CloseIDCardReader();
			int iopen = OpenIDCardReader();
			if (iopen < 0)
				AfxMessageBox("���֤����������ʧ�ܣ�");
		}*/
		else {
			pDlg->drawHelpImage(pDlg->m_iplImgHelpImg);
			if (reset_screen) {
				pDlg->m_pAttentionDlg->setVisible(false);
				pDlg->m_pOperationDlg->ShowWindow(SW_SHOW);
				reset_screen = false;
			}
		}

		// �򿪲��������ͷ
		pDlg->checkAndOpenAllCamera();	// �ڰ�������ѭ�����


		detect_dt = clock() - time_s;
		if (detect_dt < 1000 / times_per_sec) {
			Sleep(1000 / times_per_sec - detect_dt);
		}
	}

	CloseIDCardReader();
	SetEvent(pDlg->m_eIdcardDetectEnd);
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
		if (!matframe.empty() && !matframehide.empty()) {
			// livecheck
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
			bool bNoIDCard = pDlg->m_pOperationDlg->isNoIDCardCheckIn();
			if (live && !bNoIDCard) {
				// ��ȡ���֤��Ƭfeat,����������ȡ
#if DEBUG_LOG_FILE
				clock_t stime2 = clock();
				g_CriticalSection.Lock();
				pDlg->m_logfile << "start get photo feat!" << endl;
				g_CriticalSection.Unlock();
#endif
				WaitForSingleObject(pDlg->m_eGetIDCardFeat, INFINITE);
				if (!pDlg->m_bFaceDetectRun)
					break;

				if (!pDlg->m_bIDCardNoChange || pDlg->m_photoFaceFeat.size()<=0) {
					// �����֤�б仯������ֵΪ��ʱ��ȡ
					Mat matphoto = cvarrToMat(pDlg->m_iplImgPhoto);
					pDlg->getIDCardFeat(matphoto);
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

			// ����ʶ���߳�
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
		m_iMainDevIdx = getDeviceIndex(m_cfgCameraVid, m_cfgCameraPid);
	if (-1 == m_iHideDevIdx)
		m_iHideDevIdx = getDeviceIndex(m_cfgCameraHideVid, m_cfgCameraHidePid);

#if PREVIEW_DEBUG
	m_iMainDevIdx = 0;
	m_iHideDevIdx = -1;
#endif

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

		// ������ͷ����رգ��������´�
		pDlg->checkAndOpenAllCamera();

		pDlg->m_vcapMain.read(cFrame);
		if (cFrame.empty()) {
			//AfxMessageBox("��ȡͼ��֡����");
			if (pDlg->m_bMainFrameSuccess){
				// �Ѵ򿪵�����³����رպ����´�
				pDlg->m_vcapMain.release();
				pDlg->m_bMainFrameSuccess = false;
				pDlg->m_iMainDevIdx = -1;
			}
			continue;
		}
		else
			pDlg->m_bMainFrameSuccess = true;

		if (pDlg->m_iHideDevIdx >= 0) {
			// ��������������ʱ��ɵ������ڴ˶���
			pDlg->m_vcapHide.read(cFrameHide);
			if (cFrameHide.empty()) {
				//AfxMessageBox("��������ͷ��ȡͼ��֡����");
				if (pDlg->m_bHideFrameSuccess){
					// �Ѵ򿪵�����³����رպ����´�
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
#if TEST_FILE_DEBUG
			cvReleaseImage(&(pDlg->m_iplImgCameraImg));
			cvReleaseImage(&(pDlg->m_iplImgCameraImgHide));
			pDlg->m_iplImgCameraImg = cvCloneImage(pDlg->m_iplImgTestImage);
			pDlg->m_iplImgCameraImgHide = cvCloneImage(pDlg->m_iplImgTestImage2);
#endif
			pDlg->m_bCmdDetect = false;
			SetEvent(pDlg->m_eCaptureForDetect);

#if DEBUG_LOG_FILE
			g_CriticalSection.Lock();
			pDlg->m_logfile << "end capture for detect! " << clock() - time_global << "ms" << endl;
			time_global = clock();
			g_CriticalSection.Unlock();
#endif
		}

		// �����������ڽ�ȡ����ͼ��
		//if (pDlg->m_faces.size() > 0) {
		//	//for (int i = 0; i<faces.size(); i++)
		//	{
		//		int i = 0;
		//		rectangle(cFrame,
		//			Point(facex, facey),
		//			Point(facex + FaceImgSize.width, facey + FaceImgSize.height),
		//			Scalar(0, 255, 0), 2);    // �������
		//	}
		//}

		
		// ɨ���
		if(pDlg->m_bDrawScan)
			pDlg->drawScanRect(cFrame);

		// ���ͼ��
		if (pDlg->m_bDrawResultIconRight)
			pDlg->drawResultIcon(cFrame, pDlg->m_ResultIconRight);
		if (pDlg->m_bDrawResultIconWrong)
			pDlg->drawResultIcon(cFrame, pDlg->m_ResultIconWrong);

		if(pDlg->m_bIDCardReadOK)
			pDlg->showPreview(newframe, pDlg->m_iPreviewX);
		Sleep(1);
	}

	//pDlg->closeAllCamera();
	SetEvent(pDlg->m_eCameraEnd);
	return 0;
}

//====================================

// fdv thread

UINT FdvThread(LPVOID lpParam)
{
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)lpParam;
	//pDlg->m_bFdvRun = true;

	bool bNoIDCard = pDlg->m_pOperationDlg->isNoIDCardCheckIn();

	try {
		char szPath[1024] = { 0 };
		GetModuleFileName(NULL, szPath, MAX_PATH);
		std::string strModulePath = ExtractFilePath(szPath);

		/*
		// ��ȡ����ͷ����,base64����תIplImage
		std::string facedatastr;
		Base64::Decode(pDlg->m_sCaptureBase64, &facedatastr);
		std::vector<uchar> facebuff(facedatastr.begin(), facedatastr.end());
		cv::Mat facemat = cv::imdecode(facebuff, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		IplImage imgTmp = facemat;
		pDlg->m_CaptureImage = cvCloneImage(&imgTmp);
		//*/
		
		{
			// mat
			Mat matframe = cvarrToMat(pDlg->m_CaptureImage);
			//Mat matphoto = cvarrToMat(pDlg->m_iplImgPhoto);

			// ��ʾ
			//pDlg->m_pInfoDlg->clearResultIcon();  // ��֤���ͼ�����
			pDlg->m_bDrawResultIconRight = pDlg->m_bDrawResultIconWrong = false;
			pDlg->m_pInfoDlg->drawCameraImage(pDlg->m_CaptureImage);
			pDlg->m_bDrawScan = false;

			if (bNoIDCard) {
				while (!pDlg->m_pInfoDlg->isIDCardNoReady()) {
					// �ȴ�������Ч���֤����
					if (pDlg->m_bCmdFdvStop) {	// ��ʱ�˳�
						pDlg->m_bFdvRun = false;
						SetEvent(pDlg->m_eFdvEnd);
						//SetEvent(pDlg->m_eFaceDetectResume);
						return 0;
					}
					Sleep(10);
				}
				pDlg->m_pInfoDlg->enableIDCardNoBtn(false);
			}

			if (pDlg->m_bCmdFdvStop) { // ��ʱ�˳�
				pDlg->m_bFdvRun = false;
				SetEvent(pDlg->m_eFdvEnd);
				//SetEvent(pDlg->m_eFaceDetectResume);
				return 0;
			}
			pDlg->stopClearTimer();
			//if (!bNoIDCard) {
			//	pDlg->m_pInfoDlg->drawIDCardImage(pDlg->m_iplImgPhoto);
			//}
			if (false == pDlg->m_bIsAliveSample) {
				// �ǻ��徯��
				pDlg->m_pInfoDlg->setResultTextSize(60);
				pDlg->m_pInfoDlg->setResultText("�ǻ��壡");
				pDlg->setClearTimer();
				pDlg->m_bFdvRun = false;
				SetEvent(pDlg->m_eFdvEnd);
				SetEvent(pDlg->m_eFaceDetectResume);
				return 0;
			}
			if (!bNoIDCard) {
				pDlg->m_pInfoDlg->setResultTextSize(60);
				pDlg->m_pInfoDlg->setResultText("--%");
			}

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
				// ���ڻ�����ʱ�ɵõ�feat��ֻ��û�������������ȡfeat
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
			if (!bNoIDCard) {
				if (NULL == pDlg->m_iplImgPhoto) {
					VerifyCB(200, _T("���֤��Ƭ����!"), -1, "", (MTLIBPTR)pDlg);
				}
				else if ("" == pDlg->m_photoFaceFeat) {
					VerifyCB(200, _T("���֤��Ƭ��������!"), -1, "", (MTLIBPTR)pDlg);
				}
				else {
					MTLibCallVerify(pDlg->m_cfgUrl,
						pDlg->m_cfgAppId, pDlg->m_cfgApiKey, pDlg->m_cfgSecretKey, uuid,
						pDlg->m_macId, pDlg->m_cfgRegisteredNo,
						pDlg->m_IDCardId, pDlg->m_IDCardIssuedate, pDlg->m_photoFaceFeat, pDlg->m_frameFaceFeats, 1,
						VerifyCB, (MTLIBPTR)pDlg, ::stoi(pDlg->m_cfgTimeOut));
				}
			}
			else {
				string nostr = pDlg->m_pInfoDlg->getIDCardNo();
				strcpy_s(pDlg->m_IDCardId, nostr.c_str());
				strcpy_s(pDlg->m_IDCardIssuedate,"");
				MTLibCallVerify(pDlg->m_cfgUrl,
					pDlg->m_cfgAppId, pDlg->m_cfgApiKey, pDlg->m_cfgSecretKey, uuid,
					pDlg->m_macId, pDlg->m_cfgRegisteredNo,
					pDlg->m_IDCardId, pDlg->m_IDCardIssuedate, "None", pDlg->m_frameFaceFeats, 1,
					VerifyCB, (MTLIBPTR)pDlg, ::stoi(pDlg->m_cfgTimeOut));
			}
			//*/
#else
			//*
			MTLibCallVerify_Image("http://192.168.1.201:8004/idcardfdv",//pDlg->m_cfgUrl,
				pDlg->m_cfgAppId, pDlg->m_cfgApiKey, pDlg->m_cfgSecretKey, uuid,
				pDlg->m_macId, pDlg->m_cfgRegisteredNo,
				pDlg->m_IDCardId, pDlg->m_IDCardIssuedate, idcardPhoto, verifyPhotos, 1,
				VerifyCB, (MTLIBPTR)pDlg, ::stoi(pDlg->m_cfgTimeOut));
			/**/
#endif

		}
		pDlg->m_bFdvRun = false;
		SetEvent(pDlg->m_eFdvEnd);
//		SetEvent(pDlg->m_eFaceDetectResume);
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
			// Ԥ����ʼ����������֤����ͣ�ϴ�
			// �ص���æ��������¿�����
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



void CIDCardFdvDlg::setClearTimer(float sec)
{
	KillTimer(CLEAR_INFOIMG_TIMER);
	int m = (int)(sec * 1000);
	SetTimer(CLEAR_INFOIMG_TIMER, m, NULL);
}

void CIDCardFdvDlg::stopClearTimer()
{
	KillTimer(CLEAR_INFOIMG_TIMER);
}

void CIDCardFdvDlg::afterIDCardReadOK()
{
	m_bIDCardReadOK = true;

	m_pMsgDlg->hide();

	m_bDrawScan = true;

#if SHOW_ATTENTION
	m_pAttentionDlg->setVisible(true);
	m_pAttentionDlg->UpdateWindow();
#else
	m_iPreviewX = 0;
#endif
}

bool CIDCardFdvDlg::idcardPreRead()
{
	bool ret = true;
#if DEBUG_LOG_FILE
	g_CriticalSection.Lock();
	time_global = clock();
	time_global2 = time_global;
	clock_t stime = time_global;
	m_logfile << endl << endl;
	m_logfile << "start read idcard!" << endl;
	g_CriticalSection.Unlock();
#endif

#if TEST_FILE_DEBUG
	strcpy_s(m_IDCardId, "339005198608221614");
	strcpy_s(m_IDCardIssuedate, "20111207");
	m_iplImgPhoto = cvCloneImage(m_iplImgTestPhoto);
	SetEvent(m_eGetIDCardFeat);
	return true;
#endif
	m_pMsgDlg->setMessage("���ڶ�ȡ���֤��Ϣ������",0);
	int content_ret = Authenticate_Content();
	if (1 != content_ret) {
		ret = false;
	}
	else {
		afterIDCardReadOK();
		char idcardid[256] = { 0 };
		long len = IDCardReader_GetPeopleIDCode(idcardid, sizeof(idcardid));
		if (strcmp(m_IDCardId, idcardid) == 0 && m_iplImgPhoto != NULL) {
			// ���֤�ޱ仯��֮ǰ��������ȡ
			m_bIDCardNoChange = true;
		}
		else {
			m_bIDCardNoChange = false;
			// ��ȡ���֤��
			long len = IDCardReader_GetPeopleIDCode(m_IDCardId, sizeof(m_IDCardId));
			// ��ȡ���֤��Ч��
			len = IDCardReader_GetStartDate(m_IDCardIssuedate, sizeof(m_IDCardIssuedate));

			Mat matphoto;
			getIDCardMatPhoto(matphoto);

			if (matphoto.empty())
				ret = false;
		}
	}

	if (!ret) {
		memset(m_IDCardId, 0, sizeof(m_IDCardId));
		memset(m_IDCardIssuedate, 0, sizeof(m_IDCardIssuedate));
		memset(m_IDCardPhoto, 0, sizeof(m_IDCardPhoto));
		if (m_iplImgPhoto) {
			cvReleaseImage(&(m_iplImgPhoto));
			m_iplImgPhoto = NULL;
		}
		m_pMsgDlg->setMessage("���֤��Ϣ��ȡ�������������֤���·��á�", 1.9f);
		setClearTimer(2.0f);
	}
	else {
		m_pInfoDlg->ShowWindow(SW_SHOW);
		m_pInfoDlg->drawIDCardImage(m_iplImgPhoto);
		setClearTimer(FDVDLG_DEFAULT_CLEAR_TIME);
	}

	//getIDCardFeat(matphoto);
	SetEvent(m_eGetIDCardFeat);	// ready to get feat

#if DEBUG_LOG_FILE
	g_CriticalSection.Lock();
	m_logfile << "end read idcard! time:" << clock() - stime << "ms" << endl;
	g_CriticalSection.Unlock();
#endif

	return ret;
}
void CIDCardFdvDlg::getIDCardMatPhoto(cv::Mat &matphoto)
{
	// ��ȡ��Ƭ
	long lenbmp = IDCardReader_GetPhotoBMP(m_IDCardPhoto, sizeof(m_IDCardPhoto));

	if (m_iplImgPhoto) {
		cvReleaseImage(&(m_iplImgPhoto));	// ���ͷ�
		m_iplImgPhoto = NULL;
	}

	if (lenbmp > 0) {
		vector<char>  vcBuf;
		vcBuf.insert(vcBuf.end(), m_IDCardPhoto, m_IDCardPhoto + lenbmp);
		matphoto = cv::imdecode(vcBuf, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	}

	if (!matphoto.empty()) {
		IplImage imgTmp = matphoto;
		m_iplImgPhoto = cvCloneImage(&imgTmp);
	}
	//m_iplImgPhoto = BMP2Ipl((unsigned char*)m_IDCardPhoto, lenbmp);
}

void CIDCardFdvDlg::getIDCardFeat(cv::Mat &matphoto)
{
	if (matphoto.empty())
		return;

	std::vector < std::vector<int>> face_rects;

	g_CriticalSectionAiFdr.Lock();
	int photo_face_cnt = m_pfrmwrap->dectect_faces(matphoto, face_rects, 1, true);
	g_CriticalSectionAiFdr.Unlock();
	if (photo_face_cnt < 1) {
		face_rects.push_back({ 19,35,80,97 }); // l t r b
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

	if (m_iplImgTestPhoto != NULL) {
		cvReleaseImage(&m_iplImgTestPhoto);
		m_iplImgTestPhoto = NULL;
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

	if (m_pMsgDlg) {
		delete m_pMsgDlg;
		m_pMsgDlg = NULL;
	}

	if (m_pOperationDlg) {
		delete m_pOperationDlg;
		m_pOperationDlg = NULL;
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (true == m_bMainWinClose)
		return;
	m_bMainWinClose = true;	// �ر�������->ȫ���˳���ֻ������һ��

	m_pInfoDlg->ShowWindow(SW_HIDE);
	
	stopClearTimer();

	stopFaceDetectThread(); // ֹͣԤ���߳�ǰ��ֹͣ���
	stopCameraThread();
	m_pAttentionDlg->setVisible(false);
	closeAllCamera();
	Sleep(20);

	waitFdvThreadStopped();
	stopIdcardDetectThread();

	// IDCardReader
	UnloadIDCardReader();

	CDialogEx::OnClose();
}




void CIDCardFdvDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CIDCardFdvDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (nIDEvent)
	{
	case CLEAR_INFOIMG_TIMER:
		waitFdvThreadStopped();	
		m_bDrawResultIconRight = m_bDrawResultIconWrong = false;//m_pInfoDlg->clearResultIcon();
		Sleep(100);
		m_pInfoDlg->ShowWindow(SW_HIDE);
		//m_pInfoDlg->clearCameraImage();	// SW_HIDE���軭��Ч����������
		//m_pInfoDlg->clearIDCardImage();		
		m_pInfoDlg->setResultText("");
		m_pInfoDlg->UpdateData(FALSE);
		m_pInfoDlg->UpdateWindow();
		stopClearTimer();
		ResetEvent(m_eCameraResume);	
		m_bDrawScan = false;
		m_pInfoDlg->enableIDCardNoBtn(true);
		m_pInfoDlg->resetIDCardNoInput();
		m_pOperationDlg->resetNoIDCardCheckIn();

		if (m_bShowAdv) {
			Sleep(100);
			m_pAttentionDlg->setVisible(false);
			// ��ʾ���
			m_HtmlView.ShowWindow(SW_SHOW);
			m_HtmlView.UpdateWindow();
			SetTimer(ADV_AND_BACK_TIMER, 1000 * 10, NULL);
		}
		else {
			Sleep(50);
			SetEvent(m_eIdcardDetectResume);
		}
		break;
	case ADV_AND_BACK_TIMER:
		KillTimer(ADV_AND_BACK_TIMER);
		// ������������
		m_HtmlView.ShowWindow(SW_HIDE);
		SetEvent(m_eIdcardDetectResume);
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CIDCardFdvDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if ((nID & 0xFFF0) == SC_MOVE)
		return;

	CDialogEx::OnSysCommand(nID, lParam);
}


void CIDCardFdvDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���

	// CDialogEx::OnOK();
}


BOOL CIDCardFdvDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE: //Esc�����¼�
			return true;
		case VK_RETURN: //Enter�����¼�
			return true;
		default:
			;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
