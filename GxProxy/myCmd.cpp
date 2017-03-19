
#include "myCmd.h"
#include "GxProxy.h"
#include "myProxy.h"
#include "uv.h"
#include "xnet.h"

static const char* X_APP_VERSION = "GXPROXY 2017-03-19";

struct sx_cmd_t mycmds[]=
{
	{"client",mycmd_client_list,1,1,"client list"},
	{ "@!PROXY",mycmd_client_proxy,1,1,"PROXY <IP> <PORT>" },
	{ "@!VER",mycmd_system_helo,0,0,"VER" },
};

unsigned int mycmds_count=3;

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
	
	sockaddr_in _addr;
	std::string _server = tok->GetValueStringByIndex(1);
	int _service = tok->GetValueIntByIndex(2);

	xnet_addr(_server.c_str(), _service, &_addr);

	CxTcpClient* _tcp = CxMyProxy::Instance()->findWitchConnect(_addr);
	XX_ASSERT(_tcp);
	cli->m_proxy = _tcp;

	return 1;
}

int mycmd_system_helo(CxMyClient* cli, const char* buf, int size, XTokenizer* tok)
{
	cli->SendPto(X_APP_VERSION,(int)strlen(X_APP_VERSION));
	return 1;
}

