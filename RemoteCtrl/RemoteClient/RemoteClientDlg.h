
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"

#define WM_SEND_PACKET (WM_USER+1)
#define WM_SEND_PROGRESS (WM_USER+2)

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
	int SendCommandPacket(int nCmd,bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0);
	CString CRemoteClientDlg::GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTreeSelected);
	void LoadFileInfo();
	void LoadFileCurrent();
	static void ThreadForDownloadFile(void*);
	void DownloadFile();
	static void ThreadForWatchData(void*);
	void WatchData();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_statusDlg;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	bool IsFull() const {
		return m_isFull;
	}
	void SetImageStatus(bool bStatus = false) {
		m_isFull = bStatus;
	}
	CImage& GetImage() {		//⭐艹，死点，切记拿全局对象实例时要返回引用
		return m_image;
	}
private:
	CImage m_image;	//获取屏幕图像的缓存
	bool m_isFull;	//缓存是否有数据，true为有，false为无
	bool m_isClosed;//监视是否关闭
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
	afx_msg LRESULT OnSendPacket(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnSendProgress(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedBtnStartwatch();
};
