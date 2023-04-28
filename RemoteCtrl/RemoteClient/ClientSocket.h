#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#pragma pack(push)
#pragma pack(1)

#define MAX_SIZE 4096000

class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {
		TRACE("shead:%d,nlength:%d,scmd:%d,ssum:%d\r\n", sHead, nLength, sCmd, sSum);
	}
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
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) :CPacket() {
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
			TRACE("%s\r\n", strData.c_str());
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
	~CPacket() {}

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

typedef struct MouseEvent {
	MouseEvent() :nAction(0), nButton(-1) {
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	//动作：点击、双击、移动、拖动...
	WORD nButton;	//左键(0)、右键(1)、中键(2)
	POINT ptXY;		//坐标
}MouseEv, * PMouseEv;

typedef struct FileInfo {
	FileInfo() :isInvalid(false), isDirectory(-1), hasNext(true) {
		memset(szFileName, 0, sizeof(szFileName));
	}
	bool isInvalid;     //是否无效目录/文件：0 否；1 是
	char szFileName[256];
	bool isDirectory;   //是否为目录：0 否 ；1 是
	bool hasNext;       //是否还有后续： 0 没有；1 有

}*PFileInfo;

/// <summary>
/// 一个错误代码解析方法
/// </summary>
/// <param name="wsaErrCode"></param>
/// <returns></returns>
std::string GetErrorInfo(int wsaErrCode);

/// <summary>
/// 
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
	bool InitSocket(int nIP,int nPort) {
		if (m_sockSrv != INVALID_SOCKET) CloseSocket();
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sockSrv == -1)return false;

		SOCKADDR_IN addrSrv;
		//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		TRACE("addr:%08x , ip:%08x\r\n", inet_addr("127.0.0.1"), nIP);	//addr:0100007f , ip:7f000001
		addrSrv.sin_addr.S_un.S_addr = htonl(nIP);	//The htonl function converts a u_long from host to TCP/IP network byte order (which is big-endian).
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(nPort);
		if (addrSrv.sin_addr.S_un.S_addr == INADDR_NONE) {
			AfxMessageBox("指定的IP地址不存在");	//mfc
			return false;
		}
		int ret = connect(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		if (ret == SOCKET_ERROR) {
			AfxMessageBox("连接失败");	//mfc
			TRACE("连接失败：%d\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
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

	bool Send(CPacket& pack) {		//为何传类类型的引用必须要加const？
		if (m_sockSrv == -1)return false;
		return send(m_sockSrv, pack.Data(), pack.Size(), 0) > 0;
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
private:
	SOCKET m_sockSrv;
	CPacket m_packet;
	std::vector<char>m_buffer;	//使用容器，不用担心内存泄漏
	CClientSocket() {
		m_sockSrv = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("初始化套接字环境失败，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(MAX_SIZE);	//初始化size为4096
	}
	CClientSocket(const CClientSocket&) {}
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

