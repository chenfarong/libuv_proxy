#ifndef myCmd_h__
#define myCmd_h__

/*

!@ CLIENT LIST ���ӿͻ����б�
!@ CLIENT KICK ǿ�ƶϿ��ͻ���
!@ CLIENT NAME �����ӿͻ������� ����Ϊ�˺����� 

!@ CLIENT SU �ͻ��˵�¼ �˺���+�Ự����

!@ CLIENT PROXY �ͻ��˻��µ�Ŀ��

*/
#include "myClient.h"
#include <string>

typedef int(*mycmd_proc)(CxMyClient* cli, const char* buf, int size);

struct sx_cmd_t
{
	const char* cmd;
	mycmd_proc proc;
	int state;
	int privilege;
	std::string help;
};

int mycmd_client_list(CxMyClient* cli, const char* buf, int size);


extern struct sx_cmd_t mycmds[];
extern unsigned int mycmds_count;

#endif // myCmd_h__
