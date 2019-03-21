#pragma once
#include <fstream>

// CMsgDlg 对话框

class CMsgDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMsgDlg)

public:
	CMsgDlg(int centerX, int centerY,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMsgDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MSG_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	int m_iCenterX;
	int m_iCenterY;
	int m_iScreenX;
	int m_iScreenY;
	int m_iWidth;
	int m_iHeight;

	int   m_fFontSize;
	float m_fFontRate;
	bool  m_bLockMsg;
public:
	virtual BOOL OnInitDialog();

	void setFontRate(float rate) { m_fFontRate = rate; }
	void setMessage(std::string msg, float time = 1.0f/*sec*/, bool lock = false); 
	void lockMessage() { m_bLockMsg = true; }
	void unlockMessage() { m_bLockMsg = false; }
	void hide() { ShowWindow(SW_HIDE); }
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
