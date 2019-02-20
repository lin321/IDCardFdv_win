// OperationDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "OperationDlg.h"
#include "afxdialogex.h"


// COperationDlg �Ի���
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


// COperationDlg ��Ϣ�������


BOOL COperationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	// ͸��������
	COLORREF maskColor = RGB(255, 255, 255);   //������ɫ
	SetWindowLong(this->GetSafeHwnd(),
		GWL_EXSTYLE,
		GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE) | 0x80000);  //�趨����ʹ����չģʽ 
	SetLayeredWindowAttributes(maskColor, 255, LWA_COLORKEY);

	MoveWindow(m_iScreenX, m_iScreenY, m_iWidth, m_iHeight, false);

	int rw = m_iWidth-10;
	int rh = m_iHeight-10;
	int offsetX = (m_iWidth - rw) / 2;
	int offsetY = (m_iHeight - rh) / 2;
	GetDlgItem(IDC_BTN_NOIDCARD)->MoveWindow(offsetX, offsetY, rw, rh, false);
	m_font.CreatePointFont(_FSIZE(200), "΢���ź�");
	GetDlgItem(IDC_BTN_NOIDCARD)->SetFont(&m_font);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

HBRUSH COperationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (nCtlColor == CTLCOLOR_DLG)  //�Ի�����ɫ
	{
		return (HBRUSH)::GetStockObject(WHITE_BRUSH);//͸��������ɫ
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void COperationDlg::OnBnClickedBtnNoidcard()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
