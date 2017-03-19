
/*

�ͻ���һ�������������ͷ���һ����ӭ��Ϣ�� ��֪�汾�ţ��Ƿ�����ͨ�ż��ܣ���Ʒ��Ϣ

*/

#include "myClient.h"
#include "LibXs.h"
#include "GxProxy.h"

X_IMPL_SINSTANCE(CxMyClientPool)


CxMyClient::CxMyClient()
{
	m_delegate = NULL;

	Reset();
}

void CxMyClient::Reset()
{

	m_bSSL = false;
	m_nState = 0;
	m_iPrivilege = 0;

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

}

void CxMyClient::Encrypto(char* buf, int size)
{

}

void CxMyClient::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{
}

void CxMyClient::OnTcpRecv(CxTcpClient* sender, const char* buf, int size)
{
	
	if (sender == this) {
		//�Ǵӿͻ����Ǳ�������

		//�ж��Ƿ���н��� 
		Decrypto((char*)buf, size);

		//��������ӷ������� ǰ������Ƿ����������Ƿ������ ��������ڴ����Ǳ�ʵ��
		if (m_cur_proxy) m_cur_proxy->Send(buf, size);

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
	pto_type = 0;
}

int CxTcpClient::Send(const char* buf, int size)
{
	return size;
}

int CxTcpClient::Recv(const char* buf, int size)
{
	m_input.write(buf, size);

	//TODO ������Խ����������
	while (1)
	{
		CxDChunk* chunk = NULL;

		if (pto_type == 0)
		{
			chunk = m_input.getChunkEMark("\r\n\r\n", 4);
		}
		else
		{
			chunk = m_input.getChunkPreUint32();
		}

		if(chunk==NULL) break;

		//
		if (0 != DoCmd(chunk->c_str(), chunk->length())) {
			
			//;
		}
		//
		Send(chunk->c_str(), chunk->length());
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

int CxMyClient::DoCmd(const char* buf, unsigned int size)
{
	return 0;
}
