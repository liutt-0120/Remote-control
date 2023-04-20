// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

/// <summary>
/// debug：输出包数据
/// </summary>
/// <param name="pData"></param>
/// <param name="nSize"></param>
void Dump(BYTE* pData,size_t nSize) {
    std::string strOut;
    char buf[8] = "";
    for (size_t i = 0; i < nSize; i++) {
        if (i > 0 && (i % 16 == 0)) strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str()); 
    // error output:FF FE CC CC 09 00 00 00 01 00 CC CC 98 5B 83
    // CC CC用了#pragma pack(push) #pragma pack(1) #pragma pack(pop)处理
    // 后面的数据输出也不对，因为CPacket里数据用string存储，封装的数据部分其实是数据的首地址。
    // 解决方法：新增CPacket::Data()处理包数据
    // 改后值：FF FE 09 00 00 00 01 00 43 2C 44 2C 45 F0 CD  （√）
}

int MakeDriverInfo() {
    std::string result;
    for (int i = 1; i <= 26; ++i) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0)
                result += ',';      //tip，骚
            result += 'A' + i - 1;
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());  //打包
    //Dump((BYTE*)&pack, pack.nLength + 6);     //直接使用封装的包，其中的string存储的是数据的首地址，因此传出的包数据有误
    //现使用Data()整理包数据
    Dump((BYTE*)pack.Data(), pack.Size());
    //CServerSocket::getInstance()->Send(CPacket(1, (BYTE*)result.c_str(), result.size()));   //这个报错很正常啊居然在这疑惑半天，这是个临时变量，引用肯定不能绑定这
    //CServerSocket::getInstance()->Send(pack);                                               // 这感觉也很危险，把个局部变量给引用
    return 0;
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            //// TODO: 在此处为应用程序的行为编写代码。
            //CServerSocket* pserver = CServerSocket::getInstance();
            //if (pserver != NULL) {
            //    if(pserver->InitSocket()==false){
            //        MessageBox(NULL, _T("网络初始化异常，请检查网络状态"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
            //        exit(0);
            //    }
            //}
            //int count = 0;
            //while (pserver != NULL) {
            //    if (pserver->AccpetClient() == false) {
            //        if (count >= 3) {
            //            MessageBox(NULL, _T("多次无法正常接入用户，结束程序！"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(NULL, _T("无法正常接入用户，自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }
            //    else count = 0;
            //    int ret = pserver->DealCommand();
            //    //TODO:
            //}

            int nCmd = 1;
            switch (nCmd)
            {
            case 1:     //查看磁盘分区
                MakeDriverInfo();
                break;
            default:
                break;
            }


        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
