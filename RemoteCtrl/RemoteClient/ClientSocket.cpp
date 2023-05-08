#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_clientInstance = NULL;


CClientSocket::CGarbo CClientSocket::garbo;		//临了临了，帮忙把静态指针delete了

bool CClientSocket::Send(const char* pData, int nSize)
{
	if (m_sockSrv == -1)return false;
	return send(m_sockSrv, pData, nSize, 0) > 0;
}

bool CClientSocket::Send(const CPacket& pack)
{
	if (m_sockSrv == -1)return false;
	std::string strOut;
	//TRACE("nLength:%d，cmd:%d，data:%s\r\n", pack.nLength, pack.sCmd, pack.strData.c_str());

	pack.Data(strOut);

	return send(m_sockSrv, strOut.c_str(), strOut.size(), 0) > 0;
}

void CClientSocket::ThreadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->ThreadFunc();
	_endthread();
}

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
			auto tmp = m_lstSend.front();
			CPacket head = tmp;
			if (Send(tmp) == false) {				//将队列中要发送的发出去
				TRACE(_T("发送失败！\r\n"));
				InitSocket();
				continue;
			}
			auto pr = m_mapPack.insert(std::pair<HANDLE, std::list<CPacket>>(tmp.hEvent, std::list<CPacket>()));
			int len = recv(m_sockSrv, pBuffer + index, MAX_SIZE - index, 0);
			if (len > 0 || index > 0) {
				index += len;
				len = index;
				size_t size = (size_t)len;
				TRACE("len:%d,size:%d\r\n", len, size);
				CPacket pack((BYTE*)pBuffer, size);

				if (size > 0) {
					//TODO：通知对应事件
					pack.hEvent = tmp.hEvent;
					pr.first->second.push_back(pack);	
					SetEvent(pack.hEvent);
					memmove(pBuffer, pBuffer + size, index - size);
					index -= size;
				}
			}
			else if (len <= 0 && index <= 0) {
				TRACE(_T("好像没接收到啥\r\n"));
				continue;
			}
			m_lstSend.pop_front();
		}
	}
}

bool CClientSocket::SendInPacketList(CPacket& pack)
{
	TRACE("pack的length:%d\r\n", pack.nLength);
	if (m_sockSrv == INVALID_SOCKET) {
		if (InitSocket() == false) return false;
		_beginthread(CClientSocket::ThreadEntry, 0, this);
	}
	TRACE("往里塞,length = %d\r\n",pack.nLength);
	m_lstSend.push_back(pack);
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
