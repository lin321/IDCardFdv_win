#pragma once

#include "CvvImage.h"

// CAttentionDlg 对话框

class CAttentionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAttentionDlg)

public:
	CAttentionDlg(int screenX, int screenY, int width, int height, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAttentionDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ATTENTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	int m_iScreenX;
	int m_iScreenY;
	int m_iWidth;
	int m_iHeight;
	float m_fontRate;
	IplImage* m_att_icon;
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void setFontRate(float rate);
	void loadAllData();
	void setVisible(bool visible);
private:
	void drawAttentionIcon(IplImage* img);
};
