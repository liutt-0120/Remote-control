#pragma once

/// <summary>
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

    /// <summary>
/// 错误信息显示
/// </summary>
    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        //strerror(errno);  //标准C语言库: 获取系统错误消息字符串
        //windows错误一般用↓
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
            , NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);
        LocalFree(lpMessageBuf);    //sys创建的allocate,free一下
    }

    /// <summary>
    /// 运行权限检测
    /// </summary>
    /// <returns></returns>
    static bool IsAdmin() {
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
        return true;
    }

    /// <summary>
    /// 获取管理员权限, 使用该权限创建进程
    /// </summary>
    /// <returns></returns>
    static bool RunAsAdmin() {
        //使用管理员权限创建进程
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        //创建拥有admin权限的子进程
        bool ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
        if (!ret) {
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    /// <summary>
    /// 通过修改开机启动文件夹实现开机启动
    /// </summary>
    /// <param name="strPath"></param>
    /// <returns></returns>
    static bool WriteStartupDir(const CString& strPath) {
        //获取进程所在路径
        //CString strCmd = GetCommandLine();
        //strCmd.Replace(_T("\""), _T(""));
        //或者这样写
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        return CopyFile(sPath, strPath, FALSE);
        //fopen\CFile\system(copy)\CopyFile\OpenFile都是可用来执行复制文件操作.
    }

    /// <summary>
/// 写注册表实现开机启动
/// </summary>
    static bool WriteRegisterTable(const CString& strPath) {
        //将程序mklink进系统文件夹
        char sPath[MAX_PATH] = "";
        char sSys[MAX_PATH] = "";
        GetCurrentDirectoryA(MAX_PATH, sPath);  //获取当前路径
        GetSystemDirectoryA(sSys, sizeof(sSys));  //获取系统路径: C:\\Windows\\system32 获取的32,后面mklink往WOW64里放...
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
            return false;
        }
        //使用REG_SZ：
        //TCHAR sSysPath[MAX_PATH] = _T("");
        //GetSystemDirectoryW(sSysPath, MAX_PATH);
        //CString strPath = sSysPath + CString(_T("\\RemoteCtrl.exe"));
        //ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
        //使用REG_EXPAND_SZ：
        //CString strPath = CString(_T("%SystemRoot%\\SysWOW64\\RemoteCtrl.exe"));    //可以调用者传,也可以自己写带扩展的
        //ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));

        if (ret != ERROR_SUCCESS) { //写入不成功，则：
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败，是否权限不足？"), _T("error"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }

    /// <summary>
    /// 用于带MFC命令行项目初始化(通用)
    /// </summary>
    /// <returns></returns>
    static bool Init() {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr)
        {
            wprintf(L"错误: GetModuleHandle 失败\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            return false;
        }
        return true;
    }
};

