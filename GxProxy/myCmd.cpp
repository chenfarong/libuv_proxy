
#include "myCmd.h"


struct sx_cmd_t mycmds[]=
{
	{"client",mycmd_client_list,1,1,"client list"},
};

unsigned int mycmds_count=1;

int mycmd_client_list(CxMyClient* cli, const char* buf, int size)
{
	return 0;
}

