#pragma once

#include "CvvImage.h"
#include "ThresholdSettingDlg.h"
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

	int m_iResultTextSize;
	IplImage* m_iplImgClearImg;

private:
	void drawImage(IplImage* img, UINT ID, int type, bool clearArea = false);
public:
	void drawCameraImage(IplImage* img);
	void drawIdcardImage(IplImage* img);
	void drawResultIcon(IplImage* img);
	void clearCameraImage();
	void clearIdcardImage();
	void clearResultIcon();
	void setResultText(std::string result);
	void setResultTextSize(int size) { m_iResultTextSize = size; }
	void setThresholdText(std::string text);
};
