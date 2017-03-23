
#include "myCmd.h"
#include "GxProxy.h"
#include "myProxy.h"
#include "uv.h"
#include "xnet.h"
#include "myConfig.h"
#include "myServer.h"

static const char* X_APP_VERSION = "GXPROXY 2017-03-19";

struct sx_cmd_t mycmds[]=
{
	{"client",mycmd_client_list,1,1,"client list"},
	{ "@!PROXY",mycmd_client_proxy,1,1,"PROXY <IP> <PORT>" },
	{ "@!VER",mycmd_system_helo,0,0,"VER" },
	{ "@!SHUTDOWN",mycmd_system_shoutdown,0,0,"SYSTEM_SHUTDOWN" },
	
};

unsigned int mycmds_count=4;

int mycmd_client_list(CxMyClient* cli, const char* buf, int size, XTokenizer* tok)
{
	return 0;
}


/**

@!PROXY 127.0.0.1 7700

*/
int mycmd_client_proxy(CxMyClient* cli, const char* buf, int size, XTokenizer* tok)
{
	//取得目标服务器地址 和端口 然后连接
	//return 0;

	if (tok->NumLines() < 2)
	{
		//cli->SendPto();
		return 1;
	}


	sockaddr_in _addr;
	std::string _server = tok->GetValueStringByIndex(1);

	//关闭
	if (_server.compare("CLOSE") == 0)
	{
		if (cli->m_proxy) {
			//TODO 如果是 1 对 1 的就关闭连接
			if (CxMyConfig::proxy_type == 0)
			{
				CxMyProxy::Instance()->Recycle((CxTcpClientProxy*)cli->m_proxy);
			}

			cli->m_proxy = NULL;
		}
		return 1;
	}

	int _service = tok->GetValueIntByIndex(2);

	xnet_addr(_server.c_str(), _service, &_addr);

	CxTcpClientProxy* _tcp =(CxTcpClientProxy*) CxMyProxy::Instance()->findWitchConnect(_addr);
	XX_ASSERT(_tcp);
	_tcp->cli = cli;
//	cli->m_proxy = _tcp; //这个要等连接后才设置

	return 1;
}

int mycmd_system_helo(CxMyClient* cli, const char* buf, int size, XTokenizer* tok)
{
	cli->SendPto(X_APP_VERSION,(int)strlen(X_APP_VERSION));
	return 1;
}

int mycmd_system_help(CxMyClient* cli, const char* buf, int size, XTokenizer* tok)
{
	return 1;
}

int mycmd_system_shoutdown(CxMyClient* cli, const char* buf, int size, XTokenizer* tok)
{
	cli->Close();
	CxMyService::Shutdown();
	return 1;
}

