#include "pch.h"
#include "MyServer.h"
#include "MyTool.h"
#pragma warning(disable:4407)
CClient::CClient() 
    :m_isBusy(false), m_flags(0), 
    m_accept(new ACCEPTOVERLAPPED()),
    m_recv(new RECVOVERLAPPED()),
    m_send(new SENDOVERLAPPED()),
    m_vecSend(this,(SENDCALLBACK)&CClient::SendData)
{
    m_sockCli = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    m_buffer.resize(1024);
    memset(&m_laddr, 0, sizeof(m_laddr));
    memset(&m_raddr, 0, sizeof(m_raddr));

}
void CClient::SetOverlapped(PCLIENT& ptr) {
    m_accept->m_client = ptr.get();
    m_recv->m_client = ptr.get();
    m_send->m_client = ptr.get();
}


LPOVERLAPPED CClient::GetPOverlapped()
{
    return &(m_accept->m_overlapped);
}

LPWSABUF CClient::RecvWSABuffer()
{
    return &m_recv->m_wsabuffer;
}

LPWSABUF CClient::SendWSABuffer()
{
    return &m_send->m_wsabuffer;
}

int CClient::Recv()
{
    int ret = recv(m_sockCli, m_buffer.data() + m_used, m_buffer.size() - m_used, 0);
    if (ret <= 0)return -1;
    m_used += (size_t)ret;
    //TODO: 解析数据
    return 0;
}

int CClient::Send(void* buffer, size_t size)
{
    std::vector<char>data(size);
    memcpy(data.data(), buffer, size);
    if (m_vecSend.PushBack(data)) {
        return 0;
    }
    return -1;
}

int CClient::SendData(std::vector<char>& data)
{
    if (m_vecSend.Size() > 0) {
        int ret = WSASend(m_sockCli, SendWSABuffer(), 1, &m_received, m_flags, &m_send->m_overlapped, NULL);
        if (ret != 0 && WSAGetLastError() != WSA_IO_PENDING) {
            CMyTool::ShowError();
            return -1;
        }
    }
    return 0;
}

LPWSAOVERLAPPED CClient::GetRecvOverlappedPtr()
{
    return &m_recv->m_overlapped;
}

LPWSAOVERLAPPED CClient::GetSendOverlappedPtr()
{
    return &m_send->m_overlapped;
}



template<EOperator op>
int AcceptOverlapped<op>::AcceptWorker() {
    INT lLength = 0, rLength = 0;
    if (m_client->GetBufferSize() > 0) {
        sockaddr* pLocal = NULL,* pRemote = NULL;
        GetAcceptExSockaddrs(       //GetAcceptExSockaddrs 函数分析从对 AcceptEx 函数的调用中获取的数据，并将本地和远程地址传递给 sockaddr 结构
            m_client->GetPBuffer(), 0,      
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
            (sockaddr**)&pLocal, &lLength, //本地地址
            (sockaddr**)&pRemote, &rLength //远程地址
        );
        memcpy(m_client->GetLocalAddr(), pLocal, sizeof(sockaddr_in));
        memcpy(m_client->GetRemoteAddr(), pRemote, sizeof(sockaddr_in));
        m_server->BindNewSocket(m_client->GetClientSocket());
        int ret = WSARecv(m_client->GetClientSocket(), m_client->RecvWSABuffer(),1, m_client->GetPReceive(), &m_client->flags(), m_client->GetRecvOverlappedPtr(), NULL);
        if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            //TODO: 报错
        }
        if (!m_server->NewAccept()) {
            return -2;
        }
    }
    return -1;
}

template<EOperator op>
AcceptOverlapped<op>::AcceptOverlapped() {
    m_worker = CThreadWorker(this, (FUNCTYPE)&AcceptOverlapped<op>::AcceptWorker);
    m_operator = EAccept;
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024);
    m_server = NULL;
}

template<EOperator op>
RecvOverlapped<op>::RecvOverlapped() {
    m_worker = CThreadWorker(this, (FUNCTYPE)&RecvOverlapped<op>::RecvWorker);
    m_operator = ERecv;
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
}

template<EOperator op>
SendOverlapped<op>::SendOverlapped() {
    m_worker = CThreadWorker(this, (FUNCTYPE)&SendOverlapped<op>::SendWorker);
    m_operator = ESend;
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
    m_server = NULL;
}

CMyServer::~CMyServer()
{
    closesocket(m_sockSrv);
    std::map<SOCKET, PCLIENT>::iterator it = m_client.begin();
    for (; it != m_client.end(); it++) {
        it->second.reset();
    }
    m_client.clear();
    CloseHandle(m_hIOCP);
    //m_pool.Stop();
    WSACleanup();
}

bool CMyServer::StartService()
{
    CreateSocket();

    if (bind(m_sockSrv, (sockaddr*)&m_addr, sizeof(sockaddr)) == -1) {
        closesocket(m_sockSrv);
        m_sockSrv = INVALID_SOCKET;
        TRACE("%d\r\n", WSAGetLastError());
        return false;
    }

    if (listen(m_sockSrv, 3) == -1) {
        closesocket(m_sockSrv);
        m_sockSrv = INVALID_SOCKET;
        return false;
    }

    //创建完成端口
    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
    if (m_hIOCP == NULL) {
        closesocket(m_sockSrv);
        m_sockSrv = INVALID_SOCKET;
        m_hIOCP = INVALID_HANDLE_VALUE;
        return false;
    }
    //绑定到服务器socket
    CreateIoCompletionPort((HANDLE)m_sockSrv, m_hIOCP, (ULONG_PTR)this, 0);
    //开启线程池线程
    m_pool.Invoke();
    //分派GetQueuedCompletionStatus
    m_pool.DispatchWorker(CThreadWorker(this, (FUNCTYPE)&CMyServer::threadIocp));   //分配线程
    if (!NewAccept()) return false;
    return true;
}

bool CMyServer::NewAccept()
{
    PCLIENT pClient(new CClient());
    pClient->SetOverlapped(pClient);
    m_client.insert(std::pair<SOCKET, PCLIENT>(pClient->GetClientSocket(), pClient));
    if (!AcceptEx(m_sockSrv, pClient->GetClientSocket(), pClient->GetPBuffer(), 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, pClient->GetPReceive(), pClient->GetPOverlapped())) {
        TRACE("%d\r\n", WSAGetLastError());
        if (WSAGetLastError() != WSA_IO_PENDING) {
            closesocket(m_sockSrv);
            m_sockSrv = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }

    }
    return true;
}

void CMyServer::BindNewSocket(SOCKET s)
{
    CreateIoCompletionPort((HANDLE)s, m_hIOCP, (ULONG_PTR)this, 0);
}

int CMyServer::threadIocp()
{
    DWORD transferred = 0;
    ULONG_PTR completionKey = 0;
    OVERLAPPED* lpOverlapped = NULL;
    if (GetQueuedCompletionStatus(m_hIOCP, &transferred, &completionKey, &lpOverlapped, INFINITE)) {
        if (completionKey != 0) {
            COverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, COverlapped, m_overlapped);
            pOverlapped->m_server = this;
            switch (pOverlapped->m_operator)
            {
            case EAccept:
            {
                ACCEPTOVERLAPPED* pAccept = (ACCEPTOVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pAccept->m_worker);
            }
            break;
            case ERecv:
            {
                RECVOVERLAPPED* pRecv = (RECVOVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pRecv->m_worker);
            }
            break;
            case ESend:
            {
                SENDOVERLAPPED* pSend = (SENDOVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pSend->m_worker);
            }
            break;
            case EError:
            {
                ERROROVERLAPPED* pError = (ERROROVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pError->m_worker);
            }
            break;
            default:
                break;
            }
        }
    }
    return 0;
}
