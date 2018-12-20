// AdvHtmlView.cpp : 实现文件
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


// CAdvHtmlView 诊断

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


// CAdvHtmlView 消息处理程序


void CAdvHtmlView::OnDestroy()
{
	// TODO: 在此处添加消息处理程序代码
	if (m_pBrowserApp)
	{
		m_pBrowserApp.Release();
		m_pBrowserApp = NULL;
	}

	CWnd::OnDestroy(); //为了跳过CView(doc/frame)的OnDestroy
}


void CAdvHtmlView::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CHtmlView::PostNcDestroy(); 注释掉,防止CView 销毁自己}OnInitialUpdate函数:
}


void CAdvHtmlView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
}


int CAdvHtmlView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message); //为了跳过CView(doc/frame)的OnMouseActivate
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

	SetSilent(TRUE);	//add by wh ,bid pop script dlg false表示允许界面弹出对话框

	return TRUE;
}

void CAdvHtmlView::SetUrl(LPCTSTR url)
{
	Navigate2(url, NULL, NULL);
}
