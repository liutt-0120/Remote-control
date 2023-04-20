#include "pch.h"
#include "ServerSocket.h"

//方式一
//CServerSocket server;

//无法解析的外部符号 "private: static class CServerSocket * CServerSocket::m_instance" (? m_instance@CServerSocket@@0PAV1@A)
//解决方法：类外显式初始化静态成员变量。为何静态成员变量不能在类内初始化？静态成员变量是所有类对象及子类对象所共有的，若能在类内初始化，那每创建一个对象就会用初始值覆盖静态成员变量，这显然不合理。
CServerSocket* CServerSocket::m_instance = NULL;


CServerSocket::CGarbo CServerSocket::garbo;		//临了临了，帮忙把静态指针delete了
