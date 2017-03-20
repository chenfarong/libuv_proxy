#ifndef myProxy_h__
#define myProxy_h__

#include "Singleton.h"
#include "myClient.h"
#include "SafeContainer.h"


/**

*/
class CxTcpClientProxy : public CxTcpClient
{
public:
	CxTcpClientProxy();

public:
	virtual int Open(sockaddr_in _addr);
	virtual void Close();

public:
	static void connect_cb(uv_connect_t* req, int status);
	static void write_cb(uv_write_t* req, int status);
	static void read_cb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
	static void shutdown_cb(uv_shutdown_t* req, int status);
	static void close_cb(uv_handle_t* handle);
	static void alloc_cb(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf);

public:
	CxTcpClient* cli;

	//��������
	uv_tcp_t client;
	uv_os_sock_t sock;

	uv_connect_t connect_req; //��������
};


/**

*/
class CxMyProxy : public Singleton<CxMyProxy>,public CxTcpDelegate
{
public:
	CxMyProxy();

public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size);

public:
	/**
	���Ŀ�����Ĵ�������
	*/
	CxTcpClientProxy* find(sockaddr_in _addr);
	CxTcpClientProxy* findWitchConnect(sockaddr_in _addr);
	CxTcpClientProxy* createWitchConnect(sockaddr_in _addr);

protected:
	/**
	0 Ĭ��ֵ ������һ��socket���ӵ�Ŀ�������
	1 ����һ��socket��Ŀ�������ͨ��
	*/
	int proxy_type;
	CxSafeVector<CxTcpClientProxy*> proxyConns;

};



#endif // myProxy_h__
