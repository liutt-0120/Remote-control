#pragma once
#include <Windows.h>
#include <string>
#include <atlimage.h>


/// 工具类
/// </summary>
class CMyTool
{
public:
    /// <summary>
/// debug：输出包数据
/// </summary>
/// <param name="pData"></param>
/// <param name="nSize"></param>
    static void Dump(BYTE* pData, size_t nSize) {
        std::string strOut;
        char buf[8] = "";
        for (size_t i = 0; i < nSize; i++) {
            if (i > 0 && (i % 16 == 0)) strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }

	static int Byte2Image(CImage& image,const std::string& strBuffer) {
		BYTE* pData = (BYTE*)strBuffer.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL) {
			TRACE("内存不足");
			Sleep(1);
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		if (hRet == S_OK) {
			ULONG length = 0;
			pStream->Write(pData, strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			if ((HBITMAP)image != NULL) image.Destroy();
			image.Load(pStream);
		}
		return hRet;
	}



    /// <summary>
/// 错误代码解析
/// </summary>
/// <param name="wsaErrCode"></param>
/// <returns></returns>
	static std::string GetErrorInfo(int wsaErrCode) {
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

};