#pragma once


// CLockDialog 对话框

class CLockDialog : public CDialog
{
	DECLARE_DYNAMIC(CLockDialog)

public:
	CLockDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLockDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};
