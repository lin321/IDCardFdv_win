
// IDCardFdvDlg.h : ͷ�ļ�
//

#pragma once

#include <opencv2/objdetect/objdetect.hpp>
#include "CvvImage.h"
#include "InfoDlg.h"
#ifdef NDEBUG
#include "AiFdrWrap.h"
#endif

// CIDCardFdvDlg �Ի���
class CIDCardFdvDlg : public CDialogEx
{
// ����
public:
	CIDCardFdvDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IDCARDFDV_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	std::string m_strModulePath;

	std::string m_macId;

	//config data
	std::string m_cfgAppId;
	std::string m_cfgApiKey;
	std::string m_cfgSecretKey;
	std::string m_cfgUrl;
	std::string m_cfgTimeOut;
	std::string m_cfgRegisteredNo;

	//int camdevid;
	bool m_bCameraRun;
	CWinThread* m_thCamera;
	CEvent m_eCameraEnd;
	bool m_bFlip;
	IplImage* m_iplImgDisplay;
	IplImage* m_iplImgTemp;

	// fdv
	cv::CascadeClassifier faceCascade;
	CWinThread* m_thFdv;
	bool m_bFdvRun;
#ifdef NDEBUG
	fdr_model_wrap* m_pfrmwrap;
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
	void startCameraThread();
	void stopCameraThread();

	void drawCameraImage(IplImage* img);

	void ProcessCapture();
	void setClearTimer();

	virtual BOOL DestroyWindow();
	afx_msg void OnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
