// AdvHtmlView.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IDCardFdv.h"
#include "AdvHtmlView.h"


// CAdvHtmlView

IMPLEMENT_DYNCREATE(CAdvHtmlView, CHtmlView)

CAdvHtmlView::CAdvHtmlView()
{

}

CAdvHtmlView::~CAdvHtmlView()
{
}

void CAdvHtmlView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAdvHtmlView, CHtmlView)
	ON_WM_DESTROY()
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()


// CAdvHtmlView ���

#ifdef _DEBUG
void CAdvHtmlView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CAdvHtmlView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CAdvHtmlView ��Ϣ�������


void CAdvHtmlView::OnDestroy()
{
	// TODO: �ڴ˴������Ϣ����������
	if (m_pBrowserApp)
	{
		m_pBrowserApp.Release();
		m_pBrowserApp = NULL;
	}

	CWnd::OnDestroy(); //Ϊ������CView(doc/frame)��OnDestroy
}


void CAdvHtmlView::PostNcDestroy()
{
	// TODO: �ڴ����ר�ô����/����û���

	//CHtmlView::PostNcDestroy(); ע�͵�,��ֹCView �����Լ�}OnInitialUpdate����:
}


void CAdvHtmlView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();

	// TODO: �ڴ����ר�ô����/����û���
}


int CAdvHtmlView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message); //Ϊ������CView(doc/frame)��OnMouseActivate
}

BOOL CAdvHtmlView::CreateFromStatic(UINT nID, CWnd* pParent)
{
	CWnd* pStatic = pParent->GetDlgItem(nID);
	if (pStatic == NULL) return FALSE;
	CRect rc;
	pStatic->GetWindowRect(&rc);
	pParent->ScreenToClient(&rc);
	pStatic->DestroyWindow();
	if (!CHtmlView::Create(NULL, NULL, (WS_CHILD), rc, pParent, nID, NULL))
		return FALSE;

	SetSilent(TRUE);	//add by wh ,bid pop script dlg false��ʾ������浯���Ի���

	return TRUE;
}

void CAdvHtmlView::SetUrl(LPCTSTR url)
{
	Navigate2(url, NULL, NULL);
}
