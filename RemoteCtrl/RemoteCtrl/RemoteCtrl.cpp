// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "Command.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

/// <summary>
/// 写注册表
/// </summary>
void WriteRegisterTable(const CString& strPath) {
    //将程序mklink进系统文件夹
    char sPath[MAX_PATH] = "";
    char sSys[MAX_PATH] = "";
    GetCurrentDirectoryA(MAX_PATH, sPath);  //获取当前路径
    GetSystemDirectoryA(sSys, sizeof(sSys));  //获取系统路径: C:\\Windows\\system32 获取的32,后面mklink往64里放...
    std::string strExe = "\\RemoteCtrl.exe";
    //std::string strCmd = "mklink " + std::string(sSys) + strExe+ " " + std::string(sPath) + strExe;
    std::string strCmd = "cmd /K mklink " + std::string(sSys) + strExe + " " + std::string(sPath) + strExe;
    //cmd /K --- 执行完命令控制台不关闭
    int ret = system(strCmd.c_str()); //控制台执行mklink命令
    TRACE("ret = %d\r\n", ret);
    //注册注册表
    HKEY hKey = NULL;
    CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    //ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_WRITE, &hKey); //以写入形式打开注册表
    ////                  系统定义的根部件   子件,具体到哪个路径
    ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey); //以写入形式打开注册表
    //                                                                win10 64位没↑写不进去
    if (ret != ERROR_SUCCESS) { //打开不成功，则：
        RegCloseKey(hKey);
        MessageBox(NULL, _T("设置自动开机启动失败，是否权限不足？"), _T("error"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }
    //使用REG_SZ：
    //TCHAR sSysPath[MAX_PATH] = _T("");
    //GetSystemDirectoryW(sSysPath, MAX_PATH);
    //CString strPath = sSysPath + CString(_T("\\RemoteCtrl.exe"));
    //ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
    //使用REG_EXPAND_SZ：
    //CString strPath = CString(_T("%SystemRoot%\\system32\\RemoteCtrl.exe"));
    CString strPath = CString(_T("%SystemRoot%\\SysWOW64\\RemoteCtrl.exe"));    //可以调用者传,也可以自己写带扩展的
    ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));

    if (ret != ERROR_SUCCESS) { //写入不成功，则：
        RegCloseKey(hKey);
        MessageBox(NULL, _T("设置自动开机启动失败，是否权限不足？"), _T("error"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }
    RegCloseKey(hKey);
}

void WriteStartupDir(const CString& strPath) {
    CString strPath = _T("C:\\Users\\72703\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe");
    CString strCmd = GetCommandLine();
    strCmd.Replace(_T("\""), _T("")); 
    BOOL ret = CopyFile(strCmd, strPath, FALSE);
    if (ret == FALSE) {
        MessageBox(NULL, _T("复制文件失败,是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
        ::exit(0);
    }
    //fopen\CFile\system(copy)\CopyFile\OpenFile都是可用来执行复制文件操作.
}
/// <summary>
/// 设置开机自启动
/// 开机启动的时候,程序的权限是跟随启动用户的
/// 如果两者权限不一致,则会导致程序启动失败
/// 
/// 环境变量对开机启动有影响,如果以来dll(动态库),则可能启动失败
/// 可以将这些dll也复制到system32/SysWOW64,或者设置静态库运行
/// </summary>
void ChooseAutoInvoke() {
    //CString path = _T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe");
    //                               ↑win10 64就这么骚,在SysWOW64里生成程序
    CString path = _T("C:\\Users\\72703\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe");
    if (PathFileExists(path)) {
        return;
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
        WriteStartupDir(path);
    }

}

void ShowError() {
    LPWSTR lpMessageBuf = NULL;
    //strerror(errno);  //标准C语言库: 获取系统错误消息字符串
    //windows错误一般用↓
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
        , NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        lpMessageBuf, 0, NULL);
    OutputDebugString(lpMessageBuf);
    LocalFree(lpMessageBuf);    //sys创建的allocate,free一下
}

bool IsAdmin() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        ShowError();
        return false;
    }
    TOKEN_ELEVATION eve;
    DWORD len = 0;
    if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
        //TOKEN_ELEVATION结构指示令牌是否具有提升的权限。有--非零值;否则为零值
        ShowError();
        return false;
    }
    CloseHandle(hToken);
    if (len == sizeof(eve)) {
        return eve.TokenIsElevated;
    }
    printf("length of tokeninformation is %d\r\n", len);
    return false;
}

int main()
{
    if (IsAdmin()) {
        OutputDebugString(L"current is run as administrator\r\n");
    }
    else {
        OutputDebugString(L"current is run as normal user\r\n");
    }
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
            // TODO: 在此处为应用程序的行为编写代码。
            CCommand cmd;
            ChooseAutoInvoke();
            CServerSocket* pserver = CServerSocket::getInstance();
            if (pserver != NULL) {
                int ret = pserver->RunServer(CCommand::RunCommand, &cmd);
                switch (ret) {
                case -1:
                    MessageBox(NULL, _T("网络初始化异常，请检查网络状态"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                    exit(0);
                    break;
                case -2:
                    MessageBox(NULL, _T("多次无法正常接入用户，结束程序！"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                    exit(0);
                    break;
                }
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
