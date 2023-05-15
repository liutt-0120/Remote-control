#pragma once
#include "pch.h"
#include <list>
#include <atomic>

template<class T>
class CMyQueue
{//利用IOCP实现线程安全的队列
public:
	typedef struct IocpParam {
		size_t nOperator; //操作
		T tData;  //数据
		HANDLE hEvent;	//pop操作需要
		IocpParam(int op, const char* data, HANDLE hEve = NULL) {
			nOperator = op;
			tData = data;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = MQNone;
		}
	}PPARAM;	//Post Parameter  用于投递信息的结构体

	enum {
		MQNone,
		MQPush,
		MQPop,
		MQSize,
		MQClear
	};
public:
	CMyQueue() {
		m_lock = false;
		m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompletionPort != NULL) {
			m_hThread = (HANDLE)_beginthread(CMyQueue<T>::ThreadEntry, 0, m_hCompletionPort);
		}
	}
	~CMyQueue() {
		if (m_lock) return;		//防止析构被反复调用
		m_lock = true;
		HANDLE hTemp = m_hCompletionPort;
		PostQueuedCompletionStatus(m_hCompletionPort, 0, NULL, NULL);
		WaitForSingleObject(m_hThread, INFINITE);
		m_hCompletionPort = NULL;
		CloseHandle(hTemp);	//重复close也没问题
	}
	bool PushBack(const T& data) {
		if (m_lock == true) return false;
		IocpParam* pParam = new IocpParam(MQPush, data);
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false) delete pParam;
		return ret;
	}
	bool PopFront(T& data) {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam param(MQPop, data, hEvent);
		if (m_lock) {
			if (hEvent) CloseHandle(hEvent);
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)&param, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return false;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;	//push里不搞等待是因为大量写入数据的高效率要求要避免阻塞.
		if (ret) {
			data = param.tData;
		}
		return ret;
	}
	size_t Size() {	//搞线程安全的size
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam param(MQSize, T(), hEvent);
		if (m_lock) {
			if (hEvent) CloseHandle(hEvent);
			return -1;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)&param, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;	
		if (ret) {
			return param.nOperator;
		}
		return -1;
	}
	void Clear() {
		if (m_lock == true) return false;
		IocpParam* pParam = new IocpParam(MQClear, T());
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false) delete pParam;
		return ret;
	}
private:
	static void ThreadEntry(void* arg) {
		CMyQueue<T>* thiz = (CMyQueue<T>*)arg;
		thiz->ThreadFunc();
		_endthread();
	}
	void ThreadFunc() {
		DWORD dwTransferred = 0;
		ULONG_PTR completionKey = 0;
		OVERLAPPED* pOverlapped = NULL;
		PPARAM* pParam = NULL;
		while (GetQueuedCompletionStatus(m_hCompletionPort, &dwTransferred, &completionKey, &pOverlapped, INFINITE)) {
			if (dwTransferred == 0 || completionKey == NULL) {
				printf("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)completionKey;
			switch (pParam->nOperator) {
			case MQPush:
				m_lstData.push_back(pParam->tData);
				delete pParam;
				break;
			case MQPop:
				if (m_lstData.size() > 0) {
					pParam->tData = m_lstData.front();
					m_lstData.pop_front();
				}
				if (pParam->hEvent != NULL) {
					SetEvent(pParam->hEvent);
				}
				break;
			case MQSize:
				pParam->nOperator = m_lstData.size();
				if (pParam->hEvent != NULL) {
					SetEvent(pParam->hEvent);
				}
				break;
			case MQClear:
				m_lstData.clear();
				delete pParam;
				break;
			default:
				OutputDebugString("unknown operator\r\n");
				break;
			}
		}
		CloseHandle(m_hCompletionPort);	//线程函数要结束了在这close是没问题的
	}
private:
	std::list<T> m_lstData;
	HANDLE m_hCompletionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock 	//原子操作:队列正在析构
};

