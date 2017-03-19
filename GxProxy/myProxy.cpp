
#include "GxProxy.h"
#include "myProxy.h"


X_IMPL_SINSTANCE(CxMyProxy)



CxMyProxy::CxMyProxy()
{
	proxy_type = 0;
}

void CxMyProxy::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{
	//TODO 根据类型 看看是否要加上前边的客户端标记

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

CxTcpClient* CxMyProxy::find(sockaddr_in _addr)
{
	for (auto it:proxyConns.container)
	{
		if (memcmp(&it->peer, &_addr, sizeof(sockaddr_in)) == 0)
			return it;
	}
	return NULL;
}

CxTcpClient* CxMyProxy::findWitchConnect(sockaddr_in _addr)
{
	CxTcpClient* tcpClient=NULL;
	
	if (proxy_type == 0) {
		tcpClient = createWitchConnect(_addr);
	}else{
		tcpClient = find(_addr);
		if (tcpClient == NULL) return createWitchConnect(_addr);
	}

	return tcpClient;
}

CxTcpClient* CxMyProxy::createWitchConnect(sockaddr_in _addr)
{
	CxTcpClientProxy* cli = new CxTcpClientProxy();
	cli->Open(_addr);
	cli->SetDelegate(this);
	proxyConns.safe_push_back(cli);
	return cli;
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
	uv_connect_t* connect_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
	//uv_tcp_connect()
	sock = create_tcp_socket();

	int r = uv_tcp_init(uv_default_loop(), &client);
	XX_ASSERT(r == 0);

	r = uv_tcp_open(&client, sock);
	XX_ASSERT(r == 0);

	r = uv_tcp_connect(connect_req,
		&client,
		(const struct sockaddr*) &_addr,
		CxTcpClientProxy::connect_cb);
	XX_ASSERT(r == 0);
	return 0;
}

void CxTcpClientProxy::Close()
{
	int r;
#ifdef _WIN32
	r = closesocket(sock);
#else
	r = close(sock);
#endif
}

void CxTcpClientProxy::connect_cb(uv_connect_t* req, int status)
{
	//uv_buf_t buf = uv_buf_init("PING", 4);
	uv_stream_t* stream;
	//uv_shutdown_t* shutdown_req=(uv_shutdown_t*)malloc(sizeof(uv_shutdown_t));
	int r=0;

	stream = req->handle;


	/* Shutdown on drain. */
	//r = uv_shutdown(shutdown_req, stream, shutdown_cb);
	//XX_ASSERT(r == 0);

	/* Start reading */
	r = uv_read_start(stream, alloc_cb, read_cb);
	XX_ASSERT(r == 0);
}

void CxTcpClientProxy::write_cb(uv_write_t* req, int status)
{

}

void CxTcpClientProxy::read_cb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
{
	XX_ASSERT(tcp != NULL);

	if (nread >= 0) {
		//ASSERT(nread == 4);
		//ASSERT(memcmp("PING", buf->base, nread) == 0);
		
	}
	else {
		XX_ASSERT(nread == UV_EOF);
		printf("GOT EOF\n");
		uv_close((uv_handle_t*)tcp, close_cb);
	}

	free(buf->base); //释放内存
}

void CxTcpClientProxy::shutdown_cb(uv_shutdown_t* req, int status)
{
	free(req);
}

void CxTcpClientProxy::close_cb(uv_handle_t* handle)
{

}

void CxTcpClientProxy::alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}
