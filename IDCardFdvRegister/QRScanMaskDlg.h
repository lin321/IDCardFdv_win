#pragma once


// CQRScanMaskDlg �Ի���

class CQRScanMaskDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CQRScanMaskDlg)

public:
	CQRScanMaskDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CQRScanMaskDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_QRSCAN_MASK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

private:
	int m_iScanAreaSize;

public:
	void setScanAreaSize(int size);
	int getScanAreaSize();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
