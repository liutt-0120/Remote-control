#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_clientInstance = NULL;


CClientSocket::CGarbo CClientSocket::garbo;		//�������ˣ���æ�Ѿ�ָ̬��delete��

CClientSocket::CClientSocket() :m_nIP(INADDR_ANY), m_nPort(0),m_sockSrv(INVALID_SOCKET), m_hThread(INVALID_HANDLE_VALUE){
	if (InitSockEnv() == FALSE) {
		MessageBox(NULL, _T("��ʼ���׽��ֻ���ʧ�ܣ�������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_buffer.resize(MAX_SIZE);	//��ʼ��sizeΪ4096
	struct {
		UINT message;
		MSGFUNC func;
	}funcs[] = {
		{WM_SEND_PACK,&CClientSocket::SendPack},
		{0,NULL}
	};
	for (int i = 0; i < funcs[i].message != 0; i++) {
		if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
			TRACE("����ʧ�ܣ���Ϣֵ��%d ����ֵ��%08X ��ţ�%d\r\n", funcs[i].message, funcs[i].func, i);
		}
	}
}

CClientSocket::CClientSocket(const CClientSocket& cs)
{
	m_sockSrv = cs.m_sockSrv;
	m_nIP = cs.m_nIP;
	m_nPort = cs.m_nPort;
	m_hThread = cs.m_hThread;
	std::map<UINT, CClientSocket::MSGFUNC>::const_iterator it = cs.m_mapFunc.begin();
	for (; it != cs.m_mapFunc.end(); it++) {
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
	}
}

CClientSocket& CClientSocket::operator=(const CClientSocket& cs)
{
	if (this != &cs) {
		m_sockSrv = cs.m_sockSrv;
		m_nIP = cs.m_nIP;
		m_nPort = cs.m_nPort;
		m_hThread = cs.m_hThread;
		std::map<UINT, CClientSocket::MSGFUNC>::const_iterator it = cs.m_mapFunc.begin();
		for (; it != cs.m_mapFunc.end(); it++) {
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
		}
	}
	return *this;
}

bool CClientSocket::Send(const char* pData, int nSize)
{
	if (m_sockSrv == -1)return false;
	return send(m_sockSrv, pData, nSize, 0) > 0;
}

bool CClientSocket::Send(const CPacket& pack)
{
	if (m_sockSrv == -1)return false;
	std::string strOut;
	//TRACE("nLength:%d��cmd:%d��data:%s\r\n", pack.nLength, pack.sCmd, pack.strData.c_str());

	pack.Data(strOut);

	return send(m_sockSrv, strOut.c_str(), strOut.size(), 0) > 0;
}

/*
void CClientSocket::ThreadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->ThreadFunc();
	_endthread();
}

/// <summary>
/// �̷߳�������Ҫ���͵�list<CPacket>��ȡ���ݣ��������send��
/// </summary>
void CClientSocket::ThreadFunc()
{
	Sleep(100);
	std::string strBuffer;
	strBuffer.resize(MAX_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	while (m_sockSrv != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			TRACE("m_lstSend.size:%d\r\n",m_lstSend.size());
			m_lock.lock();
			CPacket& head = m_lstSend.front();
			m_lock.unlock();
			if (Send(head) == false) {				//��������Ҫ���͵ķ���ȥ
				TRACE(_T("����ʧ�ܣ�\r\n"));
				continue;
			}
			auto pr = m_mapPack.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));
			auto pAutoClosed = m_mapAutoClosed.find(head.hEvent);
			do {
				int len = recv(m_sockSrv, pBuffer + index, MAX_SIZE - index, 0);
				if (len > 0 || index > 0) {
					index += len;
					len = index;
					size_t size = (size_t)len;
					TRACE("len:%d,size:%d\r\n", len, size);
					CPacket pack((BYTE*)pBuffer, size);

					if (size > 0) {
						//TODO��֪ͨ��Ӧ�¼�
						pack.hEvent = head.hEvent;
						pr.first->second.push_back(pack);
						memmove(pBuffer, pBuffer + size, index - size);
						index -= size;
					}
				}
				else if (len <= 0 && index <= 0) {
					TRACE(_T("û���յ�ɶ��\r\n"));
					CloseSocket();
					break;
				}
			} while (pAutoClosed->second==false);
			SetEvent(head.hEvent);
			m_mapAutoClosed.erase(head.hEvent);
			m_lock.lock();
			m_lstSend.pop_front();
			m_lock.unlock();
			InitSocket();

		}
	}
}
*/
unsigned CClientSocket::ThreadEntry_Remake(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->ThreadFunc_Remake();
	_endthreadex(0);
	return 0;
}

void CClientSocket::ThreadFunc_Remake()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	//TODO������һ����Ϣ�����ݽṹ�����ݡ����ݳ��ȡ�ģʽ����һ���ص���Ϣ�����ݽṹ��HWND MESSAGE��
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;	//���һЩΪ��Ҫ��ôд����������ߴ�ֵ�ı����Ǹ��ֲ��������������п������Ұָ�룻��˶Է������new�����ݴ�ֵ���˴�ȷ�����Ƶ��ֺ��ͷŶ���ռ�õĶ��ڴ�
	HWND hWnd = (HWND)lParam;
	if (InitSocket() == true) {
		int ret = send(m_sockSrv, (char*)data.strData.c_str(), (int)data.strData.size(), 0);
		if (ret > 0) {
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(MAX_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_sockSrv != INVALID_SOCKET) {
				int length = recv(m_sockSrv, pBuffer + index, MAX_SIZE - index, 0);
				if (length > 0||index > 0) {
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0) {
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), NULL);	//look���������˸�new�����Ա����õķ����Ǳ�Ҫ�ǵ�delete
						if (data.nMode & CSM_AUTOCLOSE) {
							CloseSocket();
							return;
						}
					}
					index -= nLen;
					memmove(pBuffer, pBuffer + index, nLen);
				}
				else {
					//�׽��ֱ��Է��رա������豸�쳣����������
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, 1);

				}
			}
		}
		else {
			CloseSocket();
			//���ս���쳣
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else {
		//TODO��������
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, NULL);
	}
}

bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool bAutoClose) {
	if (m_hThread == INVALID_HANDLE_VALUE) {
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, CClientSocket::ThreadEntry_Remake, this, 0, &m_wThreadId);
	}
	UINT nMode = bAutoClose ? CSM_AUTOCLOSE : 0;
	std::string strOut;
	pack.Data(strOut);
	return PostThreadMessage(m_wThreadId, WM_SEND_PACK, (WPARAM)new PACKET_DATA(strOut.c_str(), strOut.size(), nMode), (LPARAM)hWnd);
}

/*
bool CClientSocket::SendInPacketList(CPacket& pack,bool bAutoClose)
{
	if (m_sockSrv == INVALID_SOCKET&&m_hThread == INVALID_HANDLE_VALUE) {
		if (InitSocket() == false) return false;
		m_hThread = (HANDLE)_beginthread(CClientSocket::ThreadEntry_Remake, 0, this);
	}

	m_mapAutoClosed.insert({ pack.hEvent, bAutoClose });
	m_lock.lock();
	m_lstSend.push_back(pack);
	m_lock.unlock();
	//m_mapPack.insert(std::pair<HANDLE, std::list<CPacket>>(pack.hEvent, std::list<CPacket>()));
	return true;
}


bool CClientSocket::GetRecvPacket(std::list<CPacket>& packlst, HANDLE& hEvent)
{
	auto pr = m_mapPack.find(hEvent);
	if (pr != m_mapPack.end()) {
		std::list<CPacket>::iterator it;
		for (it = pr->second.begin(); it != pr->second.end(); it++) {
			packlst.push_back(*it);
		}
		m_mapPack.erase(hEvent);
		return true;
	}
	else{
		return false;
	}
}
*/
