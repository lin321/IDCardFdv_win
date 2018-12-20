#pragma once

#ifdef _WIN32_WCE
#error "Windows CE ��֧�� CHtmlView��"
#endif 

// CAdvHtmlView Html ��ͼ

class CAdvHtmlView : public CHtmlView
{
	DECLARE_DYNCREATE(CAdvHtmlView)

public:
	CAdvHtmlView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CAdvHtmlView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	virtual void PostNcDestroy();
	virtual void OnInitialUpdate();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);

	BOOL CreateFromStatic(UINT nID, CWnd* pParent);
	void SetUrl(LPCTSTR url);
};


