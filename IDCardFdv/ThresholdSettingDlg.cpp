// ThresholdSettingDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "ThresholdSettingDlg.h"
#include "afxdialogex.h"

#include "IDCardFdvDlg.h"
#include "InfoDlg.h"

// CThresholdSettingDlg �Ի���

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


// CThresholdSettingDlg ��Ϣ�������


BOOL CThresholdSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	GetDlgItem(IDC_THRESHOLD_EDIT)->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
				   // �쳣: OCX ����ҳӦ���� FALSE
}


void CThresholdSettingDlg::OnEnChangeThresholdEdit()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString csAreaS;
	GetDlgItem(IDC_THRESHOLD_EDIT)->GetWindowText(csAreaS);
	// ֻ����������  
	int nStringLength = csAreaS.GetLength();
	int nDotCount = 0;
	// ���ַ����ܶ���1��  
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

	// �������������ֺ͵�������ַ�  
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();
	CIDCardFdvDlg *p = dynamic_cast<CIDCardFdvDlg*>(this->GetParent()->GetParent());
	if (NULL != p) {
		CString csAreaS;
		GetDlgItem(IDC_THRESHOLD_EDIT)->GetWindowText(csAreaS);
		string text = csAreaS.GetString();
		if ("" == text) {
			AfxMessageBox("��������ֵ��");
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
}





void CThresholdSettingDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���

	CDialogEx::OnOK();
}
