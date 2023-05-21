#pragma once
#include "pch.h"
#include <atomic>
#include <vector>
#include <mutex>


/*
* 由用户自定义添加操作
* 想增加什么执行内容, 继承ThreadFuncBase类
*/
class CThreadFuncBase{};
typedef int (CThreadFuncBase::* FUNCTYPE)();


/*
* 将要执行的内容完全脱离线程
* 创建ThreadWorker实例, 将实例更新进线程执行
*/
class CThreadWorker {
public: 
	CThreadWorker():thiz(NULL), func(NULL){}
	CThreadWorker(void* obj, FUNCTYPE f):thiz((CThreadFuncBase*)obj),func(f) {}

	//拷贝构造
	CThreadWorker(const CThreadWorker& worker){
		thiz = worker.thiz;
		func = worker.func;
	}
	//拷贝赋值
	CThreadWorker& operator=(const CThreadWorker& worker){
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}
	//调用重载
	int operator()() {
		if (IsValid()) {
			return (thiz->*func)();
		}
		return -1;
	}
	//内容是否有效
	bool IsValid() const{
		return (thiz != NULL) && (func != NULL);
	}
private:
	//自定义操作
	CThreadFuncBase* thiz;
	//自定义操作的成员函数
	FUNCTYPE func;

};

//线程类
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
		if (ret == WAIT_TIMEOUT) //线程这块这块查查这几个宏是干嘛的,这么做有什么意义
			TerminateThread(m_hThread, -1);	
		UpdateWorker();
		return ret == WAIT_OBJECT_0;
	}

	// 监控线程是否有效
	bool IsValid() {	//返回true表示有效 返回false表示线程异常或已终止
		if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE) return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	}

	//更新线程内的工作
	void UpdateWorker(const ::CThreadWorker& worker = ::CThreadWorker()) {
		if (m_worker.load() != NULL && m_worker.load() != &worker) {
			::CThreadWorker* pWorker = m_worker.load();
			m_worker.store(NULL);
			delete pWorker;
		}
		//感觉这块逻辑有问题
		if (m_worker.load() == &worker) return;
		if (!worker.IsValid()) {
			m_worker.store(NULL);
			return;
		}
		//不为NULL说明线程中的工作还在继续

		m_worker.store(new ::CThreadWorker(worker));
	}

	//线程是否空闲, m_worker里是空实例就为空闲. true为空闲,false为忙碌
	bool IsIdle() {
		if (m_worker.load() == NULL) return true;
		return !m_worker.load()->IsValid();
	}
private:

	//线程函数
	void ThreadWorker() {
		while (m_bStatus) {

			//没活干空转
			if (m_worker.load() == NULL) {
				Sleep(1);
				continue;
			}
			::CThreadWorker worker = *m_worker.load();
			if (worker.IsValid()) {			//若worker为空实例就空转. 中途暂时不需要该线程可挂空, 不用让线程结束
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
					int ret = worker();			//循环调用CThreadFuncBase派生类的成员函数
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
	bool m_bStatus;//false表示线程将关闭 true表示线程正在运行
	std::atomic<::CThreadWorker*> m_worker;	//原子操作,帮助你使用多个线程正确操控对象
};

//线程池
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
		if (ret == false) {		//只要有一个线程启动失败, 就关闭全部线程
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop();
			}
		}
		//线程都启动成功才认为成功
		return true;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
			delete m_threads[i];
			m_threads[i] = NULL;
		}
	}

	//将worker分配给空闲线程. 返回-1为线程都忙,>=0表示分配给第n个线程
	int DispatchWorker(const CThreadWorker& worker) {
		int index = -1;
		m_lock.lock();	//避免多线程同时调用给同一线程分配worker
						//这种轮询方式线程规模大些(百级以上)就会有问题,线程满负荷时达到最大时间复杂度
						//优化思路:提供一个列表存储空闲线程编号,线程被使用就从列表中移除,空闲就加入列表,用列表.size()就可判断有没有空闲线程
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



