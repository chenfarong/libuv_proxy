
#include "myServer.h"
#include "GxProxy.h"
#include "xnet.h"


/*
�������һ���˿�
*/


X_IMPL_SINSTANCE(CxMyServer)


CxMyServer::CxMyServer()
{
	m_nState = INIT;

	loop = uv_default_loop();
	uv_loop_init(loop);

}

CxMyServer::~CxMyServer()
{

}

int64 x_uv_tcp_fd(uv_tcp_t& handle)
{
#ifdef _MSC_VER
	return (int64)handle.socket;
#else
	return handle.io_watcher.fd;
#endif

}

void CxMyServer::connect_cb(uv_stream_t* stream, int status)
{
	conn_rec* conn;
	int r;
	struct sockaddr sockname, peername;
	int namelen = sizeof(struct sockaddr);


	XX_ASSERT(status == 0);
	XX_ASSERT(stream == (uv_stream_t*)&CxMyServer::Instance()->tcp_server);

	conn = (conn_rec*)malloc(sizeof *conn);
	XX_ASSERT(conn != NULL);

	r = uv_tcp_init(stream->loop, &conn->handle);
	XX_ASSERT(r == 0);

	r = uv_accept(stream, (uv_stream_t*)&conn->handle);
	XX_ASSERT(r == 0);


	//��ȡ�ͻ��˵�IP��ַ
	r = uv_tcp_getpeername(&conn->handle, &peername, &namelen);
	struct sockaddr_in* check_addr = (struct sockaddr_in*)&peername;
	//char* check_ip = inet_ntoa(*(in_addr*)check_addr);
	char cli_ip[128];
	uv_ip4_name(check_addr, (char*)cli_ip, 128);

	int64 cli_fd = x_uv_tcp_fd(conn->handle);
	conn->client = CxMyClientPool::Instance()->findClientByFD(cli_fd, true);
	conn->client->handle = &conn->handle;
//	conn->client->TimeConn =  XG_GetTickCount();

	r = uv_read_start((uv_stream_t*)&conn->handle, alloc_cb, read_cb);
	XX_ASSERT(r == 0);


	//conn->client->OnAccept(cli_fd,conn->ssl);

	//conn->client->SetProtocol(0, "\r\n\r\n", 4);
	//sprintf(conn->client->ipAddr, "%s", cli_ip);
	//conn->client->input.clear();

	//if (CxEnvironment::enable_alone)
	//{
	//	std::string str;
	//	str = CxEnvironment::Instance()->ipAddr + ":" + std::to_string(CxEnvironment::Instance()->port)
	//		+ ":" + std::to_string((int)conn->client->fd);
	//	conn->client->name = str.c_str();
	//	CxPlayer* player = sharedPlayerManager->findAnCreate((int)conn->client->fd, (int)conn->client->fd);
	//	player->clear();
	//	player->setLineState(true);
	//}


	XLOG_INFO("�µĿͻ��� %d ���� %s\n", x_uv_tcp_fd(conn->handle), cli_ip);
}

void CxMyServer::alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	//TODO �����Ż� ����Ƶ�������ͷ�

	do {
		buf->base = (char*)malloc(suggested_size);
	} while (buf->base == NULL);
	buf->len = suggested_size;
}

void CxMyServer::read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	int r;
	conn_rec* conn;
	conn = container_of(stream, conn_rec, handle);

#if(0)
	//�����ǵ�ǰֱ�ӷ��ظ��ͻ���
	if (nread >= 0)
	{
		
		write_req_t * wr = (write_req_t*)malloc(sizeof *wr);
		XX_ASSERT(wr != NULL);
		wr->buf = uv_buf_init(buf->base, nread);

		if (uv_write(&wr->req, stream, &wr->buf, 1, 0)) {
			XX_FATAL("uv_write failed");
		}
		return;
	}
#endif

	//TODO ��������
	if (nread >=0)
	{
		conn->client->Recv(buf->base, nread);
		free(buf->base);
		return;
	}

	//if (nread == 0)
	//{
	//	XLOG_INFO("�ͻ��������Ͽ�����");
	//	return;
	//}


	XX_ASSERT(nread == UV_EOF);

	

	r = uv_shutdown(&conn->shutdown_req, stream, shutdown_cb);
	XX_ASSERT(r == 0);
}

void CxMyServer::shutdown_cb(uv_shutdown_t* req, int status)
{
	conn_rec* conn = container_of(req, conn_rec, shutdown_req);
	uv_close((uv_handle_t*)&conn->handle, close_cb);
}

void CxMyServer::close_cb(uv_handle_t* handle)
{
	conn_rec* conn = container_of(handle, conn_rec, handle);
	conn->client->SetFD(-1);
	free(conn);
	XLOG_WARN("�ͻ��˶Ͽ�\n");
}

void CxMyServer::wait_for_while(uv_idle_t* handle)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	CxMyClientPool::Instance()->Step();
//	XLOG_INFO("11");

}

void CxMyServer::Run()
{
	if (m_nState != INITED) return;

	uv_idle_t idler;
	uv_idle_init(loop, &idler);
	uv_idle_start(&idler, CxMyServer::wait_for_while);

	uv_run(loop, UV_RUN_DEFAULT);

}

void CxMyServer::Listen(std::string _addr, int _port, bool _auto_port/*=false*/)
{
	int r = 0;
	struct sockaddr_in addr;

	
	r = xnet_addr(0, _port, &addr);
	XX_ASSERT(r == 0);

	r = uv_tcp_init(loop, &tcp_server);
	XX_ASSERT(r == 0);

	r = uv_tcp_bind(&tcp_server, (const struct sockaddr*) &addr, 0);
	while (/*_env->auto_port &&*/ r != 0)
	{
		addr.sin_port = htons(0);
		r = uv_tcp_bind(&tcp_server, (const struct sockaddr*) &addr, 0);
		//TODO ���ö˿ں�
		//app_service_port = feServerStart(server_loop, "0", app_service_port, eventCB);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	XX_ASSERT(r == 0);

	r = uv_listen((uv_stream_t*)&tcp_server, 128, CxMyServer::connect_cb);
	XX_ASSERT(r == 0);

	m_nState = INITED;

	XLOG_WARN("�����˿�:%d",_port);

}
