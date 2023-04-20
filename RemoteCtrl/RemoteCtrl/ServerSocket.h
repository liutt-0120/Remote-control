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
	/// 封包的构造函数重载
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
	/// 解包的构造函数重载
	/// </summary>
	/// <param name="pData">包</param>
	/// <param name="nSize">接收的信息长度</param>
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		//查找包头
		for (; i < nSize; ++i) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {	//相当于是若这个包只有长度(4)、命令(2)、校验(2)或其中部分，那没数据啊，没必要解析；或有数据但包协议不完整，也不校验
			nSize = 0;
			TRACE("%s(%d):%s,解析失败:%s\n", __FILE__, __LINE__, __FUNCTION__, "");
			return;
		}
		//获取长度
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize || nLength <= 4) {		//前者意味着这次传输的包未完全接收到，不完整；后者意味着要么包里没数据，要么包不完整，都没必要解析
			nSize = 0;
			TRACE("%s(%d):%s,解析失败:%s\n", __FILE__, __LINE__, __FUNCTION__, "");
			return;
		}
		//获取命令
		sCmd = *(WORD*)(pData + i); i += 2;
		//获取数据
		strData.resize(nLength - 2 - 2);
		memcpy((void*)strData.c_str(), pData + i, nLength - 4);	//将包内数据传给strData
		i += nLength - 4;
		//获取校验
		sSum = *(WORD*)(pData + i); i += 2;
		//验证校验，这块的和校验只将数据求和
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;	//长度值（从控制命令开始，到和校验结束）+包头(2)+长度(4) = 包长度
			return;
		}
		nSize = 0;
		return;
	}
	~CPacket(){}

	int Size() {	//包的大小
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
	WORD sHead;		//包头，固定为0xFEFF
	DWORD nLength;	//长度（从控制命令开始，到和校验结束）
	WORD sCmd;		//控制命令
	std::string strData;	//数据
	WORD sSum;		//和校验

	std::string strOut;	//输出组合起来的整个包数据
};

#pragma pack(pop)

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

	int DealCommand() {
		if (m_sockCli == -1)return -1;
		//char recvbuf[1024] = "";
		char* recvBuf = new char[MAX_SIZE];	//缓冲区较大，存储到堆上
		memset(recvBuf, 0, MAX_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_sockCli, recvBuf + index, MAX_SIZE - index, 0);	//len --- 本次接收到数据的长度
			if (len <= 0) {
				return -1;
			}
			index += len;	//修正起始位置
			len = index;	//实际接收到数据的长度
			m_packet = CPacket((BYTE*)recvBuf, len);	//len接收实际处理的包的长度
			if (len > 0) {
				memmove(recvBuf, recvBuf + len, MAX_SIZE - len);	//将已被处理的包所占的内存位置释放：后面的数据挪前面来
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

	bool Send(CPacket& pack) {		//为何传类类型的引用必须要加const？
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

