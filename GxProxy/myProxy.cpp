
#include "GxProxy.h"
#include "myProxy.h"


X_IMPL_SINSTANCE(CxMyProxy)

void CxTcpDelegateClient::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{

}

void CxTcpDelegateClient::OnTcpRecv(CxTcpClient* sender, const char* buf, int size)
{
	//TODO ���Ƿ��Ǳ�������
	//TODO ���Ƿ��д���Ŀ�� ����оͷ���
}

CxMyProxy::CxMyProxy()
{
	proxy_type = 0;
}

void CxMyProxy::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{
	//TODO �������� �����Ƿ�Ҫ����ǰ�ߵĿͻ��˱��

}

void CxMyProxy::OnTcpRecv(CxTcpClient* sender, const char* buf, int size)
{
	//�������� �����Ƿ�Ҫ����ǰ�ߵĿͻ��˱��
	if (proxy_type == 0) {
		CxTcpClientProxy* _tcp = (CxTcpClientProxy*)sender;
		if (_tcp->client) _tcp->client->SendPto(buf, size);
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
	client = NULL;
}
