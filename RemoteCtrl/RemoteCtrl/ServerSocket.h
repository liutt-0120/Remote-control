#pragma once

#include "pch.h"
#include "framework.h"

#pragma pack(push)
#pragma pack(1)

#define MAX_SIZE 4096

class CPacket {
public:
	CPacket():sHead(0),nLength(0),sCmd(0),sSum(0){}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	/// <summary>
	/// ����Ĺ��캯������
	/// </summary>
	/// <param name="nCmd"></param>
	/// <param name="pData"></param>
	/// <param name="nSize"></param>
	CPacket(WORD nCmd,const BYTE* pData,size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 2 + 2;
		sCmd = nCmd;
		strData.resize(nSize);
		memcpy((void*)strData.c_str(), pData, nSize);
		for (int i = 0; i < nSize; ++i) {
			sSum += strData[i] & 0xFF;
		}
	}

	/// <summary>
	/// ����Ĺ��캯������
	/// </summary>
	/// <param name="pData">��</param>
	/// <param name="nSize">���յ���Ϣ����</param>
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		//���Ұ�ͷ
		for (; i < nSize; ++i) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {	//�൱�����������ֻ�г���(4)������(2)��У��(2)�����в��֣���û���ݰ���û��Ҫ�������������ݵ���Э�鲻������Ҳ��У��
			nSize = 0;
			TRACE("%s(%d):%s,����ʧ��:%s\n", __FILE__, __LINE__, __FUNCTION__, "");
			return;
		}
		//��ȡ����
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize || nLength <= 4) {		//ǰ����ζ����δ���İ�δ��ȫ���յ�����������������ζ��Ҫô����û���ݣ�Ҫô������������û��Ҫ����
			nSize = 0;
			TRACE("%s(%d):%s,����ʧ��:%s\n", __FILE__, __LINE__, __FUNCTION__, "");
			return;
		}
		//��ȡ����
		sCmd = *(WORD*)(pData + i); i += 2;
		//��ȡ����
		strData.resize(nLength - 2 - 2);
		memcpy((void*)strData.c_str(), pData + i, nLength - 4);	//���������ݴ���strData
		i += nLength - 4;
		//��ȡУ��
		sSum = *(WORD*)(pData + i); i += 2;
		//��֤У�飬���ĺ�У��ֻ���������
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;	//����ֵ���ӿ������ʼ������У�������+��ͷ(2)+����(4) = ������
			return;
		}
		nSize = 0;
		return;
	}
	~CPacket(){}

	int Size() {	//���Ĵ�С
		return nLength + 6;
	}

	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;		//��ͷ���̶�Ϊ0xFEFF
	DWORD nLength;	//���ȣ��ӿ������ʼ������У�������
	WORD sCmd;		//��������
	std::string strData;	//����
	WORD sSum;		//��У��

	std::string strOut;	//����������������������
};

#pragma pack(pop)

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

	int DealCommand() {
		if (m_sockCli == -1)return -1;
		//char recvbuf[1024] = "";
		char* recvBuf = new char[MAX_SIZE];	//�������ϴ󣬴洢������
		memset(recvBuf, 0, MAX_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_sockCli, recvBuf + index, MAX_SIZE - index, 0);	//len --- ���ν��յ����ݵĳ���
			if (len <= 0) {
				return -1;
			}
			index += len;	//������ʼλ��
			len = index;	//ʵ�ʽ��յ����ݵĳ���
			m_packet = CPacket((BYTE*)recvBuf, len);	//len����ʵ�ʴ���İ��ĳ���
			if (len > 0) {
				memmove(recvBuf, recvBuf + len, MAX_SIZE - len);	//���ѱ�����İ���ռ���ڴ�λ���ͷţ����������Ųǰ����
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char* pData,int nSize) {
		if (m_sockCli == -1)return false;
		return send(m_sockCli, pData, nSize, 0) > 0;
	}

	bool Send(CPacket& pack) {		//Ϊ�δ������͵����ñ���Ҫ��const��
		if (m_sockCli == -1)return false;
		return send(m_sockCli, pack.Data(), pack.Size(), 0) > 0;
	}
private:
	SOCKET m_sockSrv;
	SOCKET m_sockCli;
	CPacket m_packet;
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

