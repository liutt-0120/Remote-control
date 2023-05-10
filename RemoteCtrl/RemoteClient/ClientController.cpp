#include "pch.h"
#include "ClientController.h"


CClientController* CClientController::m_ctrlInstance = NULL;
std::map<UINT, CClientController::MSGFUNC>CClientController::m_mapFunc;
CClientController::CGarbo CClientController::garbo;

CClientController* CClientController::GetInstance()
{
    if (m_ctrlInstance == NULL) {
        m_ctrlInstance = new CClientController();
        //struct { UINT nMsg; MSGFUNC func; } MsgFuncs[] = {
        //    {WM_SEND_PACK,&CClientController::OnSendPack},
        //    {WM_SEND_DATA,&CClientController::OnSendData},
        //    {WM_SHOW_STATUS,&CClientController::OnShowStatus},
        //    {WM_SHOW_WATCH,&CClientController::OnShowWatcher},
        //    {(UINT)-1,NULL}
        //};
        //for (int i = 0; MsgFuncs[i].func != NULL; i++) {
        //    m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFuncs[i].nMsg, MsgFuncs[i].func));
        //}
    }
    return m_ctrlInstance;
}

int CClientController::InitController()
{
    m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadEntry, this, 0, &m_wThreadId);
    m_statusDlg.Create(IDD_DOWNLOAD_DIALOG, &m_remoteDlg);
    return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
    pMainWnd = &m_remoteDlg;
    return m_remoteDlg.DoModal();
}

////20230506 --- 估计没什么用，毕竟用户操作客户端的时间间隔不会集中到抢占资源
//LRESULT CClientController::SendMessage2Func(MSG msg)
//{
//    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//    if (hEvent == NULL)return -2;
//    MSGINFO msgInfo(msg);
//    PostThreadMessage(m_wThreadId, WM_SEND_MESSAGE, (WPARAM)&msgInfo, (LPARAM)hEvent);
//    WaitForSingleObject(hEvent, -1);
//    return msgInfo.result;
//}

int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength, std::list<CPacket>* pPackLst)
{
    CClientSocket* pClient = CClientSocket::getInstance();
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {
        return -2;
    }
    CPacket pack(nCmd, pData, nLength, hEvent);
    std::list<CPacket> tmp;
    if (pPackLst == NULL) {
        pPackLst = &tmp;
    }
    bool bRet = pClient->SendInPacketList(pack, bAutoClose);
    if (bRet) {
        WaitForSingleObject(hEvent, INFINITE);
        if (pClient->GetRecvPacket(*pPackLst, hEvent)) {
            CloseHandle(hEvent);
            TRACE("ack:%d\r\n", pPackLst->front().sCmd);
            return pPackLst->front().sCmd;
        }
        else {
            CloseHandle(hEvent);
            return -1;
        }
    }
    else {
        CloseHandle(hEvent);
        return -3;
    }


}

int CClientController::GetImage(CImage& image)
{
    CClientSocket* pClient = CClientSocket::getInstance();
    return CMyTool::Byte2Image(image, pClient->GetPacket().strData);
}

int CClientController::DownloadFile(CString strPath)
{
    CFileDialog dlg(FALSE, "*", strPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, &m_remoteDlg);	//CFileDialog类,封装用于文件打开操作或文件保存操作的常见对话框。
    if (dlg.DoModal() == IDOK) {
        m_strRemotePath = strPath;
        m_strLocalPath = dlg.GetPathName();
        m_hThreadDownload = (HANDLE)_beginthread(ThreadEntryForDownloadFile, 0, this);
        if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT) { //线程刚启动，能超时说明启动成功
            return -1;
        }
        m_remoteDlg.BeginWaitCursor();
        //m_statusDlg.m_info.SetWindowText(_T("命令执行中"));	//报错
        m_statusDlg.ShowWindow(SW_SHOW);
        m_statusDlg.CenterWindow(&m_remoteDlg);
        m_statusDlg.SetActiveWindow();
    }
    return 0;
}

void CClientController::StartWatchScreen()
{
    m_isClosed = false;
    m_hThreadWatch = (HANDLE)_beginthread(CClientController::ThreadEntryForWatchScreen, 0, this);
    //GetDlgItem(IDC_BTN_STARTWATCH)->EnableWindow(FALSE);	//防止按钮连点
    m_watchDlg.DoModal();
    m_isClosed = true;		//?设置关闭让线程解除循环，走到调用它的函数去关闭线程。否则关闭窗口后线程不会终止，再点击监控按钮又会开启一条线程，抢占CImage造成bug
    WaitForSingleObject(m_hThreadWatch, 500);
}

CPacket& CClientController::GetPacket()
{
    return CClientSocket::getInstance()->GetPacket();
}

unsigned __stdcall CClientController::ThreadEntry(void* arg)
{
    CClientController* thiz = (CClientController*)arg;
    thiz->ThreadFunc();
    _endthreadex(0);
    return 0;
}

void CClientController::ThreadFunc()
{
    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_SEND_MESSAGE) {
            MSGINFO* pmsg = (MSGINFO*)msg.wParam;
            HANDLE hEvent = (HANDLE)msg.lParam;
            std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
            if (it != m_mapFunc.end()) {
                pmsg->result = (this->*(it->second))(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
                //this->*/MSGFUNC/ func
            }
            else {
                pmsg->result = -1;
            }
            SetEvent(hEvent);
        }
        else {
            std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
            if (it != m_mapFunc.end()) {
                (this->*(it->second))(msg.message, msg.wParam, msg.lParam);
                //this->*/MSGFUNC/ func
            }
        }

    }
}

//LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//    return LRESULT();
//}
//
//LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//    return LRESULT();
//}
//
//LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//    return m_statusDlg.ShowWindow(SW_SHOW);
//}
//
//LRESULT CClientController::OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//    return m_watchDlg.DoModal();
//}

void CClientController::ThreadEntryForDownloadFile(void* args)
{
    CClientController* thiz = (CClientController*)args;
    thiz->ThreadFuncForDownloadFile();
	_endthread();
}

void CClientController::ThreadFuncForDownloadFile()
{
    FILE* pFile;
    fopen_s(&pFile, m_strLocalPath, "wb+");
    if (pFile == NULL) {
        AfxMessageBox("本地没有权限保存该文件，或文件无法创建！");
        m_statusDlg.ShowWindow(SW_HIDE);
        m_remoteDlg.EndWaitCursor();
        return;
    }
    CClientSocket* pClient = CClientSocket::getInstance();
    do {
        int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)m_strRemotePath, m_strRemotePath.GetLength());
        long long nLength = *(long long*)pClient->GetPacket().strData.c_str();	//先接收文件的总长
        if (nLength == 0) {
            AfxMessageBox("文件长度为0，无法读取");
            break;
        }
        long long nCount = 0;
        while (nCount < nLength) {	//1k 1k地接收文件
            ret = pClient->DealCommand();
            if (ret < 0) {
                AfxMessageBox("传输失败！");
                TRACE("传输失败 ret:%d\r\n", ret);
                break;
            }
            fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
            nCount += pClient->GetPacket().strData.size();
            double progressData = (double)nCount * 100 / nLength;
            CString str;
            str.Format(_T("%.2lf"), progressData);
            m_statusDlg.m_info2.SetWindowText(_T("已下载：") + str);
            //m_statusDlg.UpdateData(false);
        }
        m_remoteDlg.MessageBox(_T("下载成功！"));
    } while (false);
    fclose(pFile);
    pClient->CloseSocket();
    m_statusDlg.ShowWindow(SW_HIDE);
    m_remoteDlg.EndWaitCursor();

}

void CClientController::ThreadEntryForWatchScreen(void* args)
{
    CClientController* thiz = (CClientController*)args;
    thiz->ThreadFuncForWatchScreen();
    _endthread();
}

void CClientController::ThreadFuncForWatchScreen()
{
    Sleep(50);
    while (!m_isClosed) {	//防止线程在里面黑转不出来
        if (m_watchDlg.IsFull() == false) {	//若为false，更新数据到缓存
            std::list<CPacket> packList;
            int ret = SendCommandPacket(6, true, NULL, 0, &packList);
            if (ret == 6) {
                if (CMyTool::Byte2Image(m_watchDlg.GetImage(), packList.front().strData) == 0) {
                    m_watchDlg.SetImageStatus(true);
                    TRACE("获取图片！ ret = %d\r\n", ret);
                }
                else {
                    TRACE("获取图片失败！ ret = %d\r\n", ret);
                }
            }
            else {
                Sleep(1);
            }
        }
        else {
            Sleep(1);
        }

    }
}
