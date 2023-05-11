#pragma once

#ifndef  WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER)+2	//发送包数据应答
#endif

// CWatchDialog 对话框

class CWatchDialog : public CDialog
{
	DECLARE_DYNAMIC(CWatchDialog)

public:
	CWatchDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWatchDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_WATCH };
#endif
public:
	int m_nObjWdith;
	int m_nObjHeight;
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
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CPoint UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen = false);
	virtual void OnOK();
	afx_msg void OnBnClickedBtnLock();
	afx_msg void OnBnClickedBtnUnlock();
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);

};
