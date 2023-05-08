#pragma once


// CStatusDlg 对话框

class CStatusDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatusDlg)

public:
	CStatusDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CStatusDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DOWNLOAD_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 下载信息
	CString m_info;
	// 下载进度条
	CProgressCtrl m_progressBar;
	CStatic m_info2;
};
