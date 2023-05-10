#pragma once

#include "pch.h"
#include "framework.h"
#include <list>
#include "CPacket.h"
#define MAX_SIZE 409600


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



typedef void(*SOCKET_CALLBACK)(void* arg, int status, std::list<CPacket>& pack_list,CPacket& inPacket);
class CServerSocket {
public:
	int RunServer(SOCKET_CALLBACK callback,void* arg,int port = 9527) {

		//socketl���ӳ�ʼ��
		bool bRet = InitSocket(port);
		if (!bRet) {
			return -1;
		}
		//����˽��տͻ�������
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				++count;
			}
			//��������
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
			TRACE("CServerSocket* getInstance��ʼ��m_instance\r\n");
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
protected:
	//socket��غ���
	bool InitSocket(int port) {
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sockSrv == -1)return false;

		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(9527);

		//��
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
		char* recvBuf = new char[MAX_SIZE];	//�������ϴ󣬴洢������
		if (recvBuf == NULL) {
			TRACE("�ڴ治�㣡\r\n");
			return -2;
		}
		memset(recvBuf, 0, MAX_SIZE);
		size_t index = 0;
		while (true)  {
			size_t len = recv(m_sockCli, recvBuf + index, MAX_SIZE - index, 0);	//len --- ���ν��յ����ݵĳ���
			if (len <= 0) {
				delete[] recvBuf;
				return -1;
			}
			index += len;	//������ʼλ��
			len = index;	//ʵ�ʽ��յ����ݵĳ���
			m_packet = CPacket((BYTE*)recvBuf, len);	//len����ʵ�ʴ���İ��ĳ���
			if (len > 0) {
				memmove(recvBuf, recvBuf + len, MAX_SIZE - len);	//���ѱ�����İ���ռ���ڴ�λ���ͷţ����������Ųǰ����
				index -= len;
				delete[] recvBuf;	//��������һ�ξͿ����ͷ��ˣ����������Ӧ��Ҫ������ֻ��һ����Ų���ݵ�����û�������
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

	bool Send(CPacket& pack) {		//Ϊ�δ������͵����ñ���Ҫ��const��
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
			MessageBox(NULL, _T("��ʼ���׽��ֻ���ʧ�ܣ�������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
				TRACE("CGarbo�ͷ�m_instance\r\n");
				delete m_instance;
		}
	};
	static CGarbo garbo;
};

