
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif
private:
	int SendCommandPacket(int nCmd, BYTE* pData = NULL, size_t nLength = 0);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	// 服务器IP
	DWORD m_server_address;
	// 服务器端口
	CString m_port;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_tree;
};
