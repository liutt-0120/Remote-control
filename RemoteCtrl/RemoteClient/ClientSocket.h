#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include "CPacket.h"
#include "MyTool.h"
#include <map>
#include <list>
#include <mutex>
#define MAX_SIZE 409600
#define WM_SEND_PACK (WM_USER)+1
#define WM_SEND_PACK_ACK (WM_USER)+2

enum {
	CSM_AUTOCLOSE = 1,	//CSM = Client Socket Mode自动关闭模式
};

/// <summary>
/// 发送的数据结构
/// </summary>
typedef struct PacketData{
	std::string strData;
	UINT nMode;
	PacketData(const char* pData, size_t nLen, UINT mode) {
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
	}
	PacketData(const PacketData& packData) {
		strData = packData.strData;
		nMode = packData.nMode;
	}
	PacketData& operator=(const PacketData& packData) {
		if (this != &packData) {
			strData = packData.strData;
			nMode = packData.nMode;
		}
		return *this;
	}
}PACKET_DATA;
/// <summary>
/// 客户端socket网络类
/// </summary>
class CClientSocket {
public:
	static CClientSocket* getInstance() {
		if (m_clientInstance == NULL) {
			m_clientInstance = new CClientSocket();
		}
		return m_clientInstance;
	}

	//socket相关函数
	bool InitSocket() {
		if (m_sockSrv != INVALID_SOCKET) CloseSocket();
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sockSrv == -1)return false;

		SOCKADDR_IN addrSrv;
		//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		TRACE("addr:%08x , ip:%08x\r\n", inet_addr("127.0.0.1"), m_nIP);	//addr:0100007f , ip:7f000001
		addrSrv.sin_addr.S_un.S_addr = htonl(m_nIP);	//The htonl function converts a u_long from host to TCP/IP network byte order (which is big-endian).
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(m_nPort);
		if (addrSrv.sin_addr.S_un.S_addr == INADDR_NONE) {
			AfxMessageBox("指定的IP地址不存在");	//mfc
			return false;
		}
		int ret = connect(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		if (ret == SOCKET_ERROR) {
			AfxMessageBox("连接失败");	//mfc
			TRACE("连接失败：%d\r\n", WSAGetLastError(), CMyTool::GetErrorInfo(WSAGetLastError()).c_str());
		}
		return true;
	}

	int DealCommand() {
		if (m_sockSrv == -1)return -1;
		//char recvbuf[1024] = "";
		//char* recvBuf = new char[MAX_SIZE];	//改用成员变量
		char* recvBuf = m_buffer.data();	//若buffer里有多个包可以一一处理
		static size_t index = 0;
		while (true) {
			size_t len = recv(m_sockSrv, recvBuf + index, MAX_SIZE - index, 0);	//len --- 本次接收到数据的长度
			if (((int)len <= 0) && ((int)index <= 0)) {
				return -1;
			}
			TRACE("recv len = %d(0x%08X) index = %d(0x%08X)\r\n", len, len, index, index);
			index += len;	//修正起始位置
			len = index;	//实际接收到数据的长度
			TRACE("recv len = %d(0x%08X) index = %d(0x%08X)\r\n", len, len, index, index);
			m_packet = CPacket((BYTE*)recvBuf, len);	//len接收实际处理的包的长度
			if (len > 0) {
				memmove(recvBuf, recvBuf + len, MAX_SIZE - len);	//将已被处理的包所占的内存位置释放：后面的数据挪前面来
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MouseEv& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MouseEv));
			return true;
		}
		return false;
	}

	CPacket& GetPacket() {
		return m_packet;
	}

	/// <summary>
	/// 
	/// </summary>
	void CloseSocket() {
		closesocket(m_sockSrv);
		m_sockSrv = INVALID_SOCKET;
	}

	void UpdateAddress(int ip,int port) {
		m_nIP = ip;
		m_nPort = port;
	}
	/*
	bool SendInPacketList(CPacket& pack, bool bAutoClose);

	bool GetRecvPacket(std::list<CPacket>& packlst,HANDLE& hEvent);
	*/
	SOCKET& GetSockSrv() {
		return m_sockSrv;
	}

	bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool bAutoClose);

protected:
	static void ThreadEntry(void* arg);
	void ThreadFunc();
	//reconfiguration
protected:
	static unsigned WINAPI ThreadEntry_Remake(void* arg);
	void ThreadFunc_Remake();

	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC> m_mapFunc;
private:
	std::list<CPacket> m_lstSend;	//存发出去的pack
	std::map<HANDLE, std::list<CPacket>> m_mapPack;	//存recv的事件对应的回复packList
	std::map<HANDLE, bool> m_mapAutoClosed;
	std::mutex m_lock;
	int m_nIP;	//地址
	int m_nPort;	//端口
	SOCKET m_sockSrv;
	CPacket m_packet;
	std::vector<char>m_buffer;	//使用容器，不用担心内存泄漏
	HANDLE m_hThread;
	UINT m_wThreadId;
	CClientSocket();
	CClientSocket(const CClientSocket& cs);
	CClientSocket& operator=(const CClientSocket& cs);
	~CClientSocket() {
		closesocket(m_sockSrv);
		WSACleanup();
	}
	static CClientSocket* m_clientInstance;

	bool InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	bool Send(const char* pData, int nSize);

	bool Send(const CPacket& pack);


private:
	class CGarbo {
	public:

		~CGarbo() {
			if (m_clientInstance != NULL)
				delete m_clientInstance;
		}
	};
	static CGarbo garbo;
	

};

