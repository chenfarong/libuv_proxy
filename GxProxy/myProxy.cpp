
#include "GxProxy.h"
#include "myProxy.h"


X_IMPL_SINSTANCE(CxMyProxy)



CxMyProxy::CxMyProxy()
{
	proxy_type = 0;
}

CxMyProxy::~CxMyProxy()
{
	for (auto it : proxyConns.container)
	{
		delete it;
	}
	proxyConns.container.clear();
}

void CxMyProxy::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{
	//TODO 根据类型 看看是否要加上前边的客户端标记

	CxTcpClientProxy* conn = (CxTcpClientProxy*)sender;
	uv_stream_t* stream = (uv_stream_t*)&conn->client;

	write_req_t * wr = (write_req_t*)malloc(sizeof *wr);
	XX_ASSERT(wr != NULL);
	wr->buf = uv_buf_init((char*)buf, size); //这里没有申请新的内存

	int r = uv_write(&wr->req, stream, &wr->buf, 1, CxTcpClientProxy::write_cb);
	if (r!=0)
	{
		XX_FATAL("uv_write failed");
	}

}

void CxMyProxy::OnTcpRecv(CxTcpClient* sender, const char* buf, int size)
{
	//根据类型 看看是否要剥离前边的客户端标记
	if (proxy_type == 0) {
		CxTcpClientProxy* _tcp = (CxTcpClientProxy*)sender;
		if (_tcp->cli) _tcp->cli->SendPto(buf, size);
	}
	else {
		int* fd = (int*)buf;
		CxMyClient* cli= CxMyClientPool::Instance()->findClientByFD(*fd, false);
		if (cli) cli->SendPto(buf, size);
	}
}

CxTcpClientProxy* CxMyProxy::find(sockaddr_in _addr)
{
	for (auto it:proxyConns.container)
	{
		if (memcmp(&it->peer, &_addr, sizeof(sockaddr_in)) == 0)
			return it;
	}
	return NULL;
}

CxTcpClientProxy* CxMyProxy::findWitchConnect(sockaddr_in _addr)
{
	CxTcpClientProxy* tcpClient=NULL;
	
	if (proxy_type == 0) {
		tcpClient = createWitchConnect(_addr);
	}else{
		tcpClient = find(_addr);
		if (tcpClient == NULL) return createWitchConnect(_addr);
	}

	return tcpClient;
}

CxTcpClientProxy* CxMyProxy::createWitchConnect(sockaddr_in _addr)
{
	CxTcpClientProxy* cli = createClient();
	cli->Open(_addr);
	return cli;
}

CxTcpClientProxy* CxMyProxy::createClient()
{
	//从回收中找一个 如果没找到就新建
	for (auto it : proxyConns.container)
	{
		if (!it->IsOnline()) {
			XLOG_DEBUG("回收站重新使用");
			return it;
		}
	}

	CxTcpClientProxy* cli = new CxTcpClientProxy();
	cli->SetDelegate(this);
	proxyConns.safe_push_back(cli);
	return cli;
}

void CxMyProxy::Recycle(CxTcpClientProxy* _cli)
{
	XX_ASSERT(_cli);
	//_cli->Close();
	//TODO 把这个放入回收站 等待重新被使用

	_cli->SetFD(-1);


}

CxTcpClientProxy::CxTcpClientProxy()
{
	cli = NULL;
}

static uv_os_sock_t create_tcp_socket(void) {
	uv_os_sock_t sock;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
#ifdef _WIN32
	XX_ASSERT(sock != INVALID_SOCKET);
#else
	XX_ASSERT(sock >= 0);
#endif

#ifndef _WIN32
	{
		/* Allow reuse of the port. */
		int yes = 1;
		int r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
		XX_ASSERT(r == 0);
	}
#endif

	return sock;
}



int CxTcpClientProxy::Open(sockaddr_in _addr)
{

	connect_req.data = this;

	//uv_tcp_connect()
	sock = create_tcp_socket();

	int r = uv_tcp_init(uv_default_loop(), &client);
	XX_ASSERT(r == 0);

	r = uv_tcp_open(&client, sock);
	XX_ASSERT(r == 0);

	r = uv_tcp_connect((uv_connect_t*)&connect_req,
		&client,
		(const struct sockaddr*) &_addr,
		CxTcpClientProxy::connect_cb);
	XX_ASSERT(r == 0);

	//这里一个代理和客户端绑定的事件

	return 0;
}

void CxTcpClientProxy::Close()
{
	int r=0;

//#ifdef _WIN32
//	r = closesocket(sock);
//#else
//	r = close(sock);
//#endif

	uv_shutdown_t* sreq = (uv_shutdown_t*)malloc(sizeof *sreq);
	XX_ASSERT(0 == uv_shutdown(sreq, connect_req.handle, shutdown_cb));

	
}

struct sx_muv_read_q
{
	uv_tcp_t handle;
	CxTcpClientProxy* owner;
};

struct sx_proxy_stream 
{
	uv_stream_t stream;
	CxTcpClientProxy* owner;
};

void CxTcpClientProxy::connect_cb(uv_connect_t* req, int status)
{
	int r = 0;
	uv_stream_t* stream = req->handle;
	stream->data = req->data;

	if (status != 0)
	{
		XLOG_WARN("连接失败");
		goto lend;
	}

	//
	CxTcpClientProxy* _cli = (CxTcpClientProxy*)req->data;

	_cli->SetFD((int64)_cli->sock);

	CxMyClient* _mcli = (CxMyClient*)_cli->cli;// ->m_proxy = _cli;
	_mcli->m_proxy = _cli;



	/* Shutdown on drain. */
	//r = uv_shutdown(shutdown_req, stream, shutdown_cb);
	//XX_ASSERT(r == 0);

	/* Start reading */
	r = uv_read_start(stream, CxTcpClientProxy::alloc_cb, CxTcpClientProxy::read_cb);
	if (r != 0) {
		XLOG_ERROR("%s", uv_err_name(r));
		//连接服务器失败 
		goto lend;
	}
	XX_ASSERT(r == 0);


lend:
//	free(req); //后面还要使用 成员属性 不得释放
	XLOG_DEBUG("成功连接");
}

void CxTcpClientProxy::write_cb(uv_write_t* req, int status)
{
	if (status) {
		fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));
		XX_ASSERT(0);
	}

	write_req_t* wr = (write_req_t*)req;

//	free(wr->buf.base); //释放内存 导致崩溃
	free(req);
}

void CxTcpClientProxy::read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	XX_ASSERT(stream != NULL);
	CxTcpClientProxy* _owner = (CxTcpClientProxy*)stream->data;// container_of(conn, sx_proxy_connect, uvreq);

	if (nread >= 0) {
		if(_owner) _owner->Recv(buf->base, nread);
	}
	else {
//		XX_ASSERT(nread == UV_EOF);
//		printf("GOT EOF\n");
		XLOG_WARN("连接目标服务器断开");
		uv_close((uv_handle_t*)stream, close_cb);
	}

	free(buf->base); //释放内存
}

void CxTcpClientProxy::shutdown_cb(uv_shutdown_t* req, int status)
{
	uv_close((uv_handle_t*)req->handle, close_cb);
	free(req);
}

void CxTcpClientProxy::close_cb(uv_handle_t* handle)
{
	//释放内存重要
	CxTcpClientProxy* _self = (CxTcpClientProxy*)handle->data;
	if(_self) _self->SetFD(-1);
}

void CxTcpClientProxy::alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	do {
		buf->base = (char*)malloc(suggested_size);
	} while (buf->base == NULL);

	buf->len = suggested_size;
}
