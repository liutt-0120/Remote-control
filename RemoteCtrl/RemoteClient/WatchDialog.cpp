// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "WatchDialog.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"
#include "ClientController.h"
// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_nObjWdith = -1;
	m_nObjHeight = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_isFull = false;
	// TODO:  在此添加额外的初始化
	SetTimer(0, 50, NULL);



	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		if (IsFull()) {
			CRect rect;
			m_picture.GetWindowRect(rect);
			if (m_nObjWdith == -1) {
				m_nObjWdith = GetImage().GetWidth();
			}
			if (m_nObjHeight == -1) {
				m_nObjHeight =GetImage().GetHeight();

			}
			GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);	//缩放显示
			//m_picture.InvalidateRect(NULL);			//重绘，这玩意儿有没有好像无所谓
			GetImage().Destroy();
			SetImageStatus();
			TRACE("更新图片完成 %d %d\r\n", m_nObjWdith, m_nObjHeight);
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWdith != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MouseEv event;
		event.ptXY = remote;
		event.nButton = 0;	//左键
		event.nAction = 1;	//双击
		CClientController::GetInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWdith != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MouseEv event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 2;	//按下
		CClientController::GetInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWdith != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MouseEv event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 3;	//放开
		CClientController::GetInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWdith != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MouseEv event;
		event.ptXY = remote;
		event.nButton = 1;	//右键
		event.nAction = 1;	//双击
		CClientController::GetInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWdith != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MouseEv event;
		event.ptXY = remote;
		event.nButton = 1;	//右键
		event.nAction = 2;	//按下
		CClientController::GetInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWdith != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MouseEv event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 3;	//放开
		CClientController::GetInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWdith != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MouseEv event;
		event.ptXY = remote;
		event.nButton = 3;	//没有按键
		event.nAction = 0;	//不相关
		CClientController::GetInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnMouseMove(nFlags, point);
}

CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	if (isScreen == true) {
		ScreenToClient(&point);
	}
	CRect clientRect;
	m_picture.GetWindowRect(clientRect);
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	//int width = 1920, height = 1080;
	return CPoint(point.x * m_nObjWdith / width0, point.y * m_nObjHeight / height0);
}

/// <summary>
/// 禁掉回车关闭窗口
/// </summary>
void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	CClientController::GetInstance()->SendCommandPacket(7);

}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	CClientController::GetInstance()->SendCommandPacket(8);

}
