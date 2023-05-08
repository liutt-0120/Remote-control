#pragma once
#include "ClientSocket.h" 
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "WatchDialog.h"
#include <map>
#include "resource.h"
#include "ClientSocket.h"
#include "MyTool.h"

//#define WM_SEND_PACK (WM_USER)+1
//#define WM_SEND_DATA (WM_USER)+2
//#define WM_SHOW_STATUS (WM_USER)+3
//#define	WM_SHOW_WATCH (WM_USER)+4
#define WM_SEND_MESSAGE (WM_USER)+0x1000
/// <summary>
/// MVC中的C，视图和数据的中间传递者，解耦
/// </summary>
class CClientController
{
public:
	// 单例 获取全局唯一对象
	static CClientController* GetInstance();
	// 初始化操作
	int InitController();
	// 启动
	int Invoke(CWnd*& pMainWnd);
	////发送消息
	//LRESULT SendMessage2Func(MSG msg);
	//更新服务器地址
	void UpdateAddress(int ip, int port) {
		CClientSocket::getInstance()->UpdateAddress(ip, port);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}
	//bool SendPacket(const CPacket& pack) {
	//	CClientSocket* pClient = CClientSocket::getInstance();
	//	if (pClient->InitSocket() == false)return false;
	//	pClient->Send(pack);
	//}

	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0, std::list<CPacket>* pPackLst = NULL);
	int GetImage(CImage& image);

	int DownloadFile(CString strPath);

	void StartWatchScreen();
	CPacket& GetPacket();

protected:
	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg),
		m_hThread(INVALID_HANDLE_VALUE),
		m_wThreadId(-1),
		m_hThreadDownload(INVALID_HANDLE_VALUE),
		m_hThreadWatch(INVALID_HANDLE_VALUE),
		m_isClosed(true){}

	~CClientController(){
		WaitForSingleObject(m_hThread, 100);
	}

	//线程开启 函数入口
	static unsigned WINAPI ThreadEntry(void* arg);
	//实际被线程触发的方法
	void ThreadFunc();

	//LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);

	//下载的线程入口
	static void ThreadEntryForDownloadFile(void* args);
	void ThreadFuncForDownloadFile();
	//屏幕监视线程入口
	static void ThreadEntryForWatchScreen(void* args);
	void ThreadFuncForWatchScreen();

private:
	static CClientController* m_ctrlInstance;

	typedef struct MsgInfo {
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& mInfo) {
			result = mInfo.result;
			memcpy(&msg, &mInfo.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& mInfo) {
			if (this != &mInfo) {
				result = mInfo.result;
				memcpy(&msg, &mInfo.msg, sizeof(MSG));
			}
			return *this;
		}
	} MSGINFO;
	//成员函数指针，按需指向不同的消息函数
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC>m_mapFunc;
	//

	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	CWatchDialog m_watchDlg;

	HANDLE m_hThread;
	unsigned m_wThreadId;

	HANDLE m_hThreadDownload;
	// 线程直接拿成员变量
	CString m_strRemotePath;
	CString m_strLocalPath;

	HANDLE m_hThreadWatch;
	bool m_isClosed;//监视是否关闭

	class CGarbo {
	public:

		~CGarbo() {
			if (m_ctrlInstance != NULL)
				delete m_ctrlInstance;
		}
	};
	static CGarbo garbo;
};

