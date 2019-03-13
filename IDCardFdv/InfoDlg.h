#pragma once

#include "CvvImage.h"
#include "ThresholdSettingDlg.h"
#include "utility_funcs.h"
// CInfoDlg 对话框

class CInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInfoDlg)

public:
	CInfoDlg(int screenX, int screenY, int width, int height, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInfoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INFO_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnThresholdSet();
	afx_msg void OnBnClickedBtnIdcardno();
	afx_msg void OnEnChangeEditIdcardno();

private:
	int m_iScreenX;
	int m_iScreenY;
	int m_iWidth;
	int m_iHeight;

	enum {
		DRAW_TYPE_CENTER = 0,
		DRAW_TYPE_FULL,
		DRAW_TYPE_RATIO,				// 保持宽高比铺满控件
	};

	int			m_iMode;
	WORK_PTR	m_pFdvDlg;
	float		m_fFontRate;
	CFont		m_IDCardNoFont;
	bool		m_bIDCardNoReady;
	bool		m_bIDCardNoNoneText;
	int			m_iResultTextSize;
	IplImage*	m_iplImgClearImg;

private:
	void setShowMode(int mode);
	void drawImage(IplImage* img, UINT ID, int type, bool clearArea = false);
	void getItemRect(int nID, cv::Rect &rect);
public:
	enum {
		INFO_MODE_PHOTO = 0,
		INFO_MODE_INCARDNO
	};
	void setMode(int mode);
	void setFdvDlgPtr(WORK_PTR ptr) { m_pFdvDlg = ptr; }
	void setFontRate(float rate) { m_fFontRate = rate; }
	void drawCameraImage(IplImage* img);
	void drawIDCardImage(IplImage* img);
	void drawResultIcon(IplImage* img);
	void clearCameraImage();
	void clearIDCardImage();
	void clearResultIcon();
	void setResultText(std::string result);
	void setResultTextSize(int size) { m_iResultTextSize = size; }
	void setThresholdText(std::string text);
	void getInfoDlgScreenRect(cv::Rect &rect);
	void getCameraImageRect(cv::Rect &rect);
	void getResultIconRect(cv::Rect &rect);
	bool isIDCardNoReady() {return m_bIDCardNoReady;}
	void resetIDCardNoInput();
	std::string getIDCardNo();
	void enableIDCardNoBtn(bool enable);
	afx_msg void OnEnSetfocusEditIdcardno();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnKillfocusEditIdcardno();
};
