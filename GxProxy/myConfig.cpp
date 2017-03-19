
#include "myConfig.h"


template<>
CxMyConfig* Singleton<CxMyConfig>::si_instance = NULL;


CxMyConfig::CxMyConfig()
{
	SetAttrib("server", "ip", "0.0.0.0");
	SetAttrib("server", "port", "5454");
	SetAttrib("server", "ssl_enable", "0");
	SetAttrib("server", "ssl_port", "4545");
	SetAttrib("server", "proxy_type", "1"); //默认一个客户端一个代理连接
}

