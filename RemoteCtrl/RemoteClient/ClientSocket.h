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
	/// ����Ĺ��캯������
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
	/// ����Ĺ��캯������
	/// </summary>
	/// <param name="pData">��</param>
	/// <param name="nSize">���յ���Ϣ����</param>
	CPacket(const BYTE* pData, size_t& nSize):CPacket() {
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
		if (nLength + i > nSize) {		//ǰ����ζ����δ���İ�δ��ȫ���յ�����������������ζ��Ҫô����û���ݣ�Ҫô������������û��Ҫ����
			nSize = 0;
			TRACE("%s(%d):%s,����ʧ��:%s\n", __FILE__, __LINE__, __FUNCTION__, "");
			return;
		}
		//��ȡ����
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {	//nLength���� 2byte-cmd + 2byte-sum + ?byte-str������4˵��ֻ��cmd��sumû����
			//��ȡ����
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);	//���������ݴ���strData
			TRACE("%s\r\n", strData.c_str());
			i += nLength - 4;
		}

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
	~CPacket() {}

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

typedef struct MouseEvent {
	MouseEvent() :nAction(0), nButton(-1) {
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	//�����������˫�����ƶ����϶�...
	WORD nButton;	//���(0)���Ҽ�(1)���м�(2)
	POINT ptXY;		//����
}MouseEv, * PMouseEv;

typedef struct FileInfo {
	FileInfo() :isInvalid(false), isDirectory(-1), hasNext(true) {
		memset(szFileName, 0, sizeof(szFileName));
	}
	bool isInvalid;     //�Ƿ���ЧĿ¼/�ļ���0 ��1 ��
	char szFileName[256];
	bool isDirectory;   //�Ƿ�ΪĿ¼��0 �� ��1 ��
	bool hasNext;       //�Ƿ��к����� 0 û�У�1 ��

}*PFileInfo;

/// <summary>
/// һ����������������
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

	//socket��غ���
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
			AfxMessageBox("ָ����IP��ַ������");	//mfc
			return false;
		}
		int ret = connect(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		if (ret == SOCKET_ERROR) {
			AfxMessageBox("����ʧ��");	//mfc
			TRACE("����ʧ�ܣ�%d\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
		}
		return true;
	}

	int DealCommand() {
		if (m_sockSrv == -1)return -1;
		//char recvbuf[1024] = "";
		//char* recvBuf = new char[MAX_SIZE];	//���ó�Ա����
		char* recvBuf = m_buffer.data();	//��buffer���ж��������һһ����
		static size_t index = 0;
		while (true) {
			size_t len = recv(m_sockSrv, recvBuf + index, MAX_SIZE - index, 0);	//len --- ���ν��յ����ݵĳ���
			if (((int)len <= 0) && ((int)index <= 0)) {
				return -1;
			}
			TRACE("recv len = %d(0x%08X) index = %d(0x%08X)\r\n", len, len, index, index);
			index += len;	//������ʼλ��
			len = index;	//ʵ�ʽ��յ����ݵĳ���
			TRACE("recv len = %d(0x%08X) index = %d(0x%08X)\r\n", len, len, index, index);
			m_packet = CPacket((BYTE*)recvBuf, len);	//len����ʵ�ʴ���İ��ĳ���
			if (len > 0) {
				memmove(recvBuf, recvBuf + len, MAX_SIZE - len);	//���ѱ�����İ���ռ���ڴ�λ���ͷţ����������Ųǰ����
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

	bool Send(CPacket& pack) {		//Ϊ�δ������͵����ñ���Ҫ��const��
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
	std::vector<char>m_buffer;	//ʹ�����������õ����ڴ�й©
	CClientSocket() {
		m_sockSrv = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("��ʼ���׽��ֻ���ʧ�ܣ�������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(MAX_SIZE);	//��ʼ��sizeΪ4096
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

