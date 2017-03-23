#ifndef myServer_h__
#define myServer_h__

#include "uv.h"
#include "LibXs.h"
#include "Singleton.h"

class CxMyServer : public Singleton<CxMyServer>
{
public:
	
	enum STS
	{
		INIT,
		INITED,
	};

public:
	CxMyServer();
	virtual ~CxMyServer();

public:

	static void connect_cb(uv_stream_t* stream, int status);
	static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void shutdown_cb(uv_shutdown_t* req, int status);
	static void close_cb(uv_handle_t* handle);
	static void wait_for_while(uv_idle_t* handle);

public:
	void Run();
	void Listen(std::string _addr,int _port,bool _auto_port=false);
	
protected:
	uv_loop_t*  loop;
	uv_tcp_t    tcp_server;

	int m_nState;

};


class CxMyService
{
public:
	static void OnStart();
	static void OnStop();

	static void Shutdown();

	/**
	服务器状态改变的时候
	*/
	static void OnChangeStatus(int64 status);

};


#endif // myServer_h__
