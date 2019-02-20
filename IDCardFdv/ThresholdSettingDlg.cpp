// ThresholdSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "ThresholdSettingDlg.h"
#include "afxdialogex.h"

#include "IDCardFdvDlg.h"
#include "InfoDlg.h"

// CThresholdSettingDlg 对话框

IMPLEMENT_DYNAMIC(CThresholdSettingDlg, CDialogEx)

CThresholdSettingDlg::CThresholdSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_THRESHOLD_SETTING_DIALOG, pParent)
{

}

CThresholdSettingDlg::~CThresholdSettingDlg()
{
}

void CThresholdSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CThresholdSettingDlg, CDialogEx)
	ON_EN_CHANGE(IDC_THRESHOLD_EDIT, &CThresholdSettingDlg::OnEnChangeThresholdEdit)
	ON_BN_CLICKED(IDOK, &CThresholdSettingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CThresholdSettingDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CThresholdSettingDlg 消息处理程序


BOOL CThresholdSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	GetDlgItem(IDC_THRESHOLD_EDIT)->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
				   // 异常: OCX 属性页应返回 FALSE
}


void CThresholdSettingDlg::OnEnChangeThresholdEdit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString csAreaS;
	GetDlgItem(IDC_THRESHOLD_EDIT)->GetWindowText(csAreaS);
	// 只允许输数据  
	int nStringLength = csAreaS.GetLength();
	int nDotCount = 0;
	// 点字符不能多于1个  
	for (int nIndex = 0; nIndex < nStringLength; nIndex++)
	{
		if (csAreaS[nIndex] == '.')
		{
			nDotCount++;
			if (nDotCount > 1)
			{
				CString csTmp;
				csTmp = csAreaS.Left(nIndex);
				csTmp += csAreaS.Right(csAreaS.GetLength() - nIndex - 1);
				//csRadius = csRadius.Left( nIndex + 1 ) + csRadius.Right( nStringLength - ( nIndex + 1 ) - 1 );  
				GetDlgItem(IDC_THRESHOLD_EDIT)->SetWindowText(csTmp);
				return;
			}
		}
	}

	// 不允许输入数字和点以外的字符  
	for (int nIndex = 0; nIndex < nStringLength; nIndex++)
	{
		if (csAreaS[nIndex] != '.' && (csAreaS[nIndex] > '9' || csAreaS[nIndex] < '0'))
		{
			csAreaS = csAreaS.Left(nIndex) + csAreaS.Right(csAreaS.GetLength() - nIndex - 1);
			GetDlgItem(IDC_THRESHOLD_EDIT)->SetWindowText(csAreaS);
			return;
		}
	}
}

void CThresholdSettingDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CIDCardFdvDlg *p = dynamic_cast<CIDCardFdvDlg*>(this->GetParent()->GetParent());
	if (NULL != p) {
		CString csAreaS;
		GetDlgItem(IDC_THRESHOLD_EDIT)->GetWindowText(csAreaS);
		string text = csAreaS.GetString();
		if ("" == text) {
			AfxMessageBox("请输入数值！");
			return;
		}
		double threshold = std::stod(csAreaS.GetString());
		if (threshold > 100)
			threshold = 100.0;
		p->m_dThreshold = threshold;
		CInfoDlg *pInfo = dynamic_cast<CInfoDlg*>(this->GetParent());
		if (NULL != pInfo) {
			CString thstr;
			thstr.Format("%.2f%%", threshold);
			pInfo->setThresholdText(thstr.GetString());
		}
	}
	CDialogEx::OnOK();
}


void CThresholdSettingDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}





void CThresholdSettingDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::OnOK();
}
