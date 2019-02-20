// OperationDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "OperationDlg.h"
#include "afxdialogex.h"


// COperationDlg 对话框
#define _FSIZE(size) ((int)(size * m_fFontRate))

IMPLEMENT_DYNAMIC(COperationDlg, CDialogEx)

COperationDlg::COperationDlg(int screenX, int screenY, int width, int height, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_OPERATION_DIALOG, pParent)
{
	m_iScreenX = screenX;
	m_iScreenY = screenY;
	m_iWidth = width;
	m_iHeight = height;

	m_fFontRate = 1.0f;
	m_bNoIDCardCheckIn = false;
}

COperationDlg::~COperationDlg()
{
}

void COperationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COperationDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_NOIDCARD, &COperationDlg::OnBnClickedBtnNoidcard)
END_MESSAGE_MAP()


// COperationDlg 消息处理程序


BOOL COperationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 透明化处理
	COLORREF maskColor = RGB(255, 255, 255);   //掩码颜色
	SetWindowLong(this->GetSafeHwnd(),
		GWL_EXSTYLE,
		GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE) | 0x80000);  //设定窗体使用扩展模式 
	SetLayeredWindowAttributes(maskColor, 255, LWA_COLORKEY);

	MoveWindow(m_iScreenX, m_iScreenY, m_iWidth, m_iHeight, false);

	int rw = m_iWidth-10;
	int rh = m_iHeight-10;
	int offsetX = (m_iWidth - rw) / 2;
	int offsetY = (m_iHeight - rh) / 2;
	GetDlgItem(IDC_BTN_NOIDCARD)->MoveWindow(offsetX, offsetY, rw, rh, false);
	m_font.CreatePointFont(_FSIZE(200), "微软雅黑");
	GetDlgItem(IDC_BTN_NOIDCARD)->SetFont(&m_font);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

HBRUSH COperationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_DLG)  //对话框颜色
	{
		return (HBRUSH)::GetStockObject(WHITE_BRUSH);//透明掩码颜色
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void COperationDlg::OnBnClickedBtnNoidcard()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bNoIDCardCheckIn = true;
}

void COperationDlg::resetNoIDCardCheckIn()
{
	m_bNoIDCardCheckIn = false;
}

bool COperationDlg::isNoIDCardCheckIn()
{
	return m_bNoIDCardCheckIn;
}
