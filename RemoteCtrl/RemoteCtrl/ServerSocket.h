#pragma once

#include "pch.h"
#include "framework.h"

///// <summary>
///// ��ʽһ��������Ŀ��ֻ����Ŀ���Startup����Ŀ��βCleanup�׽��ֻ������������ֿ��ŵ���ʽ�����Ա�֤���������Ŀ�в���ʵ������
///// </summary>
//class CServerSocket
//{
//public:
//	CServerSocket() {
//		if (InitSockEnv() == FALSE) {
//			MessageBox(NULL, _T("��ʼ���׽��ֻ���ʧ�ܣ�������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
//ȷ��Ψһ�Ծ�Ҫʹ�õ���ģʽ��ʹ���һ�������Ϊϵͳ�е�Ψһʵ��
//-----------------------------------------------------------------------------------------


///// <summary>
///// ��ʽ����
///// ���캯��˽�л������ⱻ����ʵ��
///// һ��˽�е�ָ��Ψһʵ���ľ�ָ̬�룻
///// ����һ�����еĺ��������Ի�ȡ���Ψһ��ʵ������������Ҫ��ʱ�򴴽���ʵ����
///// </summary>
//class CServerSocket
//{
//public:
//	//��¶�����������Ѿ����㵥��ģʽ
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
//			MessageBox(NULL, _T("��ʼ���׽��ֻ���ʧ�ܣ�������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
* ��ʽ���Ѿ������㵥��ģʽ��CServerSocket* server = CServerSocket::getInstance();ȷ��Ψһһ��ʵ��������Ҫʱdelete server;����
* �������������⣺
*	1.�̲߳���ȫ�ģ����ɣ�����˵new���̰߳�ȫ������˵���ǣ�
*	2.delete�����ױ����ǣ�����Ҳ���ѱ�֤��delete֮��û�д����ٵ���GetInstance������
* һ�����Ƶķ�������������Լ�֪���ں��ʵ�ʱ����Լ�ɾ��������˵��ɾ���Լ��Ĳ������ڲ���ϵͳ�е�ĳ�����ʵĵ��ϣ�ʹ����ǡ����ʱ���Զ�ִ�С�
* ����֪���������ڽ�����ʱ��ϵͳ���Զ��������е�ȫ�ֱ�����
* ��ʵ�ϣ�ϵͳҲ���������е���ľ�̬��Ա������������Щ��̬��ԱҲ��ȫ�ֱ���һ����
* ������������������ڵ������ж���һ�������ľ�̬��Ա������Ψһ��������������������ɾ���������ʵ����
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

	//socket��غ���
	bool InitSocket() {
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sockSrv == -1)return false;

		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(6000);

		//��
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
			MessageBox(NULL, _T("��ʼ���׽��ֻ���ʧ�ܣ�������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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

