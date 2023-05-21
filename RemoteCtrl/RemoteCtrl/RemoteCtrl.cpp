// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "Command.h"
#include <conio.h>
#include "MyQueue.h"
#include <MSWSock.h>
#include "MyServer.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define INVOKE_PATH _T("C:\\Users\\72703\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")
//define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe")        用注册表就搞这个
//                                      ↑win10 64就这么骚,32位的程序基本在SysWOW64里生成,不管你获取的系统路径是不是 C:\\Windows\\system32

// 唯一的应用程序对象

CWinApp theApp;

/// <summary>
/// 设置开机自启动
/// 开机启动的时候,程序的权限是跟随启动用户的
/// 如果两者权限不一致,则会导致程序启动失败
/// 
/// 环境变量对开机启动有影响,如果以来dll(动态库),则可能启动失败
/// 可以将这些dll也复制到system32/SysWOW64,或者设置静态库运行
/// </summary>
static bool ChooseAutoInvoke(const CString& strPath) {
    if (PathFileExists(strPath)) {
        return false;
    }
    CString strInfo = _T("该程序只允许用于合法的用途！\n");
    strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态！\n");
    strInfo += _T("如果你不希望这样，请按“取消”按钮，退出程序。\n");
    strInfo += _T("按下“是”按钮，该程序将被复制到你的机器上，并随系统启动而自动运行！\n");
    strInfo += _T("按下“否”按钮，程序只运行一次，不会在系统内留下任何东西！\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    //   是/否/取消     警告标志和声音    在最前面，提醒用户
    if (ret == IDYES) {
        //WriteRegisterTable(path);
        if (!CMyTool::WriteStartupDir(strPath)) {
            MessageBox(NULL, _T("复制文件失败,是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
    }
    else if (ret == IDCANCEL) {
        return false;
    }
    return true;
}
bool InitSockEnv() {
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
        return FALSE;
    }
    return TRUE;
}
void iocp() {
    bool ret = InitSockEnv();
    if (!ret) {
        TRACE("初始化环境失败\r\n");
        ::exit(0);
    }
    CMyServer server;
    server.StartService();
    getchar();
}
int main()
{
    if (!CMyTool::Init()) return 1;
    iocp();
    /*
if (CMyTool::IsAdmin()) {
    //OutputDebugString(L"current is run as administrator\r\n");
    //MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
    if (!CMyTool::Init()) return 1;
    if (ChooseAutoInvoke(INVOKE_PATH)) {
        CCommand cmd;
        CServerSocket* pserver = CServerSocket::getInstance();
        if (pserver != NULL) {
            int ret = pserver->RunServer(CCommand::RunCommand, &cmd);
            switch (ret) {
            case -1:
                MessageBox(NULL, _T("网络初始化异常，请检查网络状态"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                break;
            case -2:
                MessageBox(NULL, _T("多次无法正常接入用户，结束程序！"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                break;
            }
        }
    }
}
else {
    //OutputDebugString(L"current is run as normal user\r\n");
    //MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
    if (!CMyTool::RunAsAdmin()) {
        CMyTool::ShowError();
        return 1;
    }
}
*/
    return 0;
}

/*
class COverlapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;       //操作
    char m_buffer[4096];    //传值
    COverlapped() {
        m_operator = 0;
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        memset(m_buffer, 0, sizeof(m_buffer));
    }
};
*/

