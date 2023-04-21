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

#include <io.h>
#include <list>
typedef struct FileInfo{
    FileInfo():isInvalid(false),isDirectory(-1),hasNext(true) {
        memset(szFileName, 0, sizeof(szFileName));
    }
    bool isInvalid;     //是否无效目录/文件：0 否；1 是
    char szFileName[256];
    bool isDirectory;   //是否为目录：0 否 ；1 是
    bool hasNext;       //是否还有后续： 0 没有；1 有

}*PFileInfo;

int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FileInfo> lstFileInfos;     //这块本来计划用list拿了所有文件后统一发送，但鉴于有的文件夹可能包含大量文件，如日志，网页的临时文件等，那处理完所有文件将耗费大量时间，甲方很可能以为卡了忍不住疯狂点击。
                                            
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前命令有误，不是获取文件列表"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        FileInfo fInfo;
        fInfo.isInvalid = true;
        fInfo.isDirectory = true;
        memcpy(fInfo.szFileName, strPath.c_str(), strPath.size());
        //lstFileInfos.push_back(fInfo);
        CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有访问目录的权限"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    }
    do {
        FileInfo fInfo;
        fInfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(fInfo.szFileName, fdata.name, strlen(fdata.name));
        //lstFileInfos.push_back(fInfo);
        CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
        CServerSocket::getInstance()->Send(pack);       //现计划处理一个就发一个
    } while (!_findnext(hfind, &fdata));
    
    FileInfo fInfo;         //这感觉可以一个函数都用这一个FileInfo啊？♥
    fInfo.hasNext = false;
    CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
    CServerSocket::getInstance()->Send(pack);

    return 0;
}

int RunFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    return 0;
}

int DownloadFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb"); //在客户端那边叫下载，在服务端这块叫（本地）上传、发送，因此是读文件。不知道客户端要下载哪类文件，按二进制方式读。
    if (err != 0) {
        CPacket pack(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != NULL) {
        fseek(pFile, 0, SEEK_END);  //SEEK_END文件结尾
        data = _ftelli64(pFile);
        CPacket fLenPack(4, (BYTE*)&data, 8);   //先把获取到的文件大小发过去
        fseek(pFile, 0, SEEK_SET);  //SEEK_SET文件开头

        char buffer[1024] = ""; //不超过1K，避免被截断
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);   //读1K发一次
        } while (rlen >= 1024);
        fclose(pFile);      //有开有关
    }
    
    CPacket pack(4, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
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
            case 2:
                MakeDirectoryInfo();
                break;
            case 3:
                RunFile();
                break;
            case 4:
                DownloadFile();
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
