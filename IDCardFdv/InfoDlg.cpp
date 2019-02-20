// InfoDlg.cpp : ʵ���ļ�
//
#include "stdafx.h"
#include "IDCardFdv.h"
#include "IDCardFdvDlg.h"
#include "InfoDlg.h"
#include "afxdialogex.h"
#include <regex>

using namespace std;
using namespace cv;
// CInfoDlg �Ի���
#define _FSIZE(size) ((int)(size * m_fFontRate))

IMPLEMENT_DYNAMIC(CInfoDlg, CDialogEx)

CInfoDlg::CInfoDlg(int screenX, int screenY, int width, int height, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INFO_DIALOG, pParent)
{
	m_iScreenX = screenX;
	m_iScreenY = screenY;
	m_iWidth = width;
	m_iHeight = height;

	m_iMode = INFO_MODE_PHOTO;
	m_pFdvDlg = NULL;
	m_fFontRate = 1.0f;
	m_bIDCardNoReady = false;
	m_bIDCardNoNoneText = true;
	m_iResultTextSize = 60;
	m_iplImgClearImg = NULL;
}

CInfoDlg::~CInfoDlg()
{
	if (m_iplImgClearImg != NULL) {
		cvReleaseImage(&m_iplImgClearImg);
		m_iplImgClearImg = NULL;
	}
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_THRESHOLD_SET, &CInfoDlg::OnBnClickedBtnThresholdSet)
	ON_BN_CLICKED(IDC_BTN_IDCARDNO, &CInfoDlg::OnBnClickedBtnIdcardno)
	ON_EN_CHANGE(IDC_EDIT_IDCARDNO, &CInfoDlg::OnEnChangeEditIdcardno)
	ON_EN_SETFOCUS(IDC_EDIT_IDCARDNO, &CInfoDlg::OnEnSetfocusEditIdcardno)
	ON_EN_KILLFOCUS(IDC_EDIT_IDCARDNO, &CInfoDlg::OnEnKillfocusEditIdcardno)
END_MESSAGE_MAP()


// CInfoDlg ��Ϣ�������


BOOL CInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	//*
	// ͸��������
	COLORREF maskColor = RGB(255, 255, 255);   //������ɫ
	SetWindowLong(this->GetSafeHwnd(),
		GWL_EXSTYLE,
		GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE) | 0x80000);  //�趨����ʹ����չģʽ 
	SetLayeredWindowAttributes(maskColor, 255, LWA_COLORKEY);

	/**/
	m_iplImgClearImg = cvCreateImage(cvSize(2, 2), IPL_DEPTH_8U, 3);
	for (int i = 0; i<m_iplImgClearImg->height; i++)
		for (int j = 0; j<m_iplImgClearImg->width; j++)
			cvSet2D(m_iplImgClearImg, i, j, cvScalar(255, 255, 255));


	MoveWindow(m_iScreenX, m_iScreenY,m_iWidth,m_iHeight, false);

	int offsetX = 0;
	int offsetY = 0;
	int rw = m_iWidth;
	int rh = (int)(m_iHeight * 0.2);
	GetDlgItem(IDC_CAMERA_IMG)->MoveWindow(offsetX, offsetY, rw, rh, false);
	

	offsetX = 0;
	offsetY += rh;
	rw = m_iWidth;
	rh = (int)(m_iHeight * 0.1);
	GetDlgItem(IDC_SIMILARITY)->MoveWindow(offsetX, offsetY, rw, rh, false);

	int tmpY = offsetY + rh;
	{
		// INFO_MODE_PHOTO
		offsetX = 0;
		offsetY = tmpY;
		rw = m_iWidth;
		rh = (int)(m_iHeight * 0.2);
		GetDlgItem(IDC_IDCARD_IMG)->MoveWindow(offsetX, offsetY, rw, rh, false);

		offsetX = 0;
		offsetY += rh;
		rw = m_iWidth;
		rh = (int)(m_iHeight * 0.25);
		GetDlgItem(IDC_RESULT_ICON)->MoveWindow(offsetX, offsetY, rw, rh, false);
		
		// 	INFO_MODE_IDCARDNO
		offsetX = 0;
		offsetY = tmpY;
		//CRect rect;
		//GetDlgItem(IDC_EDIT_IDCARDNO)->GetClientRect(&rect);
		rw = m_iWidth;
		rh = (int)(m_iHeight * 0.04 * m_fFontRate);
		GetDlgItem(IDC_EDIT_IDCARDNO)->MoveWindow(offsetX, offsetY, rw, rh, false);
		m_IDCardNoFont.CreatePointFont(_FSIZE(130), "Microsoft Yahei UI");
		GetDlgItem(IDC_EDIT_IDCARDNO)->SetFont(&m_IDCardNoFont);

		offsetY += rh;
		CRect rect;
		GetDlgItem(IDC_BTN_IDCARDNO)->GetClientRect(&rect);
		rw = (int)((rect.right - rect.left) * 1.4f * m_fFontRate);
		rh = (int)((rect.bottom - rect.top) * 1.4f * m_fFontRate);
		offsetX = (m_iWidth - rw) / 2;
		offsetY += ((int)(m_iHeight * 0.1) - rh) / 2;
		GetDlgItem(IDC_BTN_IDCARDNO)->MoveWindow(offsetX, offsetY, rw, rh, false);
		GetDlgItem(IDC_BTN_IDCARDNO)->SetFont(&m_IDCardNoFont);
	}

	offsetX = 0;
	offsetY = tmpY + (int)(m_iHeight * 0.45);
	rw = m_iWidth;
	rh = (int)(m_iHeight * 0.05);
	GetDlgItem(IDC_THRESHOLD_TEXT1)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetX = 0;
	offsetY += rh;
	rw = m_iWidth;
	rh = (int)(m_iHeight * 0.1);
	GetDlgItem(IDC_THRESHOLD)->MoveWindow(offsetX, offsetY, rw, rh, false);

	offsetY += rh;
	CRect rect;
	GetDlgItem(IDC_BTN_THRESHOLD_SET)->GetClientRect(&rect);
	rw = rect.right - rect.left;
	rh = rect.bottom - rect.top;
	offsetX = (m_iWidth - rw) / 2;
	offsetY += ((int)(m_iHeight * 0.1) - rh) / 2;
	GetDlgItem(IDC_BTN_THRESHOLD_SET)->MoveWindow(offsetX, offsetY, rw, rh, false);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


HBRUSH CInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (nCtlColor == CTLCOLOR_DLG)  //�Ի�����ɫ
	{
		return (HBRUSH)::GetStockObject(WHITE_BRUSH);//͸��������ɫ
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_SIMILARITY)
	{
		CFont font;
		//font.CreatePointFont(400, "����");
		font.CreateFont(m_iResultTextSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
			"΢���ź�");
		pDC->SelectObject(&font);
		//pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 255));
		//return (HBRUSH)GetStockObject(HOLLOW_BRUSH);

		CBrush *brush;
		brush = new CBrush(RGB(255, 255, 255));
		return (HBRUSH)(brush->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_EDIT&&pWnd->GetDlgCtrlID() == IDC_EDIT_IDCARDNO)
	{		
		pDC->SetBkColor(RGB(255, 255, 254));//�������屳����ɫ

		CBrush *brush;
		brush = new CBrush(RGB(255, 255, 254));
		return (HBRUSH)(brush->m_hObject);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_THRESHOLD_TEXT1)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}

	if (nCtlColor == CTLCOLOR_STATIC&&pWnd->GetDlgCtrlID() == IDC_THRESHOLD)
	{
		CFont font;
		//font.CreatePointFont(200, "����");
		font.CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
			"΢���ź�");
		pDC->SelectObject(&font);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 255));
		//return (HBRUSH)GetStockObject(HOLLOW_BRUSH);

		CBrush *brush;
		brush = new CBrush(RGB(255, 255, 255));
		return (HBRUSH)(brush->m_hObject);
	}


	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void CInfoDlg::setShowMode(int mode)
{
	if (INFO_MODE_PHOTO == mode) {
		GetDlgItem(IDC_IDCARD_IMG)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_RESULT_ICON)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_IDCARDNO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_IDCARDNO)->ShowWindow(SW_HIDE);
	}
	else {
		// INFO_MODE_IDCARDNO
		GetDlgItem(IDC_IDCARD_IMG)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RESULT_ICON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_IDCARDNO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_IDCARDNO)->ShowWindow(SW_SHOW);
	}
}

void CInfoDlg::setMode(int mode) 
{
	m_iMode = mode;
	setShowMode(mode);
}

void CInfoDlg::drawImage(IplImage* img, UINT ID, int type, bool clearArea)
{
	CWnd* pWnd = GetDlgItem(ID);
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

	if (DRAW_TYPE_CENTER == type) {
			int tx = (int)(rw - iw)/2;					// ʹͼƬ����ʾλ�������ڿؼ�������
			int ty = (int)(rh - ih)/2;
			SetRect(rect, tx, ty, tx+iw, ty+ih);
	}
	else if(DRAW_TYPE_FULL == type)
		SetRect(rect, 0, 0, rw, rh);	// �����ؼ�
	else if (DRAW_TYPE_RATIO == type) {
		int tx, ty, tw, th;
		if (iw *0.1 / ih > rw * 0.1 / rh) {
			tw = rw;
			th = ih * tw / iw;
			tx = 0;
			ty = (rect.bottom - rect.top - th)/2;
		}
		else {
			th = rh;
			tw = iw * th / ih;
			tx = (rect.right - rect.left - tw)/2;
			ty = 0;
		}
		SetRect(rect, tx, ty, tx + tw, ty + th);	// ���ֿ�߱�
	}

	if (clearArea) {
		CRect clearrect;
		SetRect(clearrect, 0, 0, rw, rh);
		//CvvImage clearimg;
		//clearimg.CopyOf(m_iplImgClearImg, -1);
		//clearimg.DrawToHDC(hDC, &clearrect);
		pDC->FillSolidRect(clearrect, RGB(255, 255, 255));
	}

	CvvImage cimg;
	cimg.CopyOf(img, -1);							// ����ͼƬ
	cimg.DrawToHDC(hDC, &rect);				// ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������

	ReleaseDC(pDC);
}

void CInfoDlg::drawIdcardImage(IplImage* img)
{
	if (!img)
		return;

//	clock_t time1 = clock();
	CvSize ImgOrgSize;
	ImgOrgSize.width = img->width;
	ImgOrgSize.height = img->height;
	IplImage *iplImgTemp = cvCreateImage(ImgOrgSize, IPL_DEPTH_8U, 3);
	cvCopy(img, iplImgTemp);
	iplImgTemp->origin = img->origin;
	cv::Mat imgMat = cvarrToMat(iplImgTemp);
	Mat_<Vec3b>::iterator it = imgMat.begin<Vec3b>();
	Mat_<Vec3b>::iterator itend = imgMat.end<Vec3b>();
	for (; it != itend; it++)
	{
		if ((*it)[0] == 255 && (*it)[1] == 255 && (*it)[2] == 255)
			(*it)[0] = 254;
	}
//	clock_t dt = clock() - time1;
	drawImage(iplImgTemp, IDC_IDCARD_IMG, DRAW_TYPE_CENTER);
	
	cvReleaseImage(&iplImgTemp);
}

void CInfoDlg::drawCameraImage(IplImage* img)
{
	if (!img)
		return;

//	clock_t time1 = clock();
	CvSize ImgOrgSize;
	ImgOrgSize.width = img->width;
	ImgOrgSize.height = img->height;
	IplImage *iplImgTemp = cvCreateImage(ImgOrgSize, IPL_DEPTH_8U, 3);
	cvCopy(img, iplImgTemp);
	cv::Mat imgMat = cvarrToMat(iplImgTemp);
	Mat_<Vec3b>::iterator it = imgMat.begin<Vec3b>();
	Mat_<Vec3b>::iterator itend = imgMat.end<Vec3b>();
	for (; it != itend; it++)
	{
		// ����ɫ���ӽ�ɫ�޸�
		if ((*it)[0] >= 250 && (*it)[1] >= 250 && (*it)[2] >= 250) {
			(*it)[0] = 250;
			(*it)[1] = 250;
			(*it)[2] = 250;
		}
	}
//	clock_t dt = clock() - time1;
	drawImage(iplImgTemp, IDC_CAMERA_IMG, DRAW_TYPE_RATIO, true);

	cvReleaseImage(&iplImgTemp);
}

void CInfoDlg::clearCameraImage() { drawImage(m_iplImgClearImg, IDC_CAMERA_IMG, DRAW_TYPE_FULL); }
void CInfoDlg::clearIdcardImage() { drawImage(m_iplImgClearImg, IDC_IDCARD_IMG, DRAW_TYPE_FULL); }
void CInfoDlg::clearResultIcon() { drawImage(m_iplImgClearImg, IDC_RESULT_ICON, DRAW_TYPE_FULL); }

void CInfoDlg::drawResultIcon(IplImage* img)
{
	if (!img)
		return;

	drawImage(img, IDC_RESULT_ICON, DRAW_TYPE_CENTER);
}

void CInfoDlg::setResultText(std::string result)
{
	CWnd* pWnd = GetDlgItem(IDC_SIMILARITY);
	if(pWnd)
		pWnd->SetWindowText(result.c_str());
}

void CInfoDlg::setThresholdText(std::string text)
{
	CWnd* pWnd = GetDlgItem(IDC_THRESHOLD);
	if (pWnd)
		pWnd->SetWindowText(text.c_str());
}

void CInfoDlg::getInfoDlgScreenRect(cv::Rect &rect)
{
	rect.x = m_iScreenX;
	rect.y = m_iScreenY;
	rect.width = m_iWidth;
	rect.height = m_iHeight;
}

void CInfoDlg::getItemRect(int nID, cv::Rect &rect)
{
	CWnd* pWnd = GetDlgItem(nID);
	if (pWnd) {
		CRect r;
		pWnd->GetWindowRect(&r);
		ScreenToClient(&r);
		rect.x = r.left;
		rect.y = r.top;
		rect.width = r.right - r.left;
		rect.height = r.bottom - r.top;
	}
}

void CInfoDlg::getCameraImageRect(cv::Rect &rect)
{
	getItemRect(IDC_CAMERA_IMG, rect);
}

void CInfoDlg::getResultIconRect(cv::Rect &rect)
{
	getItemRect(IDC_RESULT_ICON, rect);
}

void CInfoDlg::resetIDCardNoInput()
{
	GetDlgItem(IDC_EDIT_IDCARDNO)->SetWindowText(_T("�������������֤����"));
	GetDlgItem(IDC_EDIT_IDCARDNO)->EnableWindow(true);
	GetDlgItem(IDC_BTN_IDCARDNO)->SetWindowText(_T("ȷ��"));
	m_bIDCardNoReady = false;
	m_bIDCardNoNoneText = true;
}

std::string CInfoDlg::getIDCardNo()
{
	CString idcardno;
	GetDlgItem(IDC_EDIT_IDCARDNO)->GetWindowText(idcardno);
	return idcardno.GetString();
}

void CInfoDlg::enableIDCardNoBtn(bool enable)
{
	GetDlgItem(IDC_BTN_IDCARDNO)->EnableWindow(enable);
}

void CInfoDlg::OnBnClickedBtnThresholdSet()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)m_pFdvDlg;
	if (pDlg)
		pDlg->stopClearTimer();

	CThresholdSettingDlg dlg(this);
	dlg.DoModal();

	if (pDlg)
		pDlg->setClearTimer(FDVDLG_DEFAULT_CLEAR_TIME);
}


void CInfoDlg::OnBnClickedBtnIdcardno()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)m_pFdvDlg;

	if (m_bIDCardNoReady) {
		GetDlgItem(IDC_EDIT_IDCARDNO)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_IDCARDNO)->SetFocus();
		GetDlgItem(IDC_BTN_IDCARDNO)->SetWindowText(_T("ȷ��"));
		m_bIDCardNoReady = false;
	}
	else {
		string idcardno = getIDCardNo();
		const regex rx("^[1-9]\\d{7}((0\\d)|(1[0-2]))(([0|1|2]\\d)|3[0-1])\\d{3}$|^[1-9]\\d{5}[1-9]\\d{3}((0\\d)|(1[0-2]))(([0|1|2]\\d)|3[0-1])\\d{3}([0-9]|X)$");
		smatch results;

		if (regex_match(idcardno, results, rx)) {
			GetDlgItem(IDC_EDIT_IDCARDNO)->EnableWindow(false);
			GetDlgItem(IDC_BTN_IDCARDNO)->SetWindowText(_T("�޸�"));
			m_bIDCardNoReady = true;
		}
		else {
			AfxMessageBox("��Ч�����֤���룡");
			m_bIDCardNoReady = false;
		}
	}

	if (pDlg)
		pDlg->setClearTimer(FDVDLG_DEFAULT_CLEAR_TIME);
}


void CInfoDlg::OnEnChangeEditIdcardno()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CIDCardFdvDlg* pDlg = (CIDCardFdvDlg*)m_pFdvDlg;
	if (pDlg)
		pDlg->setClearTimer(FDVDLG_DEFAULT_CLEAR_TIME);
}


void CInfoDlg::OnEnSetfocusEditIdcardno()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_bIDCardNoNoneText) {
		GetDlgItem(IDC_EDIT_IDCARDNO)->SetWindowText("");
		m_bIDCardNoNoneText = false;
	}
}

void CInfoDlg::OnEnKillfocusEditIdcardno()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	string no = getIDCardNo();
	if ("" == no) {
		resetIDCardNoInput();
	}
}


void CInfoDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���

	//CDialogEx::OnOK();
}


BOOL CInfoDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE: //Esc�����¼�
			if (GetFocus() == GetDlgItem(IDC_EDIT_IDCARDNO)) {				
				GetDlgItem(IDC_EDIT_IDCARDNO)->SetWindowText("");
				UpdateData(TRUE);
			}
			return true;
		case VK_RETURN: //Enter�����¼�
			if (GetFocus() == GetDlgItem(IDC_EDIT_IDCARDNO)) {
				OnBnClickedBtnIdcardno();
			}
			return true;
		default:
			;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}



