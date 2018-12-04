// AttentionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "AttentionDlg.h"
#include "afxdialogex.h"
#include "utility_funcs.h"

#include <fstream>

using namespace std;
// CAttentionDlg 对话框

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


// CAttentionDlg 消息处理程序
BOOL CAttentionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
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
				  // 异常: OCX 属性页应返回 FALSE
}


#define _FSIZE(size) ((int)(size * m_fontRate))
HBRUSH CAttentionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	CBrush *bgcolor;
	bgcolor = new CBrush(RGB(74, 144, 226));

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_DLG)  //对话框颜色
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
		font.CreatePointFont(_FSIZE(270), "黑体");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(74, 144, 226));
		pDC->SetTextColor(RGB(255, 255, 255));

		return (HBRUSH)(bgcolor->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_ATT_TITLE02)
	{
		CFont font;
		font.CreatePointFont(_FSIZE(360), "黑体");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(74, 144, 226));
		pDC->SetTextColor(RGB(255, 255, 255));

		return (HBRUSH)(bgcolor->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_ATT_MAIN01)
	{
		CFont font;
		font.CreatePointFont(_FSIZE(320), "黑体");
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
		font.CreatePointFont(_FSIZE(270), "黑体");
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
		font.CreatePointFont(_FSIZE(255), "黑体");
		pDC->SelectObject(&font);
		pDC->SetBkColor(RGB(74, 144, 226));
		pDC->SetTextColor(RGB(255, 255, 255));

		return (HBRUSH)(bgcolor->m_hObject);
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CAttentionDlg::drawAttentionIcon(IplImage* img)
{
	CWnd* pWnd = GetDlgItem(IDC_ATT_ICON);
	if (NULL == pWnd)
		return;

	CDC* pDC = pWnd->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作

	CRect rect;

	pWnd->GetClientRect(&rect);
	int rw = rect.right - rect.left;			// 求出图片控件的宽和高
	int rh = rect.bottom - rect.top;

	int iw = img->width;						// 读取图片的宽和高
	int ih = img->height;

	SetRect(rect, 0, 0, rw, rh);	// 铺满控件

	CvvImage cimg;
	cimg.CopyOf(img, -1);							// 复制图片
	cimg.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内

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
