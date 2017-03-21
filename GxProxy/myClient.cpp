
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

X_IMPL_SINSTANCE(CxMyClientPool)


CxMyClient::CxMyClient()
{
	m_delegate = NULL;
	m_proxy = NULL;

	Reset();
	SetDelegate(this);
}

void CxMyClient::Reset()
{

	m_bSSL = false;
	m_nState = 0;
	m_iPrivilege = 0;
	m_input.clear();
//	m_proxy = NULL;

}




int CxMyClient::Open(sockaddr_in _addr)
{

	return 0;
}

void CxMyClient::Close()
{
	Reset();

	if (CxMyConfig::proxy_type == 0 && m_proxy)
	{
		//m_proxy->Close();
		CxMyProxy::Instance()->Recycle((CxTcpClientProxy*)m_proxy);
	}

}

void CxMyClient::Accept()
{
	//TODO 发送欢迎信息给客户端
}

int CxMyClient::Send(const char* buf, int size)
{

	//TODO 根据自己是否进行加密 进行加密而后发送
	Encrypto((char*)buf, size);

	//TODO 这里如果在更换密钥的时候要进行等待

	//return CxTcpClient::Send(buf, size);

	write_req_t * wr = (write_req_t*)malloc(sizeof *wr);
	XX_ASSERT(wr != NULL);
	wr->buf = uv_buf_init((char*)buf, size);

	if (uv_write(&wr->req, (uv_stream_t*)handle, &wr->buf, 1, 0)) 
	{
		XX_FATAL("uv_write failed");
	}
	return size;
}

void CxMyClient::Decrypto(char* buf, int size)
{
	//这里加解密不增减长度
}

void CxMyClient::Encrypto(char* buf, int size)
{
	//这里加解密不增减长度
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

	}
	else {
		//从目标服务器来的数据
		//在代理那边已经把谁的进行剥离了
		Send(buf, size);
	}

}

CxMyClientPool::~CxMyClientPool()
{
	for (auto it : fd_clients.container)
	{
		delete it.second;
	}
	fd_clients.container.clear();
}

void CxMyClientPool::Init(uint _client_max/*=1024*/,unsigned int _client_start)
{
	for (uint i = _client_start; i < _client_max; i++)
		findClientByFD(i, true);
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
	auto it = fd_clients.container.find(fd);
	if (it != fd_clients.container.end()) return it->second;

	if (!_create) return NULL;

	CxMyClient* cli = new CxMyClient();
	cli->SetFD(fd);
	fd_clients.SafeAppend(fd, cli);
	return cli;
}

CxTcpClient::CxTcpClient()
{
	m_socket = -1;
	pto_type = 1;
	m_delegate = NULL;
}

int CxTcpClient::Open(sockaddr_in _addr)
{
	return 0;
}

void CxTcpClient::Close()
{

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
		buffer.WritefixEMark(buf, size, "\r\n\r\n", 4);
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
			if(chunk) nsize = chunk->length() - 4;
		}
		else
		{
			chunk = m_input.getChunkPreUint32();
			if (chunk) nsize = chunk->length();
		}

		if(chunk==NULL) break;

		if (m_delegate && nsize>0) m_delegate->OnTcpRecv(this, chunk->c_str(), nsize);

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
	return m_socket >0;
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
		Close();
	}

}

void CxTcpClient::SetDelegate(CxTcpDelegate* _delegate)
{
	m_delegate = _delegate;
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

