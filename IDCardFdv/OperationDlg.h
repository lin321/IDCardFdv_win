#pragma once


// COperationDlg �Ի���

class COperationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(COperationDlg)

public:
	COperationDlg(int screenX, int screenY, int width, int height, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~COperationDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OPERATION_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	int m_iScreenX;
	int m_iScreenY;
	int m_iWidth;
	int m_iHeight;

	float m_fFontRate;
	CFont m_font;
	bool  m_bNoIDCardCheckIn;
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnNoidcard();

	void setFontRate(float rate) { m_fFontRate = rate; }
	void resetNoIDCardCheckIn();
	bool isNoIDCardCheckIn();
};
