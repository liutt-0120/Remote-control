#pragma once

#include "pch.h"
#include "framework.h"

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

class CServerSocket {
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	//socket相关函数
	bool InitSocket() {
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sockSrv == -1)return false;

		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(6000);

		//绑定
		if (-1 == bind(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) return false;
		//
		if (-1 == listen(m_sockSrv, 1)) return false;
		return true;
	}

	bool AccpetClient() {
		SOCKADDR_IN addrCli;
		int iLen = sizeof(SOCKADDR);
		m_sockCli = accept(m_sockSrv, (SOCKADDR*)&addrCli, &iLen);
		if (m_sockCli == -1)return false;
		return true;
	}

	bool ForRecv() {
		if (m_sockCli == -1)return false;
		char recvbuf[1024] = "";
		while (true) {
			int len = recv(m_sockCli, recvbuf, sizeof(recvbuf), 0);
			if (len <= 0) {
				return false;
			}
		}
		return true;
	}

	bool ForSend() {
		if (m_sockCli == -1)return false;
		char sendbuf[1024] = "";
		while (true) {
			int len = send(m_sockCli, sendbuf, sizeof(sendbuf), 0);
			if (len <= 0) {
				return false;
			}
		}
		return true;
	}
private:
	SOCKET m_sockSrv;
	SOCKET m_sockCli;
	CServerSocket(){
		m_sockSrv = INVALID_SOCKET;
		m_sockCli = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("初始化套接字环境失败，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
	}
	CServerSocket(const CServerSocket&){}
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
				delete m_instance;
		}
	};
	static CGarbo garbo;
};

