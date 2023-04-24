#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;


CClientSocket::CGarbo CClientSocket::garbo;		//临了临了，帮忙把静态指针delete了


/// <summary>
/// 一个错误代码解析方法，后面学一下里面的函数
/// </summary>
/// <param name="wsaErrCode"></param>
/// <returns></returns>
std::string GetErrorInfo(int wsaErrCode) {
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}