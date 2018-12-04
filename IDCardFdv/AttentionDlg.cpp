// AttentionDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "AttentionDlg.h"
#include "afxdialogex.h"
#include "utility_funcs.h"

#include <fstream>

using namespace std;
// CAttentionDlg �Ի���

IMPLEMENT_DYNAMIC(CAttentionDlg, CDialogEx)

CAttentionDlg::CAttentionDlg(int screenX, int screenY, int width, int height, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ATTENTION_DIALOG, pParent)
{
	m_iScreenX = screenX;
	m_iScreenY = screenY;
	m_iWidth = width;
	m_iHeight = height;

	m_fontRate = 1.0f;
	m_att_icon = NULL;
}

CAttentionDlg::~CAttentionDlg()
{
	if (m_att_icon != NULL) {
		cvReleaseImage(&m_att_icon);
		m_att_icon = NULL;
	}
}

void CAttentionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAttentionDlg, CDialogEx)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CAttentionDlg ��Ϣ�������
BOOL CAttentionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	MoveWindow(m_iScreenX, m_iScreenY, m_iWidth, m_iHeight, false);

	int offsetX = (int)(m_iWidth * 17 * 1.0f / 768);
	int offsetY = (int)(m_iHeight * 70 * 1.0f / 1080);
	int rw = (int)(m_iWidth * 188 * 1.0f / 768);
	int rh = rw;
	GetDlgItem(IDC_ATT_ICON)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetX = (int)(m_iWidth * 10 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 366 * 1.0f / 1080);
	rw = m_iWidth - offsetX * 2;
	rh = (int)(m_iHeight * 443 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_MAIN_BG)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetX = (int)(m_iWidth * 241 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 71 * 1.0f / 1080);
	rw = m_iWidth - offsetX;
	rh = (int)(m_iHeight * 70 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_TITLE01)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetX = (int)(m_iWidth * 241 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 165 * 1.0f / 1080);
	rw = m_iWidth - offsetX;
	rh = (int)(m_iHeight * 92 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_TITLE02)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetX = (int)(m_iWidth * 35 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 388 * 1.0f / 1080);
	rw = m_iWidth - offsetX - (int)(m_iWidth * 10 * 1.0f / 768);
	rh = (int)(m_iHeight * 84 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_MAIN01)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetX = (int)(m_iWidth * 111 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 525 * 1.0f / 1080);
	rw = m_iWidth - offsetX - (int)(m_iWidth * 10 * 1.0f / 768);
	rh = (int)(m_iHeight * 70 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_MAIN02)->MoveWindow(offsetX, offsetY, rw, rh, false);

	//offsetX = (int)(m_iWidth * 111 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 595 * 1.0f / 1080);
	rw = m_iWidth - offsetX - (int)(m_iWidth * 10 * 1.0f / 768);
	rh = (int)(m_iHeight * 70 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_MAIN03)->MoveWindow(offsetX, offsetY, rw, rh, false);

	//offsetX = (int)(m_iWidth * 111 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 665 * 1.0f / 1080);
	rw = m_iWidth - offsetX - (int)(m_iWidth * 10 * 1.0f / 768);
	rh = (int)(m_iHeight * 70 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_MAIN04)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetX = (int)(m_iWidth * 26 * 1.0f / 768);
	offsetY = (int)(m_iHeight * 918 * 1.0f / 1080);
	rw = m_iWidth - offsetX * 2;
	rh = (int)(m_iHeight * 67 * 1.0f / 1080);
	GetDlgItem(IDC_ATT_COMPANY)->MoveWindow(offsetX, offsetY, rw, rh, false);

	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	string strModulePath = ExtractFilePath(szPath);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


#define _FSIZE(size) ((int)(size * m_fontRate))
HBRUSH CAttentionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	CBrush *bgcolor;
	bgcolor = new CBrush(RGB(74, 144, 226));

	// TODO:  �ڴ˸��� DC ���κ�����
	if (nCtlColor == CTLCOLOR_DLG)  //�Ի�����ɫ
	{
		return (HBRUSH)(bgcolor->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_ATT_MAIN_BG)
	{
		CBrush *brush;
		brush = new CBrush(RGB(255, 255, 255));
		return (HBRUSH)(brush->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_ATT_TITLE01)
	{
		CFont font;
		font.CreatePointFont(_FSIZE(270), "����");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(74, 144, 226));
		pDC->SetTextColor(RGB(255, 255, 255));

		return (HBRUSH)(bgcolor->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_ATT_TITLE02)
	{
		CFont font;
		font.CreatePointFont(_FSIZE(360), "����");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(74, 144, 226));
		pDC->SetTextColor(RGB(255, 255, 255));

		return (HBRUSH)(bgcolor->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_ATT_MAIN01)
	{
		CFont font;
		font.CreatePointFont(_FSIZE(320), "����");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetTextColor(RGB(0, 0, 0));

		CBrush *brush;
		brush = new CBrush(RGB(255, 255, 255));
		return (HBRUSH)(brush->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC &&
		 (pWnd->GetDlgCtrlID() == IDC_ATT_MAIN02 ||
		  pWnd->GetDlgCtrlID() == IDC_ATT_MAIN03 ||
		  pWnd->GetDlgCtrlID() == IDC_ATT_MAIN04)
		)
	{
		CFont font;
		font.CreatePointFont(_FSIZE(270), "����");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetTextColor(RGB(74, 144, 226));

		CBrush *brush;
		brush = new CBrush(RGB(255, 255, 255));
		return (HBRUSH)(brush->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_ATT_COMPANY)
	{
		CFont font;
		font.CreatePointFont(_FSIZE(255), "����");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(74, 144, 226));
		pDC->SetTextColor(RGB(255, 255, 255));

		return (HBRUSH)(bgcolor->m_hObject);
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void CAttentionDlg::drawAttentionIcon(IplImage* img)
{
	CWnd* pWnd = GetDlgItem(IDC_ATT_ICON);
	if (NULL == pWnd)
		return;

	CDC* pDC = pWnd->GetDC();		// �����ʾ�ؼ��� DC
	HDC hDC = pDC->GetSafeHdc();				// ��ȡ HDC(�豸���) �����л�ͼ����

	CRect rect;

	pWnd->GetClientRect(&rect);
	int rw = rect.right - rect.left;			// ���ͼƬ�ؼ��Ŀ�͸�
	int rh = rect.bottom - rect.top;

	int iw = img->width;						// ��ȡͼƬ�Ŀ�͸�
	int ih = img->height;

	SetRect(rect, 0, 0, rw, rh);	// �����ؼ�

	CvvImage cimg;
	cimg.CopyOf(img, -1);							// ����ͼƬ
	cimg.DrawToHDC(hDC, &rect);				// ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������

	ReleaseDC(pDC);
}

void CAttentionDlg::setFontRate(float rate)
{
	m_fontRate = rate;
}

void CAttentionDlg::loadAllData()
{
	char szPath[1024] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	std::string strModulePath = ExtractFilePath(szPath);
	std::string fn = strModulePath + "attdlg_icon.png";
	m_att_icon = cvLoadImage(fn.c_str(), CV_LOAD_IMAGE_UNCHANGED);
}

void CAttentionDlg::setVisible(bool visible)
{
	if (visible) {
		ShowWindow(SW_SHOW);
		drawAttentionIcon(m_att_icon);
	}
	else {
		ShowWindow(SW_HIDE);
	}
}
