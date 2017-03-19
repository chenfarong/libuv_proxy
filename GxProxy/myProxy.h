#ifndef myProxy_h__
#define myProxy_h__

#include "Singleton.h"
#include "myClient.h"
#include "SafeContainer.h"

/**
接收从客户端来的数据
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
接收从被代理的服务器传来的数据
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
	获得目标服务的代理连接
	*/
	CxTcpClient* find(sockaddr_in _addr);
	CxTcpClient* findWitchConnect(sockaddr_in _addr);
	CxTcpClient* createWitchConnect(sockaddr_in _addr);

protected:
	/**
	0 默认值 单独的一个socket连接到目标服务器
	1 共享一个socket与目标服务器通信
	*/
	int proxy_type;
	CxSafeVector<CxTcpClientProxy*> proxyConns;

};



#endif // myProxy_h__
