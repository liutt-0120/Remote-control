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
	CPacket(const BYTE* pData, size_t& nSize) :CPacket() {
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