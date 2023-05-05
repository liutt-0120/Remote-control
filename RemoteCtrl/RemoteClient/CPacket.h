#pragma once
#include "pch.h"
#include "framework.h"
#include <string>

#pragma pack(push)
#pragma pack(1)


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
	CPacket(const BYTE* pData, size_t& nSize) :CPacket() {
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

	const char* Data(std::string& strOut) const {
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