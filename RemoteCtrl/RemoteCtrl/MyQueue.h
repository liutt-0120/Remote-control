#pragma once
#include "pch.h"
#include <list>
#include <atomic>
#include "MyThread.h"

/// <summary>
/// �̰߳�ȫ�Ķ���
/// </summary>
/// <typeparam name="T"></typeparam>
template<class T>
class CMyQueue
{//����IOCPʵ���̰߳�ȫ�Ķ���
public:
	typedef struct IocpParam {
		size_t nOperator; //����
		T tData;  //����
		HANDLE hEvent;	//pop������Ҫ
		IocpParam(int op, T data, HANDLE hEve = NULL) {
			nOperator = op;
			tData = data;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = MQNone;
		}
	}PPARAM;	//Post Parameter  ����Ͷ����Ϣ�Ľṹ��

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
			m_hThread = (HANDLE)_beginthread(CMyQueue<T>::ThreadEntry, 0, this);
		}
	}

	virtual ~CMyQueue() {
		if (m_lock) return;		//��ֹ��������������
		m_lock = true;
		BOOL ret = PostQueuedCompletionStatus(m_hCompletionPort, 0, NULL, NULL);
		WaitForSingleObject(m_hThread, INFINITE);
		TRACE("~CMyQueue��post���:%d\r\n", ret);
		if (m_hCompletionPort != NULL) {
			HANDLE hTemp = m_hCompletionPort;
			m_hCompletionPort = NULL;
			CloseHandle(hTemp);	//�ظ�closeҲû����
		}

	}

	bool PushBack(const T& data) {
		IocpParam* pParam = new IocpParam(MQPush, data);
		if (m_lock == true) {
			delete pParam;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, 
			sizeof(PPARAM), 
			(ULONG_PTR)pParam, 
			NULL);
		if (ret == false) delete pParam;
		return ret;
	}

	virtual bool PopFront(T& data) {
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
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;	//push�ﲻ��ȴ�����Ϊ����д�����ݵĸ�Ч��Ҫ��Ҫ��������.
		if (ret) {
			data = param.tData;
		}
		return ret;
	}

	size_t Size() {	//���̰߳�ȫ��size
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

	bool Clear() {
		if (m_lock == true) return false;
		IocpParam* pParam = new IocpParam(MQClear, T());
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false) delete pParam;
		return ret;
	}
protected:
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
		while (GetQueuedCompletionStatus(m_hCompletionPort, 
			&dwTransferred, 
			&completionKey, 
			&pOverlapped, INFINITE)) {
			if (dwTransferred == 0 || completionKey == NULL) {
				printf("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)completionKey;
			DealParam(pParam);
		}

		//�����Ա�̣���ֹ���滹������
		while (GetQueuedCompletionStatus(m_hCompletionPort, 
			&dwTransferred, 
			&completionKey, 
			&pOverlapped, 0)) {	//�ȴ�ʱ��0ms������в������ݾͻᴦ���������ϳ�ʱ
			if (dwTransferred == 0 || completionKey == NULL) {
				printf("thread is prepare to exit!\r\n");
				continue;
			}
			pParam = (PPARAM*)completionKey;
			DealParam(pParam);
		}
		HANDLE hTemp = m_hCompletionPort;
		CloseHandle(hTemp);	
		m_hCompletionPort = NULL;
	}
	virtual void DealParam(PPARAM* pParam) {
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
			OutputDebugStringA("unknown operator\r\n");
			break;
		}

	}

protected:
	std::list<T> m_lstData;
	HANDLE m_hCompletionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock; 	//ԭ�Ӳ���:������������
};

template<class T>
class CSendQueue :public CMyQueue<T>,public CThreadFuncBase {
public:
	typedef int (CThreadFuncBase::* EDYCALLBACK)(T& data);
	CSendQueue(void* obj, EDYCALLBACK callback):CMyQueue<T>(),m_base((CThreadFuncBase*)obj),m_callback(callback){
		m_thread.Start();
		m_thread.UpdateWorker(::CThreadWorker(this, (FUNCTYPE)&CSendQueue<T>::ThreadTick));
	}
	virtual ~CSendQueue(){
		m_thread.Stop();
		m_base = NULL;
		m_callback = NULL;
	}
protected:
	virtual bool PopFront(T& data) {
		return false;
	};
	bool PopFront() {
		typename CMyQueue<T>::IocpParam* param = new typename CMyQueue<T>::IocpParam(CMyQueue<T>::MQPop,T());
		if (CMyQueue<T>::m_lock) {
			delete param;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(CMyQueue<T>::m_hCompletionPort, sizeof(*param), (ULONG_PTR)&param, NULL);
		if (ret == false) {
			delete param;
			return false;
		}

		return ret;
	}

	int ThreadTick() {
		if (WaitForSingleObject(CMyQueue<T>::m_hThread, 0) != WAIT_TIMEOUT) return 0;
		if (CMyQueue<T>::m_lstData.size() > 0) {
			PopFront();
		}
		Sleep(1);
		return 0;
	}

	virtual void DealParam(typename CMyQueue<T>::PPARAM* pParam) {
		switch (pParam->nOperator) {
		case CMyQueue<T>::MQPush:
			CMyQueue<T>::m_lstData.push_back(pParam->tData);
			delete pParam;
			break;
		case CMyQueue<T>::MQPop:
			if (CMyQueue<T>::m_lstData.size() > 0) {
				pParam->tData = CMyQueue<T>::m_lstData.front();
				if((m_base->*m_callback)(pParam->tData)==0)
					CMyQueue<T>::m_lstData.pop_front();
			}
			delete pParam;
			break;
		case CMyQueue<T>::MQSize:
			pParam->nOperator = CMyQueue<T>::m_lstData.size();
			if (pParam->hEvent != NULL) {
				SetEvent(pParam->hEvent);
			}
			break;
		case CMyQueue<T>::MQClear:
			CMyQueue<T>::m_lstData.clear();
			delete pParam;
			break;
		default:
			OutputDebugStringA("unknown operator\r\n");
			break;
		}

	}
private:
	CThreadFuncBase* m_base;
	EDYCALLBACK m_callback;
	CMyThread m_thread;
};

typedef CSendQueue<std::vector<char>>::EDYCALLBACK SENDCALLBACK;
