#pragma once
#include <MSWSock.h>
#include "MyThread.h"
#include <map>
#include <vector>
#include "MyQueue.h"

enum EOperator{
    ENone,
    EAccept,
    ERecv,
    ESend,
    EError
};

class CMyServer;
class CClient;
typedef std::shared_ptr<CClient> PCLIENT;

class COverlapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;              //���� ����μ�EOperator
    std::vector<char> m_buffer;    //��ֵ������
    CThreadWorker m_worker;        //������
    CMyServer* m_server;           //����������
    CClient* m_client;              //��Ӧ�Ŀͻ���
    WSABUF m_wsabuffer;            //WSABUF �ṹ�����������ĳЩ Winsock ����ʹ�õ����ݻ�����
    virtual ~COverlapped() {
        m_buffer.clear();
    }
};

template<EOperator> class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

template<EOperator> class RecvOverlapped;
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;

template<EOperator> class SendOverlapped;
typedef SendOverlapped<ESend> SENDOVERLAPPED;

template<EOperator> class ErrorOverlapped;
typedef ErrorOverlapped<EError> ERROROVERLAPPED;

//�ͻ��˲���
class CClient:public CThreadFuncBase {
public:
    CClient();
    ~CClient() {
        closesocket(m_sockCli);
        m_recv.reset();
        m_send.reset();
        m_accept.reset();
        m_buffer.clear();
        m_vecSend.Clear();
    }

    void SetOverlapped(PCLIENT& ptr);
    //����ת������
    //operator SOCKET() {
    //    return m_sockCli;
    //}

    //operator PVOID() {
    //    return &m_buffer[0];
    //}
    //operator LPOVERLAPPED(){
    //    return &m_accept->m_overlapped;
    //}
    //operator LPDWORD() {
    //    return &m_received;
    //}

    SOCKET GetClientSocket() {
        return m_sockCli;
    }
    PVOID GetPBuffer() {
        return &m_buffer[0];
    }
    LPOVERLAPPED GetPOverlapped();
    LPDWORD GetPReceive() {
        return &m_received;
    }

    LPWSABUF RecvWSABuffer();
    LPWSABUF SendWSABuffer();
    DWORD& flags() {
        return m_flags;
    }
    sockaddr_in* GetLocalAddr() {
        return &m_laddr;
    }
    sockaddr_in* GetRemoteAddr() {
        return &m_raddr;
    }
    size_t GetBufferSize() const {
        return m_buffer.size();
    }
    int Recv();
    int Send(void* buffer,size_t size);
    int SendData(std::vector<char>& data);
private:
    SOCKET m_sockCli;
    DWORD m_received;
    DWORD m_flags;
    std::shared_ptr<ACCEPTOVERLAPPED> m_accept;
    std::shared_ptr<RECVOVERLAPPED> m_recv;
    std::shared_ptr<SENDOVERLAPPED> m_send;

    std::vector<char> m_buffer;
    size_t m_used;  //�Ѿ�ʹ�õĻ�������С
    sockaddr_in m_laddr;
    sockaddr_in m_raddr;
    bool m_isBusy;
    CSendQueue<std::vector<char>>m_vecSend;   //�������ݵĶ���
};

template<EOperator>
class AcceptOverlapped :public COverlapped, public CThreadFuncBase {
public:
    AcceptOverlapped();
    int AcceptWorker();
};

template<EOperator>
class RecvOverlapped :public COverlapped, public CThreadFuncBase {
public:
    RecvOverlapped();

    int RecvWorker() {
        int ret = m_client->Recv();
        return ret;
    }
};

template<EOperator>
class SendOverlapped :public COverlapped, public CThreadFuncBase {
public:
    SendOverlapped();

    int SendWorker() {
        //TODO:
        return -1;
    }
};

template<EOperator>
class ErrorOverlapped :public COverlapped, public CThreadFuncBase {
public:
    ErrorOverlapped()
        :m_operator(EError),
        m_worker(this, &ErrorOverlapped::ErrorWorker) {   //��鲻��template���ܸ����������ʼ��,ԭ��δ֪
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024);
    }
    int ErrorWorker() {
        //TODO:
        return -1;
    }
};

//����˲���
class CMyServer :
    public CThreadFuncBase
{
public:
    CMyServer(const std::string& ip = "0.0.0.0",short port = 9527) :m_pool(10) {
        m_hIOCP = INVALID_HANDLE_VALUE;
        m_sockSrv = INVALID_SOCKET;
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        m_addr.sin_port = htons(port);
    }

    ~CMyServer();

    //
    bool StartService();

    bool NewAccept() {
        PCLIENT pClient(new CClient());
        pClient->SetOverlapped(pClient);
        m_client.insert(std::pair<SOCKET, PCLIENT>(pClient->GetClientSocket(), pClient));
        if (!AcceptEx(m_sockSrv, pClient->GetClientSocket(),pClient->GetPBuffer(), 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, pClient->GetPReceive(), pClient->GetPOverlapped())) {
            closesocket(m_sockSrv);
            m_sockSrv = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
    }


private:
    //
    void CreateSocket() {
        m_sockSrv = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        int opt = 1;
        setsockopt(m_sockSrv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    }
    int threadIocp();
private:
    CThreadPool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_sockSrv;
    sockaddr_in m_addr;
    std::map<SOCKET, std::shared_ptr<CClient>>m_client;
};

