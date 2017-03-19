
/*

�ͻ���һ�������������ͷ���һ����ӭ��Ϣ�� ��֪�汾�ţ��Ƿ�����ͨ�ż��ܣ���Ʒ��Ϣ

*/

#include "myClient.h"
#include "LibXs.h"
#include "GxProxy.h"
#include "XNetBuffer.h"
#include "XString/XString.h"

X_IMPL_SINSTANCE(CxMyClientPool)


CxMyClient::CxMyClient()
{
//	m_delegate = NULL;
	Reset();
	SetDelegate(this);
}

void CxMyClient::Reset()
{

	m_bSSL = false;
	m_nState = 0;
	m_iPrivilege = 0;
	m_proxy = NULL;
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


int CxMyClient::Open(sockaddr_in _addr)
{
	uv_connect_t* connect_req=(uv_connect_t*)malloc(sizeof(uv_connect_t));
	//uv_tcp_connect()
	sock = create_tcp_socket();

	int r = uv_tcp_init(uv_default_loop(), &client);
	XX_ASSERT(r == 0);

	r = uv_tcp_open(&client, sock);
	XX_ASSERT(r == 0);

	r = uv_tcp_connect(connect_req,
		&client,
		(const struct sockaddr*) &_addr,
		CxMyClient::connect_cb);
	XX_ASSERT(r == 0);
	return 0;
}

void CxMyClient::Close()
{

}

void CxMyClient::Accept()
{
	//TODO ���ͻ�ӭ��Ϣ���ͻ���
}

int CxMyClient::Send(const char* buf, int size)
{

	//TODO �����Լ��Ƿ���м��� ���м��ܶ�����
	Encrypto((char*)buf, size);

	//TODO ��������ڸ�����Կ��ʱ��Ҫ���еȴ�

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
	//����ӽ��ܲ���������
}

void CxMyClient::Encrypto(char* buf, int size)
{
	//����ӽ��ܲ���������
}

void CxMyClient::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{
}

void CxMyClient::OnTcpRecv(CxTcpClient* sender, const char* buf, int size)
{
	
	if (sender == this) 
	{
		//�Ǵӿͻ����Ǳ�������

		//�ж��Ƿ���н��� 
		Decrypto((char*)buf, size);

		//ִ�б�������
		if (0 != DoCmd(buf, size)) return;

		//��������ӷ������� ǰ������Ƿ����������Ƿ������ ��������ڴ����Ǳ�ʵ��
		if (m_proxy) m_proxy->SendPto(buf, size);

	}
	else {
		//��Ŀ���������������
		//TODO �ڴ����Ǳ��Ѿ���˭�Ľ��а�����
		Send(buf, size);
	}

}

void CxMyClientPool::CheckClientOnline()
{

}

void CxMyClientPool::Step()
{
	//��ʱ���ȥ�˶��� ���5�� ��һ��Ѳ��ͻ����Ƿ�����
	CheckClientOnline();

	//3������һ�ζ�̬ͨ�ż��ܱ任
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
	m_input.write(buf, size);


	//TODO ������Խ����������
	while (1)
	{
		CxDChunk* chunk = NULL;

		if (pto_type == 1)
		{
			chunk = m_input.getChunkEMark("\r\n\r\n", 4);
		}
		else
		{
			chunk = m_input.getChunkPreUint32();
		}

		if(chunk==NULL) break;

		if (m_delegate) m_delegate->OnTcpRecv(this, chunk->c_str(), chunk->length());

		//
		//if (0 != DoCmd(chunk->c_str(), chunk->length())) {
		//	
		//	//;
		//}
		//else {
		//	//��ʱֱ�ӷ��ͻظ��ͻ���
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
			//��������ʱ��

			//����ɵ����뻺��
			m_input.clear();

			m_delegate->OnTcpOpen(this);
		}
		else m_delegate->OnTcpClose(this);
	}
}

void CxTcpClient::SetDelegate(CxTcpDelegate* _delegate)
{
	m_delegate = _delegate;
}

int CxMyClient::DoCmd(const char* buf, unsigned int size)
{
	XTokenizer tok;
	tok.ParseEx2(buf, size, " \r\n");

	for (unsigned int i = 0; i < mycmds_count; i++)
	{
		//const char* _str_first; //FIXME
		if (tok.FirstTokenIs(mycmds[i].cmd))
		{
			(mycmds[i].proc)(this, buf, size,&tok);
		}
	}
	return 0;
}

void CxMyClient::connect_cb(uv_connect_t* req, int status)
{

}

void CxMyClient::write_cb(uv_write_t* req, int status)
{

}

void CxMyClient::alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{

}

void CxMyClient::read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{

}

void CxMyClient::shutdown_cb(uv_shutdown_t* req, int status)
{

}

void CxMyClient::close_cb(uv_handle_t* handle)
{

}
