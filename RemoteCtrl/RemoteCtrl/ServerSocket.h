#pragma once

#include "pch.h"
#include "framework.h"

#pragma pack(push)
#pragma pack(1)

#define MAX_SIZE 4096000

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
	CPacket(WORD nCmd,const BYTE* pData,size_t nSize):CPacket() {
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
	CPacket(const BYTE* pData, size_t& nSize):CPacket() {
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
		if (nLength + i > nSize) {		//前者意味着这次传输的包未完全接收到，不完整；后者意味着要么包里没数据，要么包不完整，都没必要解析
			nSize = 0;
			TRACE("%s(%d):%s,解析失败:%s\n", __FILE__, __LINE__, __FUNCTION__, "");
			return;
		}
		//获取命令
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {	//nLength包含 2byte-cmd + 2byte-sum + ?byte-str，等于4说明只有cmd和sum没数据
			//获取数据
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);	//将包内数据传给strData
			i += nLength - 4;
		}
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
		*(DWORD*)(pData) = nLength; pData += 4;
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

typedef struct MouseEvent{
	MouseEvent():nAction(0),nButton(-1){
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	//动作：点击、双击、移动、拖动...
	WORD nButton;	//左键(0)、右键(1)、中键(2)
	POINT ptXY;		//坐标
}MouseEv,*PMouseEv;
typedef struct FileInfo {
	FileInfo() :isInvalid(false), isDirectory(-1), hasNext(true) {
		memset(szFileName, 0, sizeof(szFileName));
	}
	bool isInvalid;     //是否无效目录/文件：0 否；1 是
	char szFileName[256];
	bool isDirectory;   //是否为目录：0 否 ；1 是
	bool hasNext;       //是否还有后续： 0 没有；1 有

}*PFileInfo;
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
		addrSrv.sin_port = htons(9527);

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
		while (true) {
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

	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2)&&(m_packet.sCmd <= 4)||(m_packet.sCmd == 9)) {
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

	void CloseClient() {
		closesocket(m_sockCli);
		m_sockCli = INVALID_SOCKET;
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
				delete m_instance;
		}
	};
	static CGarbo garbo;
};

