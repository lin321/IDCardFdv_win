#pragma once


// CQRScanMaskDlg 对话框

class CQRScanMaskDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CQRScanMaskDlg)

public:
	CQRScanMaskDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CQRScanMaskDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_QRSCAN_MASK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
