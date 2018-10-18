
// IDCardFdvDlg.h : 头文件
//

#pragma once

#include "CvvImage.h"
#include "InfoDlg.h"
#ifdef NDEBUG
#include "AiFdrWrap.h"
#else
#include <opencv2/objdetect/objdetect.hpp>
#endif

// CIDCardFdvDlg 对话框
class CIDCardFdvDlg : public CDialogEx
{
// 构造
public:
	CIDCardFdvDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IDCARDFDV_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	std::string m_strModulePath;

	std::string m_macId;

	//config data
	std::string m_cfgCameraVid;
	std::string m_cfgCameraPid;
	std::string m_cfgCameraHideVid;
	std::string m_cfgCameraHidePid;
	std::string m_cfgAppId;
	std::string m_cfgApiKey;
	std::string m_cfgSecretKey;
	std::string m_cfgUrl;
	std::string m_cfgTimeOut;
	std::string m_cfgRegisteredNo;

	// face detect thread
	bool m_bFaceDetectRun;
	CWinThread* m_thFaceDetect;
	CEvent m_eFaceDetectEnd;
	CEvent m_eCaptureForDetect;
	bool m_bCmdDetect;
	std::vector<cv::Rect> m_faces;

	//int camdevid;
	bool m_bCameraRun;
	CWinThread* m_thCamera;
	CEvent m_eCameraEnd;
	bool m_bFlip;
	IplImage* m_iplImgDisplay;
	IplImage* m_iplImgTemp;

	// fdv
	CWinThread* m_thFdv;
	bool m_bFdvRun;
	CEvent m_eFdvEnd;
#ifdef NDEBUG
	fdr_model_wrap* m_pfrmwrap;
#else
	cv::CascadeClassifier faceCascade;
#endif
	std::string m_photoFaceFeat;
	std::vector < std::string> m_frameFaceFeats;

	// capture
	bool m_bCmdCapture;
	IplImage* m_CaptureImage;
	IplImage* m_CaptureImageHide;
	CEvent m_eCaptureEnd;
	std::string m_sCaptureBase64;
	bool m_bFaceGot;
	bool m_bIsAliveSample;
	IplImage* m_iplImgTestImage;	//test
	IplImage* m_iplImgTestImage2;	//test

	bool m_bMainWinClose;

	// idcard
	char m_IdCardId[256];
	char m_IdCardIssuedate[256];
	char m_IdCardPhoto[102400];
	IplImage* m_iplImgPhoto;

	// info panel
	CInfoDlg* m_pInfoDlg;
	HBITMAP m_hBIconCamera;	// test
	IplImage* m_iplImgCameraImg;
	IplImage* m_iplImgResultIconRight;
	IplImage* m_iplImgResultIconWrong;
	double m_dThreshold;
public:
	void showPreview(IplImage* img);
	void drawCameraImage(IplImage* img);

	void startCameraThread();
	void stopCameraThread();
	void startFaceDetectThread();
	void stopFaceDetectThread();
	void startFdvThread();
	void waitFdvThreadStopped();
	void setClearTimer();

	virtual BOOL DestroyWindow();
	afx_msg void OnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
