
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"

#ifndef  WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER)+2	//发送包数据应答
#endif

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
	CString CRemoteClientDlg::GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTreeSelected);
	void LoadFileInfo();
	void LoadFileCurrent();
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
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_list;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenfile();
	afx_msg void OnDownloadfile();
	afx_msg void OnDeletefile();
	afx_msg void OnBnClickedBtnStartwatch();
	afx_msg void OnIpnFieldchangedIpaddressServer(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeServerPort();
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);

};
