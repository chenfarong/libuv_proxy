#ifndef XTcpClient_h__
#define XTcpClient_h__

#include <stdint.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

#include "XNetBuffer.h"
#include "xnet.h"

typedef int64_t int64;
typedef uint64_t uint64;

class XsTcpClient;

class XsTcpClientDelegate
{
public:
	virtual void OnConnectBefor(XsTcpClient* _sender,int _status) {};
	virtual void OnConnectAfter(XsTcpClient* _sender, int _status) {};
	virtual void OnDisconnect(XsTcpClient* _sender, int _status) {};

	virtual void OnSendAfter(XsTcpClient* _sender,const char* _buf,int _size) {};
	virtual void OnRecv(XsTcpClient* _sender, const char* _buf, int _size) {};

	/**
	收到一个完整数据包
	*/
	virtual void OnRecvPack(XsTcpClient* _sender, const char* _buf, int _size) {};
};

class XsTcpClient
{
public:
	static void xnet_recv_proc(void* sender, xnet_sock s, char* _lpBuf, int _size);

public:
	XsTcpClient();

	void Reset();

	int Connect(const char* _server, int _service);
	void Disconnect();

	bool IsConnected();

//	bool IsBlocking();

	void SetDelegate(XsTcpClientDelegate* _delegate);
	void SetPtoType(int _type,const char* _emark,unsigned int _size);


	enum TCO
	{
		TCO_CONNECT_TIMEOUT,	//连接服务器超时时间
		TCO_RECV_WORK_THREAD,	//接收数据采用线程模式
	};

	/**
	设置选项值
	*/
	void SetOption(int _type, const char* _value);

	void Step();

	/**
	发送数据给服务器
	@return 如果成功返回发送的字节数 失败返回负值
	*/
	int Send(const char* _buf, unsigned int _size);

	virtual void Recv(const char* buf, int size);

	uint64 GetSendBytes() { return m_send_bytes; }
	uint64 GetRecvBytes() { return m_recv_bytes; }
	uint64 GetSendTick() { return m_send_tick; }
	uint64 GetRecvTick() { return m_recv_tick; }

protected:
	int m_pto_type;
	int64 m_socket;
	XsTcpClientDelegate* m_delegate;

	bool m_connected;

	CxNetBuffer m_input;
	xnet_sock sk;

	uint64	
		m_send_bytes,	//发送多少字节
		m_recv_bytes,	//接收多少字节
		m_send_tick,	//发送多少个包
		m_recv_tick;	//接收多少个包

private:
	char m_buffer[4096];

};


#endif // XTcpClient_h__
