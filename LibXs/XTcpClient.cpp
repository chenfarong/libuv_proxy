
#include "XTcpClient.h"


XsTcpClient::XsTcpClient()
{
	m_delegate = NULL;
	m_socket = -1;
	m_pto_type = 1;

}

int XsTcpClient::Connect(const char* _server, int _service)
{
	sk = xnet_connect(_server, _service);
	if (sk != X_INVALID_SOCKET) 
	{
		m_socket = (int64)sk;
		if (m_delegate) m_delegate->OnConnectAfter(this,0);
		m_input.clear();
		return 0;
	}
	return -1;
}

void XsTcpClient::Disconnect()
{
	xnet_disconnect(sk);
	if (m_delegate) m_delegate->OnDisconnect(this, 0);
}

bool XsTcpClient::IsConnected()
{
	return m_socket != -1;
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
	if (!IsConnected()) return;

	memset(m_buffer, 0, 4096);
	if (0 > xnet_step(this, m_socket, XsTcpClient::xnet_recv_proc, m_buffer, 4096))
	{
		Disconnect();
	}
}

int XsTcpClient::Send(const char* _buf, unsigned int _size)
{
	if (!IsConnected()) return -1;
	xnet_send(sk, _buf, _size);
	return _size;
}

void XsTcpClient::Recv(const char* buf, int size)
{
	if (m_delegate) m_delegate->OnRecv(this,buf, size);

	m_input.write(buf, size);
	while (1)
	{
		CxDChunk* chunk = m_input.getChunkEMark("\r\n\r\n", 4);
		if (chunk == NULL) break;
		if (m_delegate) m_delegate->OnRecvPack(this, chunk->c_str(), chunk->length());
		delete chunk;
	}


}
