
#include "stdafx.h"
#include <afxmt.h> 
#include <opencv2/objdetect/objdetect.hpp>
#include "MTLibCameraLib.h"
#include "CvvImage.h"
#include "utility_funcs.h"
#ifdef NDEBUG
#include "AiFdrWrap.h"
#endif

#define OPENCV_CAPTURE 0
using namespace cv;

HWND preview_hWnd = NULL;
MTLibFaceImageCallBack MTLibFaceImageCB = NULL;
MTLibFaceResultCallBack MTLibFaceResultCB = NULL;

#if OPENCV_CAPTURE
class cv_cap_class {
public:
#ifdef NDEBUG
	fdr_model_wrap* m_pfrmwrap = NULL;
#endif
	CEvent m_eCameraEnd;
	bool m_bCameraRun;
	CWinThread* m_thCamera;
	cv::CascadeClassifier faceCascade;
};
cv_cap_class* cvcap = NULL;
#endif

void startCameraThread();

string ExtractFilePath(const string& szFile)
{
	if (szFile == "")
		return "";

	size_t idx = szFile.find_last_of("\\:");

	if (-1 == idx)
		return "";
	return string(szFile.begin(), szFile.begin() + idx + 1);
}


static VOID __stdcall FaceImageCB(HWND hWnd, BSTR imgBase64, ULONG_PTR userdata)
{
	if (NULL != MTLibFaceImageCB) {
		MTLibFaceImageCB(hWnd, imgBase64, userdata);
	}

	ZZReleaseString(imgBase64);
}

static VOID __stdcall FaceResultCB(HWND hWnd, LONG result, BSTR feature, ULONG_PTR userdata)
{
	if (NULL != MTLibFaceResultCB) {
		MTLibFaceResultCB(hWnd, result, userdata);
	}

	ZZReleaseString(feature);
}



int __stdcall MTLibLoadCamera()
{
#if OPENCV_CAPTURE
	cvcap = new cv_cap_class();
	// opencv face detect
	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	std::string strModulePath = ExtractFilePath(szPath);
	std::string faceCascadeFilename = strModulePath + "haarcascade_frontalface_default.xml";
	try {
		cvcap->faceCascade.load(faceCascadeFilename);
	}
	catch (cv::Exception e) {}
	if (cvcap->faceCascade.empty()) {
		AfxMessageBox("haarcascade_frontalface_default.xml加载出错！");
	}
	cvcap->m_bCameraRun = false;
	cvcap->m_thCamera = NULL;
#else
	ZZInitFaceMgr();
#endif

	return 0;
}

int __stdcall MTLibSetLiveChecker(ULONG_PTR checker)
{
#if OPENCV_CAPTURE
#ifdef NDEBUG
	if(cvcap)
		cvcap->m_pfrmwrap = (fdr_model_wrap*)checker;
#endif
#endif

	return 0;
}


int __stdcall MTLibOpenCamera(HWND hWnd,
	MTLibFaceImageCallBack fiCB, ULONG_PTR fi_userdata,
	MTLibFaceResultCallBack frCB, ULONG_PTR fr_userdata)
{
	preview_hWnd = hWnd;
	MTLibFaceImageCB = fiCB;
	MTLibFaceResultCB = frCB;

#if OPENCV_CAPTURE
	if (!cvcap)
		return;

	startCameraThread();
#else
	ZZOpenDevice(preview_hWnd, 2);
	ZZOpenHideDevice(preview_hWnd, 4);
	ZZOpenVideo(preview_hWnd);
	ZZOpenHideVideo(preview_hWnd);
	ZZGetFaceFeature(preview_hWnd, FaceImageCB, fi_userdata, 2, FaceResultCB, fr_userdata);
#endif

	return 0;
}

int __stdcall MTLibCloseCamera()
{
	ZZStopGetFace(preview_hWnd);
	ZZCloseHideVideo(preview_hWnd);
	ZZCloseVideo(preview_hWnd);
	ZZCloseHideDevice(preview_hWnd);
	ZZCloseDevice(preview_hWnd);

	preview_hWnd = NULL;
	MTLibFaceImageCB = NULL;
	MTLibFaceResultCB = NULL;
	return 0;
}

int __stdcall MTLibUnloadCamera()
{
#if OPENCV_CAPTURE
	delete cvcap;
	cvcap = NULL;
#else
	ZZDeinitFaceMgr();
#endif

	return 0;
}

///////////////////////////////////////////////////////////
#if OPENCV_CAPTURE
UINT CameraShowThread(LPVOID lpParam)
{
	
	if (!cvcap)
		return;

	int mainDevIdx = getDeviceIndex("2AB8", "A101");
	int hideDevIdx = getDeviceIndex("2AB8", "C101");
	if (-1 == mainDevIdx)
		mainDevIdx = 0;

	Mat cFrame;
	VideoCapture captureMain;
	captureMain.open(mainDevIdx, CAP_DSHOW);
	if (!captureMain.isOpened()) {
		SetEvent(cvcap->m_eCameraEnd);
		return 0;
	}
	captureMain.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	captureMain.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	Mat cFrameHide;
	VideoCapture captureHide;
	if (hideDevIdx >= 0) {
		captureHide.open(hideDevIdx, CAP_DSHOW);
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
			captureMain.open(mainDevIdx);
			if (!captureMain.isOpened()) {
				Sleep(2000);
				continue;
			}
			captureMain.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
			captureMain.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
			Sleep(200);
		}
		if (hideDevIdx >= 0 && !captureHide.isOpened()) {
			captureHide.open(hideDevIdx, CAP_DSHOW);
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
			//pDlg->MessageBox("读取图像帧错误！", "出错信息：", MB_ICONERROR | MB_OK);
			continue;
		}
		if (captureHide.isOpened()) {
			// 避免其他处理用时造成的误差，先在此读入
			captureHide.read(cFrameHide);
		}

		IplImage* newframe = &IplImage(cFrame);
		//
		//clock_t time1 = clock();
		//std::vector < std::vector<int>> face_rects;
		//Mat matframe(newframe);
		//pDlg->m_pfrmwrap->dectect_faces(matframe, face_rects, 0, true);
		//clock_t dt = clock() - time1;
		//CString csTemp;
		//csTemp.Format("%d", dt);
		//AfxMessageBox(csTemp);
		
		//clock_t time1 = clock();
		double scale = 2.0;
		Mat imgGray, imgSamll;
		cvtColor(cFrame, imgGray, CV_RGB2GRAY);
		resize(imgGray, imgSamll, Size(), 1 / scale, 1 / scale, INTER_NEAREST);
		equalizeHist(imgSamll, imgSamll);	// 直方图均衡化，提高图像质量
		vector<Rect> faces;
		if (!pDlg->faceCascade.empty()) {
			int flags = CASCADE_DO_CANNY_PRUNING |
				CASCADE_SCALE_IMAGE |
				CASCADE_FIND_BIGGEST_OBJECT |
				CASCADE_DO_ROUGH_SEARCH;
			pDlg->faceCascade.detectMultiScale(imgSamll, faces, 2.0, 2, flags, Size(60, 60));    // 检测人脸
		}
		//clock_t dt = clock() - time1;
		//CString csTemp;
		//csTemp.Format("%d", dt);
		//AfxMessageBox(csTemp);

		int facex, facey;
		CvSize FaceImgSize;
		if (faces.size()>0)
		{
			// 开启识别线程
			pDlg->ProcessCapture();

			// 防止人脸框频繁抖动
			static Rect biggestface;
			if (abs(faces[0].x - biggestface.x) < 5 && abs(faces[0].y - biggestface.y) < 5
				&& abs((faces[0].x + faces[0].width) - (biggestface.x + biggestface.width)) < 5
				&& abs((faces[0].y + faces[0].height) - (biggestface.y + biggestface.height)) < 5) {
				faces[0] = biggestface;
			}
			else
				biggestface = faces[0];

			facex = (int)(faces[0].x * scale);
			facey = (int)(faces[0].y * scale - faces[0].height * scale * 0.1);
			if (facex < 0) facex = 0;
			if (facey < 0) facey = 0;

			FaceImgSize.width = (int)(faces[0].width * scale * 1.0);
			FaceImgSize.height = (int)(faces[0].height * scale * 1.2);
		}



		if (pDlg->m_bCmdCapture && faces.size()>0) {
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
		if (faces.size() > 0) {
			//for (int i = 0; i<faces.size(); i++)
			{
				int i = 0;
				rectangle(cvarrToMat(newframe),
					Point(facex, facey),
					Point(facex + FaceImgSize.width, facey + FaceImgSize.height),
					Scalar(0, 255, 0), 2, LINE_8);    // 框出人脸
			}
		}

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

void startCameraThread()
{
	if (!cvcap)
		return;

	ResetEvent(cvcap->m_eCameraEnd);

	if (cvcap->m_thCamera == NULL) {
		cvcap->m_thCamera = AfxBeginThread(CameraShowThread, NULL);
		if (NULL == cvcap->m_thCamera)
		{
			TRACE("创建新的线程出错！\n");
			return;
		}
	}
}
#endif

