// QRScanMaskDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdvRegister.h"
#include "QRScanMaskDlg.h"
#include "afxdialogex.h"


// CQRScanMaskDlg 对话框

IMPLEMENT_DYNAMIC(CQRScanMaskDlg, CDialogEx)

CQRScanMaskDlg::CQRScanMaskDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_QRSCAN_MASK, pParent)
{
	m_iScanAreaSize = 200;
}

CQRScanMaskDlg::~CQRScanMaskDlg()
{
}

void CQRScanMaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CQRScanMaskDlg, CDialogEx)

	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CQRScanMaskDlg 消息处理程序


BOOL CQRScanMaskDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 透明化处理
	COLORREF maskColor = RGB(255, 255, 255);   //掩码颜色
	SetWindowLong(this->GetSafeHwnd(),
		GWL_EXSTYLE,
		GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE) | 0x80000);  //设定窗体使用扩展模式 
	SetLayeredWindowAttributes(maskColor, 128, LWA_COLORKEY | LWA_ALPHA);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



void CQRScanMaskDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()

	CRect rect;
	GetClientRect(&rect);

	int l = (rect.right - m_iScanAreaSize) / 2;
	int t = (rect.bottom - m_iScanAreaSize) / 2;
	int w = m_iScanAreaSize;
	int h = m_iScanAreaSize;
	int r = l + w;
	int b = t + h;
	int linew = 20;
	int lineh = 4;
	CBrush base(RGB(255, 255, 255));
	CBrush color(RGB(0, 0, 255));
	dc.FillRect(CRect(l, t, r, b),&base);
	dc.FillRect(CRect(l, t, l + linew, t + lineh), &color);
	dc.FillRect(CRect(l, t, l + lineh, t + linew), &color);

	dc.FillRect(CRect(r - linew, t, r, t + lineh), &color);
	dc.FillRect(CRect(r - lineh, t, r, t + linew), &color);

	dc.FillRect(CRect(l, b - linew, l + lineh, b), &color);
	dc.FillRect(CRect(l, b - lineh, l + linew, b), &color);

	dc.FillRect(CRect(r - linew, b - lineh, r, b), &color);
	dc.FillRect(CRect(r - lineh, b - linew, r, b), &color);
}

void CQRScanMaskDlg::setScanAreaSize(int size)
{
	m_iScanAreaSize = size;
}

int CQRScanMaskDlg::getScanAreaSize()
{
	return m_iScanAreaSize;
}

HBRUSH CQRScanMaskDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_DLG)   
	{
		CBrush *brush;
		brush = new CBrush(RGB(0, 0, 0));
		return (HBRUSH)(brush->m_hObject);
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
