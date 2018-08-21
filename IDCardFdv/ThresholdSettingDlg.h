#pragma once


// CThresholdSettingDlg 对话框

class CThresholdSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CThresholdSettingDlg)

public:
	CThresholdSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CThresholdSettingDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THRESHOLD_SETTING_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeThresholdEdit();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
};
