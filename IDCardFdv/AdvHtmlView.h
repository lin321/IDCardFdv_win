#pragma once

#ifdef _WIN32_WCE
#error "Windows CE 不支持 CHtmlView。"
#endif 

// CAdvHtmlView Html 视图

class CAdvHtmlView : public CHtmlView
{
	DECLARE_DYNCREATE(CAdvHtmlView)

public:
	CAdvHtmlView();           // 动态创建所使用的受保护的构造函数
	virtual ~CAdvHtmlView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	virtual void PostNcDestroy();
	virtual void OnInitialUpdate();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);

	BOOL CreateFromStatic(UINT nID, CWnd* pParent);
	void SetUrl(LPCTSTR url);
};


