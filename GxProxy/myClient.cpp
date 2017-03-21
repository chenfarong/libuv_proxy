
/*

�ͻ���һ�������������ͷ���һ����ӭ��Ϣ�� ��֪�汾�ţ��Ƿ�����ͨ�ż��ܣ���Ʒ��Ϣ

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
		//�ڴ����Ǳ��Ѿ���˭�Ľ��а�����
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
	
	// 1 �� ��һ��Ѳ��ͻ����Ƿ�����
	if (difftime(_nowSecond , m_last_check_time) >= 1) {
		CheckClientOnline();
		m_last_check_time = _nowSecond;
	}

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

	m_tiBreak = time(NULL);

	m_input.write(buf, size);

	//����������ݰ�������ִ��

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
ִ������
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
��ͻ��˷����µļ�����Կ
*/
void CxMyClient::SetCryptoKey(std::string _key)
{
	//���յ��ͻ�������Կ���óɹ�������µ���Կ����ͨ��
	//����Կ���ֽ׶� ���͸��ͻ��˵����ݶ���ѹ������Ͷ�����
}

