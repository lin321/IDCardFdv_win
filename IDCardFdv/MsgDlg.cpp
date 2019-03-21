// MsgDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "MsgDlg.h"
#include "afxdialogex.h"

using namespace std;

// CMsgDlg 对话框
#define _FSIZE(size) ((int)(size * m_fFontRate))


IMPLEMENT_DYNAMIC(CMsgDlg, CDialogEx)

CMsgDlg::CMsgDlg(int centerX, int centerY, CWnd* pParent)
	: CDialogEx(IDD_MSG_DIALOG, pParent)
{
	m_iCenterX = centerX;
	m_iCenterY = centerY;

	m_iScreenX = 0;
	m_iScreenY = 0;
	m_iWidth = 0;
	m_iHeight = 0;

	m_fFontSize = 40;
	m_fFontRate = 1.0f;

	m_bLockMsg = false;
}

CMsgDlg::~CMsgDlg()
{
}

void CMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMsgDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMsgDlg 消息处理程序


BOOL CMsgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CMsgDlg::setMessage(std::string msg, float time, bool lock)
{
	if (m_bLockMsg)
		return;

	if (lock)
		lockMessage();

//	ShowWindow(SW_HIDE);
	const char* cmsg = msg.c_str();
	size_t msglen = msg.size();	

	m_iHeight = m_fFontSize + 4;
	m_iWidth = (int)(msglen * (m_fFontSize * 0.4f) + 4);
	m_iScreenX = m_iCenterX - m_iWidth / 2;
	m_iScreenY = m_iCenterY - m_iHeight / 2;
	MoveWindow(m_iScreenX, m_iScreenY, _FSIZE(m_iWidth), _FSIZE(m_iHeight), false);
	GetDlgItem(IDC_STATIC_MSG)->MoveWindow(0, 0, _FSIZE(m_iWidth), _FSIZE(m_iHeight), false);
	GetDlgItem(IDC_STATIC_MSG)->SetWindowText(msg.c_str());
	ShowWindow(SW_SHOW);

	KillTimer(0);
	if (time > 0) {	
		int m = (int)(1000 * time);
		SetTimer(0, m, NULL);
	}
}


HBRUSH CMsgDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_STATIC_MSG)
	{
		CFont font;
		font.CreateFont(_FSIZE(m_fFontSize), 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
			"微软雅黑");
		pDC->SelectObject(&font);
		pDC->SetTextColor(RGB(74, 144, 226));
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CMsgDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 0:
		hide();
		KillTimer(0);
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}
