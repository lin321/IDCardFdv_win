#pragma once


// CThresholdSettingDlg �Ի���

class CThresholdSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CThresholdSettingDlg)

public:
	CThresholdSettingDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CThresholdSettingDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THRESHOLD_SETTING_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeThresholdEdit();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
};
