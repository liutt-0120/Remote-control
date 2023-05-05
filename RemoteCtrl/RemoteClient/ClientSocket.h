#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include "CPacket.h"
#include "MyTool.h"
#define MAX_SIZE 409600

/// <summary>
/// 客户端socket网络类
/// </summary>
class CClientSocket {
public:
	static CClientSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
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

	bool Send(const char* pData, int nSize) {
		if (m_sockSrv == -1)return false;
		return send(m_sockSrv, pData, nSize, 0) > 0;
	}

	bool Send(const CPacket& pack) {		
		if (m_sockSrv == -1)return false;
		std::string strOut;
		pack.Data(strOut);
		return send(m_sockSrv, strOut.c_str(), strOut.size(), 0) > 0;
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
private:
	int m_nIP;	//地址
	int m_nPort;	//端口
	SOCKET m_sockSrv;
	CPacket m_packet;
	std::vector<char>m_buffer;	//使用容器，不用担心内存泄漏
	CClientSocket() :m_nIP(INADDR_ANY), m_nPort(0) {
		m_sockSrv = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("初始化套接字环境失败，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(MAX_SIZE);	//初始化size为4096
	}
	CClientSocket(const CClientSocket& cs) {
		m_sockSrv = cs.m_sockSrv;
		m_nIP = cs.m_nIP;
		m_nPort = cs.m_nPort;
	}
	CClientSocket& operator=(const CClientSocket&) {}
	~CClientSocket() {
		closesocket(m_sockSrv);
		WSACleanup();
	}
	static CClientSocket* m_instance;

	bool InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

private:
	class CGarbo {
	public:

		~CGarbo() {
			if (m_instance != NULL)
				delete m_instance;
		}
	};
	static CGarbo garbo;
};

