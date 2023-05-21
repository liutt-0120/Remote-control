#pragma once
#include "pch.h"
#include <atomic>
#include <vector>
#include <mutex>


/*
* ���û��Զ�����Ӳ���
* ������ʲôִ������, �̳�ThreadFuncBase��
*/
class CThreadFuncBase{};
typedef int (CThreadFuncBase::* FUNCTYPE)();


/*
* ��Ҫִ�е�������ȫ�����߳�
* ����ThreadWorkerʵ��, ��ʵ�����½��߳�ִ��
*/
class CThreadWorker {
public: 
	CThreadWorker():thiz(NULL), func(NULL){}
	CThreadWorker(void* obj, FUNCTYPE f):thiz((CThreadFuncBase*)obj),func(f) {}

	//��������
	CThreadWorker(const CThreadWorker& worker){
		thiz = worker.thiz;
		func = worker.func;
	}
	//������ֵ
	CThreadWorker& operator=(const CThreadWorker& worker){
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}
	//��������
	int operator()() {
		if (IsValid()) {
			return (thiz->*func)();
		}
		return -1;
	}
	//�����Ƿ���Ч
	bool IsValid() const{
		return (thiz != NULL) && (func != NULL);
	}
private:
	//�Զ������
	CThreadFuncBase* thiz;
	//�Զ�������ĳ�Ա����
	FUNCTYPE func;

};

//�߳���
class CMyThread
{
public:
	CMyThread(){
		m_hThread = NULL;
		m_bStatus = false;
	}
	~CMyThread() {
		Stop();
	}

	bool Start() {
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(&CMyThread::ThreadEntry, 0, this);
		if (!IsValid()) {
			m_bStatus = false;
		}
		return m_bStatus;
	}

	bool Stop() {
		if (m_bStatus == false) return true;
		m_bStatus = false;
		DWORD ret = WaitForSingleObject(m_hThread, INFINITE);
		if (ret == WAIT_TIMEOUT) //�߳����������⼸�����Ǹ����,��ô����ʲô����
			TerminateThread(m_hThread, -1);	
		UpdateWorker();
		return ret == WAIT_OBJECT_0;
	}

	// ����߳��Ƿ���Ч
	bool IsValid() {	//����true��ʾ��Ч ����false��ʾ�߳��쳣������ֹ
		if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE) return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	}

	//�����߳��ڵĹ���
	void UpdateWorker(const ::CThreadWorker& worker = ::CThreadWorker()) {
		if (m_worker.load() != NULL && m_worker.load() != &worker) {
			::CThreadWorker* pWorker = m_worker.load();
			m_worker.store(NULL);
			delete pWorker;
		}
		//�о�����߼�������
		if (m_worker.load() == &worker) return;
		if (!worker.IsValid()) {
			m_worker.store(NULL);
			return;
		}
		//��ΪNULL˵���߳��еĹ������ڼ���

		m_worker.store(new ::CThreadWorker(worker));
	}

	//�߳��Ƿ����, m_worker���ǿ�ʵ����Ϊ����. trueΪ����,falseΪæµ
	bool IsIdle() {
		if (m_worker.load() == NULL) return true;
		return !m_worker.load()->IsValid();
	}
private:

	//�̺߳���
	void ThreadWorker() {
		while (m_bStatus) {

			//û��ɿ�ת
			if (m_worker.load() == NULL) {
				Sleep(1);
				continue;
			}
			::CThreadWorker worker = *m_worker.load();
			if (worker.IsValid()) {			//��workerΪ��ʵ���Ϳ�ת. ��;��ʱ����Ҫ���߳̿ɹҿ�, �������߳̽���
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
					int ret = worker();			//ѭ������CThreadFuncBase������ĳ�Ա����
					if (ret != 0) {
						CString str;
						str.Format(_T("thread found warning code %d\r\n"), ret);
						OutputDebugString(str);
					}
					if (ret < 0) {
						::CThreadWorker* pWorker = m_worker.load();
						m_worker.store(NULL);
						delete pWorker;
					}
				}

			}
			else {
				Sleep(1);
			}
		}
	}
	static void ThreadEntry(void* arg) {
		CMyThread* thiz = (CMyThread*)arg;
		if (thiz) {
			thiz->ThreadWorker();
		}
		_endthread();
	}
private:
	HANDLE m_hThread;
	bool m_bStatus;//false��ʾ�߳̽��ر� true��ʾ�߳���������
	std::atomic<::CThreadWorker*> m_worker;	//ԭ�Ӳ���,������ʹ�ö���߳���ȷ�ٿض���
};

//�̳߳�
class CThreadPool {
public:
	CThreadPool(size_t size) {
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++) {
			m_threads[i] = new CMyThread();
		}
	}
	CThreadPool(){}
	~CThreadPool(){
		Stop();
		m_threads.clear();
	}
	bool Invoke() {
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->Start() == false) {
				ret = false;
				break;
			}
		}
		if (ret == false) {		//ֻҪ��һ���߳�����ʧ��, �͹ر�ȫ���߳�
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop();
			}
		}
		//�̶߳������ɹ�����Ϊ�ɹ�
		return true;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
			delete m_threads[i];
			m_threads[i] = NULL;
		}
	}

	//��worker����������߳�. ����-1Ϊ�̶߳�æ,>=0��ʾ�������n���߳�
	int DispatchWorker(const CThreadWorker& worker) {
		int index = -1;
		m_lock.lock();	//������߳�ͬʱ���ø�ͬһ�̷߳���worker
						//������ѯ��ʽ�̹߳�ģ��Щ(�ټ�����)�ͻ�������,�߳�������ʱ�ﵽ���ʱ�临�Ӷ�
						//�Ż�˼·:�ṩһ���б�洢�����̱߳��,�̱߳�ʹ�þʹ��б����Ƴ�,���оͼ����б�,���б�.size()�Ϳ��ж���û�п����߳�
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->IsIdle()) {
				m_threads[i]->UpdateWorker(worker);
				index = i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadValid(size_t index) {
		if (index < m_threads.size()) {
			return m_threads[index]->IsValid();
		}
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<CMyThread*> m_threads;
};



