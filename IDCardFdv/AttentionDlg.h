#pragma once

#include "CvvImage.h"

// CAttentionDlg �Ի���

class CAttentionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAttentionDlg)

public:
	CAttentionDlg(int screenX, int screenY, int width, int height, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAttentionDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ATTENTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
