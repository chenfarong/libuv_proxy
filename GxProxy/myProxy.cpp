
#include "GxProxy.h"
#include "myProxy.h"


X_IMPL_SINSTANCE(CxMyProxy)

void CxTcpDelegateClient::OnTcpSend(CxTcpClient* sender, const char* buf, int size)
{

}

void CxTcpDelegateClient::OnTcpRecv(CxTcpClient* sender, const char* buf, int size)
{
	//TODO 看是否是本地命令
	//TODO 看是否有代理目标 如果有就发送
}

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
