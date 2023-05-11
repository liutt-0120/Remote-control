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
/// MVC�е�C����ͼ�����ݵ��м䴫���ߣ�����
/// </summary>
class CClientController
{
public:
	// ���� ��ȡȫ��Ψһ����
	static CClientController* GetInstance();
	// ��ʼ������
	int InitController();
	// ����
	int Invoke(CWnd*& pMainWnd);
	////������Ϣ
	//LRESULT SendMessage2Func(MSG msg);
	//���·�������ַ
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

	/// <summary>
	/// 1 �鿴���̷���
	/// 2 �鿴ָ��Ŀ¼�µ��ļ�
	/// 3 ���ļ�
	/// 4�����ļ�
	/// 9 ɾ���ļ�
	/// 5 ������
	/// 6 ������Ļ����
	/// 7 ����
	/// 8 ����
	/// 95 ��������
	/// </summary>
	/// <param name="hWnd">���ݰ��յ�����ҪӦ��Ĵ���</param>
	/// <param name="nCmd"></param>
	/// <param name="bAutoClose">�Ƿ��Զ��ر�</param>
	/// <param name="pData"></param>
	/// <param name="nLength"></param>
	/// <returns>����״̬��true�ɹ���falseʧ��</returns>
	bool SendCommandPacket(
		HWND hWnd,
		int nCmd, 
		bool bAutoClose = true, 
		BYTE* pData = NULL, 
		size_t nLength = 0,
		WPARAM wParam = 0
	);
	int GetImage(CImage& image);

	int DownloadFile(CString strPath);

	void StartWatchScreen();
	CPacket& GetPacket();

	void DownloadEnd();

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

	//�߳̿��� �������
	static unsigned WINAPI ThreadEntry(void* arg);
	//ʵ�ʱ��̴߳����ķ���
	void ThreadFunc();

	//LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);

	//���ص��߳����
	static void ThreadEntryForDownloadFile(void* args);
	void ThreadFuncForDownloadFile();
	//��Ļ�����߳����
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
	//��Ա����ָ�룬����ָ��ͬ����Ϣ����
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC>m_mapFunc;
	//

	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	CWatchDialog m_watchDlg;

	HANDLE m_hThread;
	unsigned m_wThreadId;

	HANDLE m_hThreadDownload;
	// �߳�ֱ���ó�Ա����
	CString m_strRemotePath;
	CString m_strLocalPath;

	HANDLE m_hThreadWatch;
	bool m_isClosed;//�����Ƿ�ر�

	class CGarbo {
	public:

		~CGarbo() {
			if (m_ctrlInstance != NULL)
				delete m_ctrlInstance;
		}
	};
	static CGarbo garbo;
};

