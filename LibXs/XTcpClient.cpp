
#include "XTcpClient.h"


XsTcpClient::XsTcpClient()
{
	m_delegate = NULL;
	m_socket = -1;
	m_pto_type = 1;
	m_connected = false;

	Reset();
}

void XsTcpClient::Reset()
{
	m_send_bytes =
		m_recv_bytes =
		m_send_tick =
		m_recv_tick = 0;
}

int XsTcpClient::Connect(const char* _server, int _service)
{
	sk = xnet_connect(_server, _service, { 6,0 },2,true);
	if (sk != X_INVALID_SOCKET) 
	{
		m_socket = (int64)sk;
		m_input.clear();

		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		m_connected = true;

		if (m_delegate) m_delegate->OnConnectAfter(this, 0);

		return 0;
	}
	return -1;
}

void XsTcpClient::Disconnect()
{
	xnet_disconnect(sk);
	m_connected = false;
	if (m_delegate) m_delegate->OnDisconnect(this, 0);
}

bool XsTcpClient::IsConnected()
{
	return m_connected;
}

void XsTcpClient::SetDelegate(XsTcpClientDelegate* _delegate)
{
	m_delegate = _delegate;
}

void XsTcpClient::SetPtoType(int _type, const char* _emark, unsigned int _size)
{

}

void XsTcpClient::SetOption(int _type, const char* _value)
{

}

void XsTcpClient::xnet_recv_proc(void* sender, xnet_sock s, char * _lpBuf, int _size)
{
	XsTcpClient* _sbr = (XsTcpClient*)sender;
	_sbr->Recv(_lpBuf, _size);
}


void XsTcpClient::Step()
{
	//xnet_step(, sk, );
//	if (!IsConnected()) return;

	memset(m_buffer, 0, 4096);
	if (0 > xnet_step(this, m_socket, XsTcpClient::xnet_recv_proc, m_buffer, 4096))
	{
		Disconnect();
	}
}

int XsTcpClient::Send(const char* _buf, unsigned int _size)
{
	int r = _size;
	if (!IsConnected()) return -1;

	r = xnet_send(sk, _buf, _size);

	m_send_bytes += _size;
	m_send_tick++;

#if(0)
	printf("发数:%llu 发包:%llu 本:%d\n",
		GetSendBytes(),
		GetSendTick(),
		_size);
#endif

	return r;
}

void XsTcpClient::Recv(const char* buf, int size)
{

	if (m_delegate) m_delegate->OnRecv(this,buf, size);

	m_input.write(buf, size);
	while (1)
	{
		CxDChunk* chunk = m_input.getChunkEMark("\r\n\r\n", 4);
		if (chunk == NULL) break;
		
		m_recv_tick++;
#if(0)
		m_recv_bytes += chunk->length();

		printf("收数:%llu,收包:%llu 本:%d\n",
			GetRecvBytes(),
			GetRecvTick(),
			chunk->length());
#endif
		if (m_delegate) m_delegate->OnRecvPack(this, chunk->c_str(), chunk->length());
		delete chunk;
	}

	m_recv_bytes += size;

	//printf("收数:%llu,收包:%llu 本:%d\n",
	//	GetRecvBytes(),
	//	GetRecvTick(),
	//	size);
}
