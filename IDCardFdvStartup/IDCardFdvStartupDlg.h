
// IDCardFdvStartupDlg.h : ͷ�ļ�
//

#pragma once
#include <fstream>

// CIDCardFdvStartupDlg �Ի���
class CIDCardFdvStartupDlg : public CDialogEx
{
// ����
public:
	CIDCardFdvStartupDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IDCARDFDVSTARTUP_DIALOG };
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
