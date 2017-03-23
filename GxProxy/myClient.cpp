
/*

客户端一旦连接上来，就发送一个欢迎信息： 告知版本号，是否启用通信加密，产品信息

*/

#include "myClient.h"
#include "LibXs.h"
#include "GxProxy.h"
#include "XNetBuffer.h"
#include "XString/XString.h"
#include "XLog.h"
#include "myProxy.h"
#include "uv.h"
#include "myServer.h"

X_IMPL_SINSTANCE(CxMyClientPool)


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

CxTcpClient::CxTcpClient()
{
	m_socket = -1;
	pto_type = 1;
	m_delegate = NULL;
}

CxTcpClient::~CxTcpClient()
{
	Close();
}

int CxTcpClient::Open(sockaddr_in _addr)
{
	return 0;
}

void CxTcpClient::Close()
{
	if (m_delegate) m_delegate->OnTcpClose(this);
}

int CxTcpClient::Send(const char* buf, int size)
{
	if (m_delegate) {
		m_delegate->OnTcpSend(this, buf, size);
	}
	return size;
}

int CxTcpClient::SendPto(const char* buf, int size)
{
	CxNetBuffer buffer;
	if (pto_type) {
		buffer.WritefixEMark2(buf, size, "\r\n\r\n", 4);
	}
	else {
		buffer.WritefixP32(buf, size);
	}
	return Send(buffer.c_str(), buffer.getContentLength());
}

int CxTcpClient::Recv(const char* buf, int size)
{

	m_tiBreak = time(NULL);

	m_input.write(buf, size);

	//这里进行数据包完整后执行

	while (1)
	{
		CxDChunk* chunk = NULL;
		int nsize = 0;
		if (pto_type == 1)
		{
			chunk = m_input.getChunkEMark("\r\n\r\n", 4);
			if (chunk) nsize = chunk->length() - 4;
		}
		else
		{
			chunk = m_input.getChunkPreUint32();
			if (chunk) nsize = chunk->length();
		}

		if (chunk == NULL) break;

		XLOG_DEBUG("数据包:%d",nsize);

		if (m_delegate && nsize > 0) m_delegate->OnTcpRecv(this, chunk->c_str(), nsize);

		//
		//if (0 != DoCmd(chunk->c_str(), chunk->length())) {
		//}
		//else {
		//	//临时直接发送回给客户端
		//	SendPto(chunk->c_str(), chunk->length());
		//}

		delete chunk;

	}

	return size;
}

bool CxTcpClient::IsOnline()
{
	return m_socket > 0;
}

void CxTcpClient::SetFD(int64 fd)
{
	if (fd == m_socket) return;

	m_socket = fd;

	if (m_delegate) {
		if (IsOnline()) {
			//设置连接时间

			//清除旧的输入缓冲
			m_delegate->OnTcpOpen(this);
		}
		else {
			m_delegate->OnTcpClose(this);
		}
	}

	if (m_socket == -1) {
		m_input.clear();
		//Close();
	}

}

void CxTcpClient::SetDelegate(CxTcpDelegate* _delegate)
{
	m_delegate = _delegate;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

CxMyClient::CxMyClient()
{
	m_delegate = NULL;
	m_proxy = NULL;

	Reset();
	SetDelegate(this);
	handle = NULL;
//	conn = NULL;
//	handle.data = this;
}

CxMyClient::~CxMyClient()
{
	//临时 以下代码崩溃
	//conn_rec* conn = container_of(this, conn_rec,client);
	//free(conn);
}

void CxMyClient::Reset()
{
	m_bSSL = false;
	m_nState = 0;
	m_iPrivilege = 0;
	m_input.clear();

}


//void CxMyClient::shutdown_cb(uv_shutdown_t* req, int status)
//{
//	free(req);
//	//uv_close((uv_handle_t*)req->handle, close_cb);
//}
//
//void CxMyClient::close_cb(uv_handle_t* handle)
//{
//	XLOG_WARN("客户端断开");
//}


int CxMyClient::Open(sockaddr_in _addr)
{

	return 0;
}

void CxMyClient::Close()
{
	//TODO 如果连接状态就关闭掉
	if (IsOnline() /*&& conn*/) {
		//int r = uv_shutdown(&conn->shutdown_req, (uv_stream_t*)handle, CxMyServer::shutdown_cb);
		//XX_ASSERT(r == 0);
		uv_close((uv_handle_t*)handle, CxMyServer::close_cb);
	}
}

void CxMyClient::Accept()
{
	//TODO 发送欢迎信息给客户端
}

int CxMyClient::Send(const char* buf, int size)
{
	if (!IsOnline()) return 0; //异常断开 就不要发送了

	//TODO 根据自己是否进行加密 进行加密而后发送
	Encrypto((char*)buf, size);

	//TODO 这里如果在更换密钥的时候要进行等待

	//return CxTcpClient::Send(buf, size);

	write_req_t * wr = new write_req_t();// (write_req_t*)malloc(sizeof *wr);
	XX_ASSERT(wr != NULL);
	wr->buf = uv_buf_init((char*)buf, size);

	if (uv_write(&wr->req, (uv_stream_t*)handle, &wr->buf, 1, CxMyClient::write_cb))
	{
//		XX_FATAL("uv_write failed");
		XLOG_WARN("客户端主动关闭了");
		//Close();
		SetFD(-1);
	}
	return size;
}

void CxMyClient::SetFD(int64 fd)
{
	CxTcpClient::SetFD(fd);
	if (fd == -1) {
		if (CxMyConfig::proxy_type == 0 && m_proxy)
		{
			CxMyProxy::Instance()->Recycle((CxTcpClientProxy*)m_proxy);
		}
		m_proxy = NULL;
	}
}

void CxMyClient::Decrypto(char* buf, int size)
{
	//这里加解密不增减长度
}

void CxMyClient::Encrypto(char* buf, int size)
{
	//这里加解密不增减长度
}

void CxMyClient::write_cb(uv_write_t* req, int status)
{
	write_req_t* wr = (write_req_t*)req;// container_of(req, write_req_t, req);
	delete wr;
}

void CxMyClient::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{
}

void CxMyClient::OnTcpRecv(CxTcpClient* sender, const char* buf, int size)
{
	
	if (sender == this) 
	{
		//是从客户端那边收来的

		//判断是否进行解密 
		Decrypto((char*)buf, size);

		//执行本地命令
		if (0 != DoCmd(buf, size)) return;

		//向代理连接发送数据 前面根据是否共享来决定是否加内容 这个操作在代理那边实现
		if (m_proxy) m_proxy->SendPto(buf, size);
		else SendPto(buf, size); //返回给客户端
	}
	else {
		//从目标服务器来的数据
		//在代理那边已经把谁的进行剥离了
		Send(buf, size);
	}

}

/**
执行命令
*/
int CxMyClient::DoCmd(const char* buf, unsigned int size)
{
	if (size < 3) return 0;

	if (buf[0] != '@') return 0;
	if (buf[1] != '!') return 0;

	XTokenizer tok(buf,size);
	std::string cmd = tok.GetValueStringByIndex(0);
//	tok.ParseEx2(buf, size, " \r\n");
	XLOG_WARN("cmd:%s", cmd.c_str());
	for (unsigned int i = 0; i < mycmds_count; i++)
	{
//		XLOG_INFO("cmd:%s", mycmds[i].cmd);
		if (0== cmd.compare(mycmds[i].cmd))
		{
			(mycmds[i].proc)(this, buf, size,&tok);
		}
	}
	return 0;
}

/**
向客户端发送新的加密密钥
*/
void CxMyClient::SetCryptoKey(std::string _key)
{
	//等收到客户端新密钥设置成功后才用新的密钥进行通信
	//在密钥握手阶段 发送给客户端的数据都先压入待发送队列中
}


CxMyClientPool::~CxMyClientPool()
{
	for (auto it : fd_clients.container)
	{
		delete it.second;
	}
	fd_clients.container.clear();

	//
	for (auto it : clients.container)
	{
		delete it;
	}
	clients.container.clear();

}

void CxMyClientPool::Init(uint _client_max/*=1024*/,unsigned int _client_start)
{
	for (uint i = _client_start; i < _client_max; i++)
		findClientByFD(i, true);
}

conn_rec* CxMyClientPool::NewConn()
{
	if (clients.container.size() > 0) {
		return clients.safe_pop_front();
	}
	return new conn_rec();
}

void CxMyClientPool::DeleteConn(conn_rec* _conn)
{
	clients.safe_push_back(_conn);
}

void CxMyClientPool::CheckClientOnline()
{
	return;

	time_t _nowSecond = time(NULL);
	XLOG_DEBUG("%d",(int)_nowSecond);
	for (auto it : fd_clients.container)
	{
		
	}
//	if (difftime(_nowSecond - m_last_check_time) > 5

}

void CxMyClientPool::Step()
{
	time_t _nowSecond = time(NULL);
	
	// 1 秒 做一次巡查客户端是否都在线
	if (difftime(_nowSecond , m_last_check_time) >= 1) {
		CheckClientOnline();
		m_last_check_time = _nowSecond;
	}

	//3分钟做一次动态通信加密变换
}

CxMyClient* CxMyClientPool::findClientByFD(int64 fd, bool _create)
{
	return NULL;

	auto it = fd_clients.container.find(fd);
	if (it != fd_clients.container.end()) {
		return it->second;
	}

	if (!_create) return NULL;

	CxMyClient* cli = new CxMyClient();
	//cli->SetFD(fd);
	fd_clients.SafeAppend(fd, cli);
	return cli;
}

void CxMyClientPool::DisconnectAll()
{
	for (auto it : fd_clients.container)
	{
		it.second->Close();
	}
}

