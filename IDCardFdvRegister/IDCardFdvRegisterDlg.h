
// IDCardFdvRegisterDlg.h : ͷ�ļ�
//

#pragma once
#include <string>

#include "CvvImage.h"
#include "QRScanMaskDlg.h"

// CIDCardFdvRegisterDlg �Ի���
class CIDCardFdvRegisterDlg : public CDialogEx
{
// ����
public:
	CIDCardFdvRegisterDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IDCARDFDVREGISTER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()

public:
	std::string m_strModulePath;

	//config data
	std::string m_cfgCameraVid;
	std::string m_cfgCameraPid;
	std::string m_cfgAppId;
	std::string m_cfgApiKey;
	std::string m_cfgSecretKey;
	std::string m_cfgUrl;
	std::string m_cfgTimeOut;
	std::string m_cfgRegisterSN;
	std::string m_cfgRegisteredNo;

	int camdevid;
	bool m_bCameraRun;
	int m_iQRbtnState;
	CWinThread* m_thCamera;
	CEvent m_eCameraEnd;
	bool m_bFlip;
	IplImage* m_iplImgDisplay;
	IplImage* m_iplImgTemp;

	int qrScanCnt;
	int qrScanCntMax;
	IplImage* m_iplImgQRimg;
	IplImage* m_iplImgQRimgGray;
	CQRScanMaskDlg* m_pScanMaskDlg;

public:
	void showPreview(IplImage* img);
	void startCameraThread();
	void stopCameraThread();

	void setProductSnText(std::string str);
	void saveConfig();
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnReg();
	virtual BOOL DestroyWindow();
	afx_msg void OnBnClickedBtnQrscan();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnClose();
};
