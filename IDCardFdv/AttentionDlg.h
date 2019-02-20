#pragma once

#include "CvvImage.h"

// CAttentionDlg �Ի���

class CAttentionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAttentionDlg)

public:
	CAttentionDlg(int screenX, int screenY, int width, int height, std::string strMain, CWnd* pParent = NULL);   // ��׼���캯��
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
	std::string m_strMain;
	float m_fFontRate;
	IplImage* m_att_icon;
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void setDepartment(std::string dep);
	void setFontRate(float rate) { m_fFontRate = rate; }
	void loadAllData();
	void setVisible(bool visible);
private:
	void drawAttentionIcon(IplImage* img);
};
