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
	/// 命令处理入口函数
	/// </summary>
	/// <param name="nCmd"></param>
	/// <param name="pack_list"></param>
	/// <param name="inPacket"></param>
	/// <returns></returns>
	int ExcuteCommand(int nCmd, std::list<CPacket>& pack_list, CPacket& inPacket);

    /// <summary>
    /// 回调函数
    /// </summary>
    /// <param name="arg"></param>
    /// <param name="status"></param>
    /// <param name="pack_list"></param>
    /// <param name="inPacket"></param>
    static void RunCommand(void* arg, int status, std::list<CPacket>& pack_list, CPacket& inPacket) {
        CCommand* thiz = (CCommand*)arg;
        int nRet = thiz->ExcuteCommand(status, pack_list, inPacket);
        if(nRet != 0)
            TRACE("执行命令失败：%d ret = %d\r\n", status, nRet);


    }
protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket&);	//成员函数指针
	std::map<int, CMDFUNC> m_mapFunc;	//命令映射功能
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
        //覆盖全屏开始~~~??
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);     //获取当前设备满屏的最大横坐标
        rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) * 1.03;    //获取当前设备满屏的最大纵坐标
        dlg.MoveWindow(rect);
        //设置锁屏字幕位置
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
        if (pText) {
            CRect rText;
            pText->GetWindowRect(rText);
            int x = (rect.right - rText.Width()) / 2;
            int y = (rect.bottom - rText.Height()) / 2;
            pText->MoveWindow(x, y, rText.Width(), rText.Height());
        }
        //设置窗口置顶??
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); //AppMsg - Warning: calling DestroyWindow in CDialog::~CDialog --
                                                                                //AppMsg - OnDestroy or PostNcDestroy in derived class will not be called
                                                                                //因为非模态对话框不存在阻塞，方法一出就歇逼了，这就提示我们没有destroy
        ShowCursor(false);      //限制鼠标功能
        ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);   //隐藏任务栏
        //dlg.GetWindowRect(rect); 
        ClipCursor(rect);       //限制鼠标活动范围
                                //双管齐下，把鼠标限制到dlg内，再限制功能                                                        

        MSG msg;                //dialog一闪而过：mfc是基于消息循环的，本项目里并没有消息循环，因此要自定义一个消息循环以满足需求
        while (GetMessage(&msg, dlg.m_hWnd, 0, 0)) {
            TranslateMessage(&msg);
            LRESULT ret = DispatchMessage(&msg);
            //测试键入结束锁机
            if (msg.message == WM_KEYDOWN) {
                TRACE("msg:%08X wParam:%08x lParam:%08x\r\n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == VK_SPACE) {
                    TRACE(_T("触发空格"));
                }

                if (msg.wParam == VK_ESCAPE)  //ESC
                    break;
            }
        }
        ClipCursor(NULL);   //恢复鼠标活动范围
        ShowCursor(true);   //恢复鼠标功能
        ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);   //显示任务栏
        dlg.DestroyWindow();

    }

    int MakeDriverInfo(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string result;
        for (int i = 1; i <= 26; ++i) {
            if (_chdrive(i) == 0) {
                if (result.size() > 0)
                    result += ',';      //tip 骚
                result += 'A' + i - 1;
            }
        }
        //CPacket pack(1, (BYTE*)result.c_str(), result.size());  //打包
        ////Dump((BYTE*)&pack, pack.nLength + 6);     //直接使用封装的包，其中的string存储的是数据的首地址，因此传出的包数据有误
        ////现使用Data()整理包数据
        //CMyTool::Dump((BYTE*)pack.Data(), pack.Size());
        //CServerSocket::getInstance()->Send(pack);
        
        pack_list.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }

    int MakeDirectoryInfo(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        //std::list<FileInfo> lstFileInfos;     //这块本来计划用list拿了所有文件后统一发送，但鉴于有的文件夹可能包含大量文件，如日志，网页的临时文件等，那处理完所有文件将耗费大量时间，甲方很可能以为卡了忍不住疯狂点击。

        //if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        //    OutputDebugString(_T("当前命令有误，不是获取文件列表"));
        //    return -1;
        //}
        if (_chdir(strPath.c_str()) != 0) {
            FileInfo fInfo;
            fInfo.hasNext = false;
            //lstFileInfos.push_back(fInfo);
            
            //CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
            //CServerSocket::getInstance()->Send(pack);
            pack_list.push_back(CPacket(2, (BYTE*)&fInfo, sizeof(fInfo)));
            OutputDebugString(_T("没有访问目录的权限"));
            return -2;
        }
        _finddata_t fdata;
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("没有找到任何文件"));
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
            //CServerSocket::getInstance()->Send(pack);       //现计划处理一个就发一个
            pack_list.push_back(CPacket(2, (BYTE*)&fInfo, sizeof(fInfo)));
        } while (!_findnext(hfind, &fdata));

        FileInfo fInfo;         //这感觉可以一个函数都用这一个FileInfo啊？?
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
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb"); //在客户端那边叫下载，在服务端这块叫（本地）上传、发送，因此是读文件。不知道客户端要下载哪类文件，按二进制方式读。
        if (err != 0) {
            //CPacket pack(4, (BYTE*)&data, 8);
            //CServerSocket::getInstance()->Send(pack);
            pack_list.push_back(CPacket(4, (BYTE*)&data, 8));
            return -1;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);  //SEEK_END文件结尾
            data = _ftelli64(pFile);
            //CPacket fLenPack(4, (BYTE*)&data, 8);   //先把获取到的文件大小发过去
            //CServerSocket::getInstance()->Send(fLenPack);
            pack_list.push_back(CPacket(4, (BYTE*)&data, 8));
            fseek(pFile, 0, SEEK_SET);  //SEEK_SET文件开头

            char buffer[1024] = ""; //不超过1K，避免被截断
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                //CPacket pack(4, (BYTE*)buffer, rlen);
                //CServerSocket::getInstance()->Send(pack);   //读1K发一次
                pack_list.push_back(CPacket(4, (BYTE*)buffer, rlen));
            } while (rlen >= 1024);
            fclose(pFile);      //有开有关
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
        case 0:         //左键
            nFlags = 1; //0000 0001
            break;
        case 1:         //右键
            nFlags = 2; //0000 0010
            break;
        case 2:         //中键
            nFlags = 4; //0000 0100
            break;
        case 3:         //没有按键
            nFlags = 8; //0000 1000
        }
        if (nFlags != 8) {
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        }
        switch (mouse.nAction) {
        case 0:
            nFlags |= 0x00; //没有按键
            break;
        case 1:             //单击
            nFlags |= 0x10; //？ | 0001 0000
            break;
        case 2:             //双击
            nFlags |= 0x20; //？ | 0010 0000
            break;
        case 3:             //按下
            nFlags |= 0x40; //？ | 0100 0000
            break;
        case 4:             //放开
            nFlags |= 0x80; //？ | 1000 0000
            break;
        default:
            break;
        }

        switch (nFlags) {
        case 0x21:  //左键双击 0010 0001
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11:  //左键单击 0001 0001
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41:  //左键按下 0100 0001
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81:  //左键放开 1000 0001
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22:  //右键双击 0010 0010
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12:  //右键单击 0001 0010
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42:  //右键按下 0100 0010
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82:  //右键放开 1000 0010
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x23:  //中键双击 0010 0100
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x13:  //中键单击 0001 0100
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x43:  //中键按下 0100 0100
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x83:  //中键放开 1000 0100
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08:  //鼠标移动 0000 1000
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
            //mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());    //这玩意儿有麻达
            break;
        }
        //CPacket pack(5, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);   //知会对面一声收到了
        pack_list.push_back(CPacket(5, NULL, 0));

        return 0;
    }

    int SendScreen(std::list<CPacket>& pack_list, CPacket& inPacket) {
        CImage screen;
        HDC hScreen = GetDC(NULL);                              // 获取设备上下文
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);   // 获取像素位宽
        int nWidth = GetDeviceCaps(hScreen, HORZRES);           // 获取水平宽度（像素）
        int nHeight = GetDeviceCaps(hScreen, VERTRES);          // 获取垂直高度（像素）
        screen.Create(nWidth, nHeight, nBitPerPixel);           // 按显示器参数创建图像
        //BitBlt 函数执行与从指定源设备上下文到目标设备上下文中的像素矩形对应的颜色数据的位块传输
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);   //图像复制到screen中
        ReleaseDC(NULL, hScreen);                               // hScreen完成任务，拜拜吧

        //// 瞅瞅图品质如何 ------- test start-------------------
        //DWORD tick = GetTickCount64();    //考虑使用“GetTickCount64”而不是“GetTickCount”。原因 : GetTickCount overflows roughly every 49 days.Code that does not take that into account can loop indefinitely.GetTickCount64 operates on 64 bit values and does not have that problem	RemoteCtrl
        //screen.Save(_T("test.png"), Gdiplus::ImageFormatPNG);
        //TRACE("png %d\n", GetTickCount64() - tick);
        //tick = GetTickCount64();
        //screen.Save(_T("test.jpg"), Gdiplus::ImageFormatJPEG);
        //TRACE("jpg %d\n", GetTickCount64() - tick);
        //tick = GetTickCount64();
        //screen.Save(_T("test.tiff"), Gdiplus::ImageFormatTIFF);
        //TRACE("tiff %d\n", GetTickCount64() - tick);
        //// ---------------------- test end---------------------

        //截图存储在内存：F12一下screen.Save看看，可以用IStream那个重载函数将图存放到内存里
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);   //从堆分配指定的字节数
        if (hMem == NULL)return -1;
        IStream* pStream = NULL;
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);    // 创建一个流对象，使用 HGLOBAL 内存句柄存储流内容
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatJPEG);  //以PNG形式保存到流
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);    //将指针移回指向截图数据开头
            PBYTE pData = (PBYTE)GlobalLock(hMem);       //锁定全局内存对象并返回指向对象内存块第一个字节的指针
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
        MessageBox(NULL, _T("已解除锁机！"), _T("解锁提示"), MB_ICONASTERISK);
        //CPacket pack(8, NULL, 0);
        //CServerSocket::getInstance()->Send(pack);
        pack_list.push_back(CPacket(8, NULL, 0));
        return 0;
    }

    int DeleteLocalFile(std::list<CPacket>& pack_list, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        //CServerSocket::getInstance()->GetFilePath(strPath);
        TCHAR sPath[MAX_PATH] = _T("");
        //mbstowcs(sPath, strPath.c_str(), strPath.size());   //将多字节字符序列转换为对应的宽字符序列 waring:_CRT_SECURE_NO_WARNINGS
        //中文乱码??，改用DeleteFileA()可，亦可改用??
        MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));  //将字符串映射到 UTF-16 (宽字符) 字符串
        SetFileAttributes(sPath, FILE_ATTRIBUTE_NORMAL);    //设定文件为一般属性（去掉只读属性）
        BOOL bRet = DeleteFile(sPath);
        if (bRet == 0) {
            TRACE("err no = %d\r\n", GetLastError());
            AfxMessageBox(_T("删除失败！"));
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

