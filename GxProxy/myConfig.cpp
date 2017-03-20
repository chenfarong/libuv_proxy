
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

	SetAttrib("client", "timeout", "30");			//客户端如果30秒没有发送消息上来就主动断开它 如果是0就不管这事
	SetAttrib("client", "net_buf_size", "40960");	//接收数据缓冲区
}


