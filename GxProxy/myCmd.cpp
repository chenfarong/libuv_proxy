
#include "myCmd.h"

static const char* X_APP_VERSION = "GXPROXY 2017-03-19";

struct sx_cmd_t mycmds[]=
{
	{"client",mycmd_client_list,1,1,"client list"},
	{ "@!VER",mycmd_system_helo,0,0,"VER" },
};

unsigned int mycmds_count=2;

int mycmd_client_list(CxMyClient* cli, const char* buf, int size)
{
	return 0;
}

int mycmd_client_proxy(CxMyClient* cli, const char* buf, int size)
{

	return 1;
}

int mycmd_system_helo(CxMyClient* cli, const char* buf, int size)
{
	cli->SendPto(X_APP_VERSION,(int)strlen(X_APP_VERSION));
	return 1;
}

