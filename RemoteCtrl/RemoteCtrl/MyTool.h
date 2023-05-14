#pragma once

/// <summary>
/// ������
/// </summary>
class CMyTool
{
public:
    /// <summary>
/// debug�����������
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
/// ������Ϣ��ʾ
/// </summary>
    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        //strerror(errno);  //��׼C���Կ�: ��ȡϵͳ������Ϣ�ַ���
        //windows����һ���á�
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
            , NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);
        LocalFree(lpMessageBuf);    //sys������allocate,freeһ��
    }

    /// <summary>
    /// ����Ȩ�޼��
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
            //TOKEN_ELEVATION�ṹָʾ�����Ƿ����������Ȩ�ޡ���--����ֵ;����Ϊ��ֵ
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
    /// ��ȡ����ԱȨ��, ʹ�ø�Ȩ�޴�������
    /// </summary>
    /// <returns></returns>
    static bool RunAsAdmin() {
        //ʹ�ù���ԱȨ�޴�������
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        //����ӵ��adminȨ�޵��ӽ���
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
    /// ͨ���޸Ŀ��������ļ���ʵ�ֿ�������
    /// </summary>
    /// <param name="strPath"></param>
    /// <returns></returns>
    static bool WriteStartupDir(const CString& strPath) {
        //��ȡ��������·��
        //CString strCmd = GetCommandLine();
        //strCmd.Replace(_T("\""), _T(""));
        //��������д
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        return CopyFile(sPath, strPath, FALSE);
        //fopen\CFile\system(copy)\CopyFile\OpenFile���ǿ�����ִ�и����ļ�����.
    }

    /// <summary>
/// дע���ʵ�ֿ�������
/// </summary>
    static bool WriteRegisterTable(const CString& strPath) {
        //������mklink��ϵͳ�ļ���
        char sPath[MAX_PATH] = "";
        char sSys[MAX_PATH] = "";
        GetCurrentDirectoryA(MAX_PATH, sPath);  //��ȡ��ǰ·��
        GetSystemDirectoryA(sSys, sizeof(sSys));  //��ȡϵͳ·��: C:\\Windows\\system32 ��ȡ��32,����mklink��WOW64���...
        std::string strExe = "\\RemoteCtrl.exe";
        //std::string strCmd = "mklink " + std::string(sSys) + strExe+ " " + std::string(sPath) + strExe;
        std::string strCmd = "cmd /K mklink " + std::string(sSys) + strExe + " " + std::string(sPath) + strExe;
        //cmd /K --- ִ�����������̨���ر�
        int ret = system(strCmd.c_str()); //����ִ̨��mklink����
        TRACE("ret = %d\r\n", ret);
        //ע��ע���
        HKEY hKey = NULL;
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
        //ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_WRITE, &hKey); //��д����ʽ��ע���
        ////                  ϵͳ����ĸ�����   �Ӽ�,���嵽�ĸ�·��
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey); //��д����ʽ��ע���
        //                                                                win10 64λû��д����ȥ
        if (ret != ERROR_SUCCESS) { //�򿪲��ɹ�����
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿"), _T("error"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        //ʹ��REG_SZ��
        //TCHAR sSysPath[MAX_PATH] = _T("");
        //GetSystemDirectoryW(sSysPath, MAX_PATH);
        //CString strPath = sSysPath + CString(_T("\\RemoteCtrl.exe"));
        //ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
        //ʹ��REG_EXPAND_SZ��
        //CString strPath = CString(_T("%SystemRoot%\\SysWOW64\\RemoteCtrl.exe"));    //���Ե����ߴ�,Ҳ�����Լ�д����չ��
        //ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));

        if (ret != ERROR_SUCCESS) { //д�벻�ɹ�����
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿"), _T("error"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }

    /// <summary>
    /// ���ڴ�MFC��������Ŀ��ʼ��(ͨ��)
    /// </summary>
    /// <returns></returns>
    static bool Init() {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr)
        {
            wprintf(L"����: GetModuleHandle ʧ��\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
            wprintf(L"����: MFC ��ʼ��ʧ��\n");
            return false;
        }
        return true;
    }
};

