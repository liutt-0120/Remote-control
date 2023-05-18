#pragma once

#include <map>
#include <atlimage.h>
#include <direct.h>
#include <io.h>
#include <list>
#include "resource.h"
#include "CPacket.h"
#include "MyTool.h"
#include "LockDialog.h"


#pragma warning(disable:4966)  //

class CCommand
{
public:
    CCommand();
	~CCommand() {};
	/// <summary>
	/// �������ں���
	/// </summary>
	/// <param name="nCmd"></param>
	/// <param name="pack_list"></param>
	/// <param name="inPacket"></param>
	/// <returns></returns>
	int ExcuteCommand(int nCmd, std::list<CPacket>& pack_list, CPacket& inPacket);

    /// <summary>
    /// �ص�����
    /// </summary>
    /// <param name="arg"></param>
    /// <param name="status"></param>
    /// <param name="pack_list"></param>
    /// <param name="inPacket"></param>
    static void RunCommand(void* arg, int status, std::list<CPacket>& pack_list, CPacket& inPacket) {
        CCommand* thiz = (CCommand*)arg;
        int nRet = thiz->ExcuteCommand(status, pack_list, inPacket);
        if(nRet != 0)
            TRACE("ִ������ʧ�ܣ�%d ret = %d\r\n", status, nRet);


    }
protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket&);	//��Ա����ָ��
	std::map<int, CMDFUNC> m_mapFunc;	//����ӳ�书��
    CLockDialog dlg;
    unsigned threadId;
protected:
    static unsigned WINAPI LockScreenByThread(void* arg) {

        CCommand* thiz = (CCommand*)arg;
        thiz->LockScreen();
        _endthreadex(0);
        return 0;
    }
    void LockScreen() {
        TRACE("%s(%d),threadId = %d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
        dlg.Create(IDD_DLG_INFO, NULL);
        dlg.ShowWindow(SW_SHOW);
        //����ȫ����ʼ~~~??
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);     //��ȡ��ǰ�豸��������������
        rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) * 1.03;    //��ȡ��ǰ�豸���������������
        dlg.MoveWindow(rect);
        //����������Ļλ��
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
        if (pText) {
            CRect rText;
            pText->GetWindowRect(rText);
            int x = (rect.right - rText.Width()) / 2;
            int y = (rect.bottom - rText.Height()) / 2;
            pText->MoveWindow(x, y, rText.Width(), rText.Height());
        }
        //���ô����ö�??
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); //AppMsg - Warning: calling DestroyWindow in CDialog::~CDialog --
                                                                                //AppMsg - OnDestroy or PostNcDestroy in derived class will not be called
                                                                                //��Ϊ��ģ̬�Ի��򲻴�������������һ����Ъ���ˣ������ʾ����û��destroy
        ShowCursor(false);      //������깦��
        ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);   //����������
        //dlg.GetWindowRect(rect); 
        ClipCursor(rect);       //���������Χ
                                //˫�����£���������Ƶ�dlg�ڣ������ƹ���                                                        

        MSG msg;                //dialogһ��������mfc�ǻ�����Ϣѭ���ģ�����Ŀ�ﲢû����Ϣѭ�������Ҫ�Զ���һ����Ϣѭ������������
        while (GetMessage(&msg, dlg.m_hWnd, 0, 0)) {
            TranslateMessage(&msg);
            LRESULT ret = DispatchMessage(&msg);
            //���Լ����������
            if (msg.message == WM_KEYDOWN) {
                TRACE("msg:%08X wParam:%08x lParam:%08x\r\n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == VK_SPACE) {
                    TRACE(_T("�����ո�"));
                }

                if (msg.wParam == VK_ESCAPE)  //ESC
                    break;
            }
        }
        ClipCursor(NULL);   //�ָ������Χ
        ShowCursor(true);   //�ָ���깦��
        ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);   //��ʾ������
        dlg.DestroyWindow();

    }

    int MakeDriverInfo(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string result;
        for (int i = 1; i <= 26; ++i) {
            if (_chdrive(i) == 0) {
                if (result.size() > 0)
                    result += ',';      //tip ɧ
                result += 'A' + i - 1;
            }
        }
        //CPacket pack(1, (BYTE*)result.c_str(), result.size());  //���
        ////Dump((BYTE*)&pack, pack.nLength + 6);     //ֱ��ʹ�÷�װ�İ������е�string�洢�������ݵ��׵�ַ����˴����İ���������
        ////��ʹ��Data()���������
        //CMyTool::Dump((BYTE*)pack.Data(), pack.Size());
        //CServerSocket::getInstance()->Send(pack);
        
        pack_list.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }

    int MakeDirectoryInfo(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        //std::list<FileInfo> lstFileInfos;     //��鱾���ƻ���list���������ļ���ͳһ���ͣ��������е��ļ��п��ܰ��������ļ�������־����ҳ����ʱ�ļ��ȣ��Ǵ����������ļ����ķѴ���ʱ�䣬�׷��ܿ�����Ϊ�����̲�ס�������

        //if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        //    OutputDebugString(_T("��ǰ�������󣬲��ǻ�ȡ�ļ��б�"));
        //    return -1;
        //}
        if (_chdir(strPath.c_str()) != 0) {
            FileInfo fInfo;
            fInfo.hasNext = false;
            //lstFileInfos.push_back(fInfo);
            
            //CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
            //CServerSocket::getInstance()->Send(pack);
            pack_list.push_back(CPacket(2, (BYTE*)&fInfo, sizeof(fInfo)));
            OutputDebugString(_T("û�з���Ŀ¼��Ȩ��"));
            return -2;
        }
        _finddata_t fdata;
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("û���ҵ��κ��ļ�"));
            FileInfo fInfo;
            fInfo.hasNext = false;
            //CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
            //CServerSocket::getInstance()->Send(pack);
            pack_list.push_back(CPacket(2, (BYTE*)&fInfo, sizeof(fInfo)));
            return -3;
        }
        do {
            FileInfo fInfo;
            fInfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(fInfo.szFileName, fdata.name, strlen(fdata.name));
            //lstFileInfos.push_back(fInfo);
            TRACE("%s\r\n", fInfo.szFileName);
            //CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
            //CServerSocket::getInstance()->Send(pack);       //�ּƻ�����һ���ͷ�һ��
            pack_list.push_back(CPacket(2, (BYTE*)&fInfo, sizeof(fInfo)));
        } while (!_findnext(hfind, &fdata));

        FileInfo fInfo;         //��о�����һ������������һ��FileInfo����?
        fInfo.hasNext = false;
        //CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
        //CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(2, (BYTE*)&fInfo, sizeof(fInfo)));
        return 0;
    }

    int RunFile(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        //CServerSocket::getInstance()->GetFilePath(strPath);
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        //CPacket pack(3, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(3, NULL, 0));

        return 0;
    }

    int DownloadFile(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        //CServerSocket::getInstance()->GetFilePath(strPath);
        long long data = 0;
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb"); //�ڿͻ����Ǳ߽����أ��ڷ�������У����أ��ϴ������ͣ�����Ƕ��ļ�����֪���ͻ���Ҫ���������ļ����������Ʒ�ʽ����
        if (err != 0) {
            //CPacket pack(4, (BYTE*)&data, 8);
            //CServerSocket::getInstance()->Send(pack);
            pack_list.push_back(CPacket(4, (BYTE*)&data, 8));
            return -1;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);  //SEEK_END�ļ���β
            data = _ftelli64(pFile);
            //CPacket fLenPack(4, (BYTE*)&data, 8);   //�Ȱѻ�ȡ�����ļ���С����ȥ
            //CServerSocket::getInstance()->Send(fLenPack);
            pack_list.push_back(CPacket(4, (BYTE*)&data, 8));
            fseek(pFile, 0, SEEK_SET);  //SEEK_SET�ļ���ͷ

            char buffer[1024] = ""; //������1K�����ⱻ�ض�
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                //CPacket pack(4, (BYTE*)buffer, rlen);
                //CServerSocket::getInstance()->Send(pack);   //��1K��һ��
                pack_list.push_back(CPacket(4, (BYTE*)buffer, rlen));
            } while (rlen >= 1024);
            fclose(pFile);      //�п��й�
        }

        //CPacket pack(4, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(4, NULL, 0));
        return 0;
    }

    int MouseEvent(std::list<CPacket>& pack_list, CPacket& inPacket) {
        MouseEv mouse;
        memcpy(&mouse, inPacket.strData.c_str(), sizeof(MouseEv));

        DWORD nFlags = 0;
        switch (mouse.nButton) {
        case 0:         //���
            nFlags = 1; //0000 0001
            break;
        case 1:         //�Ҽ�
            nFlags = 2; //0000 0010
            break;
        case 2:         //�м�
            nFlags = 4; //0000 0100
            break;
        case 3:         //û�а���
            nFlags = 8; //0000 1000
        }
        if (nFlags != 8) {
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        }
        switch (mouse.nAction) {
        case 0:
            nFlags |= 0x00; //û�а���
            break;
        case 1:             //����
            nFlags |= 0x10; //�� | 0001 0000
            break;
        case 2:             //˫��
            nFlags |= 0x20; //�� | 0010 0000
            break;
        case 3:             //����
            nFlags |= 0x40; //�� | 0100 0000
            break;
        case 4:             //�ſ�
            nFlags |= 0x80; //�� | 1000 0000
            break;
        default:
            break;
        }

        switch (nFlags) {
        case 0x21:  //���˫�� 0010 0001
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11:  //������� 0001 0001
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41:  //������� 0100 0001
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81:  //����ſ� 1000 0001
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22:  //�Ҽ�˫�� 0010 0010
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12:  //�Ҽ����� 0001 0010
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42:  //�Ҽ����� 0100 0010
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82:  //�Ҽ��ſ� 1000 0010
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x23:  //�м�˫�� 0010 0100
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x13:  //�м����� 0001 0100
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x43:  //�м����� 0100 0100
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x83:  //�м��ſ� 1000 0100
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08:  //����ƶ� 0000 1000
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
            //mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());    //������������
            break;
        }
        //CPacket pack(5, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);   //֪�����һ���յ���
        pack_list.push_back(CPacket(5, NULL, 0));

        return 0;
    }

    int SendScreen(std::list<CPacket>& pack_list, CPacket& inPacket) {
        CImage screen;
        HDC hScreen = GetDC(NULL);                              // ��ȡ�豸������
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);   // ��ȡ����λ��
        int nWidth = GetDeviceCaps(hScreen, HORZRES);           // ��ȡˮƽ��ȣ����أ�
        int nHeight = GetDeviceCaps(hScreen, VERTRES);          // ��ȡ��ֱ�߶ȣ����أ�
        screen.Create(nWidth, nHeight, nBitPerPixel);           // ����ʾ����������ͼ��
        //BitBlt ����ִ�����ָ��Դ�豸�����ĵ�Ŀ���豸�������е����ؾ��ζ�Ӧ����ɫ���ݵ�λ�鴫��
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);   //ͼ���Ƶ�screen��
        ReleaseDC(NULL, hScreen);                               // hScreen������񣬰ݰݰ�

        //// ���ͼƷ����� ------- test start-------------------
        //DWORD tick = GetTickCount64();    //����ʹ�á�GetTickCount64�������ǡ�GetTickCount����ԭ�� : GetTickCount overflows roughly every 49 days.Code that does not take that into account can loop indefinitely.GetTickCount64 operates on 64 bit values and does not have that problem	RemoteCtrl
        //screen.Save(_T("test.png"), Gdiplus::ImageFormatPNG);
        //TRACE("png %d\n", GetTickCount64() - tick);
        //tick = GetTickCount64();
        //screen.Save(_T("test.jpg"), Gdiplus::ImageFormatJPEG);
        //TRACE("jpg %d\n", GetTickCount64() - tick);
        //tick = GetTickCount64();
        //screen.Save(_T("test.tiff"), Gdiplus::ImageFormatTIFF);
        //TRACE("tiff %d\n", GetTickCount64() - tick);
        //// ---------------------- test end---------------------

        //��ͼ�洢���ڴ棺F12һ��screen.Save������������IStream�Ǹ����غ�����ͼ��ŵ��ڴ���
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);   //�Ӷѷ���ָ�����ֽ���
        if (hMem == NULL)return -1;
        IStream* pStream = NULL;
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);    // ����һ��������ʹ�� HGLOBAL �ڴ����洢������
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatJPEG);  //��PNG��ʽ���浽��
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);    //��ָ���ƻ�ָ���ͼ���ݿ�ͷ
            PBYTE pData = (PBYTE)GlobalLock(hMem);       //����ȫ���ڴ���󲢷���ָ������ڴ���һ���ֽڵ�ָ��
            SIZE_T nSize = GlobalSize(hMem);
            //CPacket pack(6, pData, nSize);
            //CServerSocket::getInstance()->Send(pack);
            pack_list.push_back(CPacket(6, pData, nSize));
            GlobalUnlock(hMem);
        }
        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();
        return 0;

    }

    int LockMachine(std::list<CPacket>& pack_list, CPacket& inPacket) {

        if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {
            //_beginthread(LockScreenByThread, 0, NULL);
            _beginthreadex(NULL, 0, LockScreenByThread, NULL, 0, &threadId);
            TRACE("threadId = %d\r\n", threadId);
        }
        //CPacket pack(7, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(7, NULL, 0));

        return 0;
    }

    int UnLockMachine(std::list<CPacket>& pack_list, CPacket& inPacket) {
        PostThreadMessage(threadId, WM_KEYDOWN, 0x1b, 0);
        MessageBox(NULL, _T("�ѽ��������"), _T("������ʾ"), MB_ICONASTERISK);
        //CPacket pack(8, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(8, NULL, 0));
        return 0;
    }

    int DeleteLocalFile(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        //CServerSocket::getInstance()->GetFilePath(strPath);
        TCHAR sPath[MAX_PATH] = _T("");
        //mbstowcs(sPath, strPath.c_str(), strPath.size());   //�����ֽ��ַ�����ת��Ϊ��Ӧ�Ŀ��ַ����� waring:_CRT_SECURE_NO_WARNINGS
        //��������??������DeleteFileA()�ɣ���ɸ���??
        MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));  //���ַ���ӳ�䵽 UTF-16 (���ַ�) �ַ���
        SetFileAttributes(sPath, FILE_ATTRIBUTE_NORMAL);    //�趨�ļ�Ϊһ�����ԣ�ȥ��ֻ�����ԣ�
        BOOL bRet = DeleteFile(sPath);
        if (bRet == 0) {
            TRACE("err no = %d\r\n", GetLastError());
            AfxMessageBox(_T("ɾ��ʧ�ܣ�"));
            return -1;
        }
        //CPacket pack(9, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(9, NULL, 0));

        return 0;

    }

    int TestConnect(std::list<CPacket>& pack_list, CPacket& inPacket) {
        //CPacket pack(95, NULL, 0);
        //bool ret = CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(95, NULL, 0));

        return 0;
    }
};

