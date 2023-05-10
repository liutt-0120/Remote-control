#pragma once

#include "pch.h"
#include "framework.h"
#include <list>
#include "CPacket.h"
#define MAX_SIZE 409600


///// <summary>
///// 方式一：这个类的目的只是项目最初Startup和项目结尾Cleanup套接字环境，而以这种开放的形式是难以保证这个类在项目中不被实例化。
///// </summary>
//class CServerSocket
//{
//public:
//	CServerSocket() {
//		if (InitSockEnv() == FALSE) {
//			MessageBox(NULL, _T("初始化套接字环境失败，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
//			exit(0);
//		}
//	}
//
//	~CServerSocket() {
//		WSACleanup();
//	}
//
//
//	bool InitSockEnv() {
//		WSADATA data;
//		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
//			return FALSE;
//		}
//		return TRUE;
//	}
//};
//extern CServerSocket server;


//-----------------------------------------------------------------------------------------
//确保唯一性就要使用单例模式：使类的一个对象成为系统中的唯一实例
//-----------------------------------------------------------------------------------------
///// <summary>
///// 方式二：
///// 构造函数私有化，避免被创建实例
///// 一个私有的指向唯一实例的静态指针；
///// 开放一个公有的函数，可以获取这个唯一的实例，并且在需要的时候创建该实例；
///// </summary>
//class CServerSocket
//{
//public:
//	//暴露这两个方法已经满足单例模式
//	static CServerSocket* getInstance() {
//		if (m_instance == NULL) {
//			m_instance = new CServerSocket();
//		}
//		return m_instance;
//	}
//
//	~CServerSocket() {
//		WSACleanup();
//	}
//private:
//	CServerSocket& operator=(const CServerSocket&) {}
//	CServerSocket(const CServerSocket&){}
//	CServerSocket() {
//		if (InitSockEnv() == FALSE) {
//			MessageBox(NULL, _T("初始化套接字环境失败，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
//			exit(0);
//		}
//	}
//
//	static CServerSocket* m_instance;
//
//	bool InitSockEnv() {
//		WSADATA data;
//		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
//			return FALSE;
//		}
//		return TRUE;
//	}
//};

/*
* 方式二已经能满足单例模式。CServerSocket* server = CServerSocket::getInstance();确保唯一一个实例，不需要时delete server;即可
* 但存在两个问题：
*	1.线程不安全的（存疑，有人说new是线程安全的有人说不是）
*	2.delete很容易被忘记，而且也很难保证在delete之后，没有代码再调用GetInstance函数。
* 一个妥善的方法是让这个类自己知道在合适的时候把自己删除，或者说把删除自己的操作挂在操作系统中的某个合适的点上，使其在恰当的时候被自动执行。
* 我们知道，程序在结束的时候，系统会自动析构所有的全局变量。
* 事实上，系统也会析构所有的类的静态成员变量，就像这些静态成员也是全局变量一样。
* 利用这个特征，可以在单例类中定义一个这样的静态成员，它的唯一工作就是在析构函数中删除单例类的实例。
* //https://blog.csdn.net/realxie/article/details/7090493
*/



typedef void(*SOCKET_CALLBACK)(void* arg, int status, std::list<CPacket>& pack_list,CPacket& inPacket);
class CServerSocket {
public:
	int RunServer(SOCKET_CALLBACK callback,void* arg,int port = 9527) {

		//socketl连接初始化
		bool bRet = InitSocket(port);
		if (!bRet) {
			return -1;
		}
		//服务端接收客户端连接
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				++count;
			}
			//接收数据
			m_callback = callback;
			m_arg = arg;
			int nRet = DealCommand();
			if (nRet > 0) {
				m_callback(m_arg, nRet,pack_list,m_packet);
				while (pack_list.size() != 0) {
					Send(pack_list.front());
					pack_list.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
	}

	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			TRACE("CServerSocket* getInstance初始化m_instance\r\n");
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
protected:
	//socket相关函数
	bool InitSocket(int port) {
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sockSrv == -1)return false;

		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(9527);

		//绑定
		if (-1 == bind(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) return false;
		//
		if (-1 == listen(m_sockSrv, 1)) return false;
		return true;
	}

	bool AcceptClient() {
		SOCKADDR_IN addrCli;
		int iLen = sizeof(SOCKADDR);
		m_sockCli = accept(m_sockSrv, (SOCKADDR*)&addrCli, &iLen);
		TRACE("client = %d\r\n", m_sockCli);
		if (m_sockCli == -1)return false;
		return true;
	}

	int DealCommand() {
		if (m_sockCli == -1)return -1;
		//char recvbuf[1024] = "";
		char* recvBuf = new char[MAX_SIZE];	//缓冲区较大，存储到堆上
		if (recvBuf == NULL) {
			TRACE("内存不足！\r\n");
			return -2;
		}
		memset(recvBuf, 0, MAX_SIZE);
		size_t index = 0;
		while (true)  {
			size_t len = recv(m_sockCli, recvBuf + index, MAX_SIZE - index, 0);	//len --- 本次接收到数据的长度
			if (len <= 0) {
				delete[] recvBuf;
				return -1;
			}
			index += len;	//修正起始位置
			len = index;	//实际接收到数据的长度
			m_packet = CPacket((BYTE*)recvBuf, len);	//len接收实际处理的包的长度
			if (len > 0) {
				memmove(recvBuf, recvBuf + len, MAX_SIZE - len);	//将已被处理的包所占的内存位置释放：后面的数据挪前面来
				index -= len;
				delete[] recvBuf;	//短链接用一次就可以释放了，长连接这块应该要保留。只用一次做挪数据的修整没有意义吧
				return m_packet.sCmd;
			}
		}
		delete[] recvBuf;
		return -1;
	}

	bool Send(const char* pData,int nSize) {
		if (m_sockCli == -1)return false;
		return send(m_sockCli, pData, nSize, 0) > 0;
	}

	bool Send(CPacket& pack) {		//为何传类类型的引用必须要加const？
		if (m_sockCli == -1)return false;
		TRACE("send file:%s\r\n", pack.strData.c_str());	
		return send(m_sockCli, pack.Data(), pack.Size(), 0) > 0;
	}

	void CloseClient() {
		if (m_sockCli != INVALID_SOCKET) {
			closesocket(m_sockCli);
			m_sockCli = INVALID_SOCKET;
		}
	}
private:
	SOCKET m_sockSrv;
	SOCKET m_sockCli;
	CPacket m_packet;
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	std::list<CPacket> pack_list;
	CServerSocket(){
		m_sockSrv = INVALID_SOCKET;
		m_sockCli = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("初始化套接字环境失败，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
	}
	CServerSocket(const CServerSocket& ss){
		m_sockSrv = ss.m_sockSrv;
		m_sockCli = ss.m_sockCli;
	}
	CServerSocket& operator=(const CServerSocket&){}
	~CServerSocket(){
		closesocket(m_sockCli);
		closesocket(m_sockSrv);
		WSACleanup();
	}
	static CServerSocket* m_instance;

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
				TRACE("CGarbo释放m_instance\r\n");
				delete m_instance;
		}
	};
	static CGarbo garbo;
};

