#ifndef myProxy_h__
#define myProxy_h__

#include "Singleton.h"
#include "myClient.h"
#include "SafeContainer.h"

/**
���մӿͻ�����������
*/
class CxTcpDelegateClient : public CxTcpDelegate
{
public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpClose(CxTcpClient* sender) {};
	virtual void OnTcpOpen(CxTcpClient* sender) {};
};


/**
���մӱ�����ķ���������������
*/
class CxTcpDelegateProxy : public CxTcpDelegate
{
public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size) {  };
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size) {  };
	virtual void OnTcpClose(CxTcpClient* sender) {};
	virtual void OnTcpOpen(CxTcpClient* sender) {};
};


/**

*/
class CxTcpClientProxy : public CxTcpClient
{
public:
	CxTcpClientProxy();


public:
	//CxSafeMap<int64, CxTcpClient*> clients;
	CxTcpClient* client;

};


/**

*/
class CxMyProxy : public Singleton<CxMyProxy>,public CxTcpDelegateProxy
{
public:
	CxMyProxy();

public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size);

public:
//	void SendToClient(int fd, const char* buf, int size);
//	void SendToTarget(int fd, const char* buf, int size);
//	void OnRecvFromTarget(int fd, const char* buf, int size);
//	void OnRecvFromClient(int fd, const char* buf, int size);


public:
	/**
	���Ŀ�����Ĵ�������
	*/
	CxTcpClient* find(sockaddr_in _addr);
	CxTcpClient* findWitchConnect(sockaddr_in _addr);
	CxTcpClient* createWitchConnect(sockaddr_in _addr);

protected:
	/**
	0 Ĭ��ֵ ������һ��socket���ӵ�Ŀ�������
	1 ����һ��socket��Ŀ�������ͨ��
	*/
	int proxy_type;
	CxSafeVector<CxTcpClientProxy*> proxyConns;

};



#endif // myProxy_h__
