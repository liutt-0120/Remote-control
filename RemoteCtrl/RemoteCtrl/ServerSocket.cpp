#include "pch.h"
#include "ServerSocket.h"

//��ʽһ
//CServerSocket server;

//�޷��������ⲿ���� "private: static class CServerSocket * CServerSocket::m_instance" (? m_instance@CServerSocket@@0PAV1@A)
//���������������ʽ��ʼ����̬��Ա������Ϊ�ξ�̬��Ա�������������ڳ�ʼ������̬��Ա���������������������������еģ����������ڳ�ʼ������ÿ����һ������ͻ��ó�ʼֵ���Ǿ�̬��Ա����������Ȼ������
CServerSocket* CServerSocket::m_instance = NULL;


CServerSocket::CGarbo CServerSocket::garbo;		//�������ˣ���æ�Ѿ�ָ̬��delete��
