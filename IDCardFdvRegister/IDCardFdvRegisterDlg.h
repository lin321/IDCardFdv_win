
// IDCardFdvRegisterDlg.h : ͷ�ļ�
//

#pragma once
#include <string>

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
	std::string m_cfgAppId;
	std::string m_cfgApiKey;
	std::string m_cfgSecretKey;
	std::string m_cfgUrl;
	std::string m_cfgRegisteredNo;

public:
	void saveConfig();
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnReg();
};
