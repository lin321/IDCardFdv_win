
// IDCardFdvStartupDlg.h : 头文件
//

#pragma once
#include <fstream>

// CIDCardFdvStartupDlg 对话框
class CIDCardFdvStartupDlg : public CDialogEx
{
// 构造
public:
	CIDCardFdvStartupDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IDCARDFDVSTARTUP_DIALOG };
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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

private:
	CWinThread* m_thWork;

public:
	std::string m_strModulePath;
	CStatic* m_pStaticMsg;

public:
	void errorExit(std::string msg);
};
