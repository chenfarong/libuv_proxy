#ifndef myCmd_h__
#define myCmd_h__

/*
@!CLIENT ECHO ���ؿͻ��˷��͵�����
@!CLIENT LIST ���ӿͻ����б�
@!CLIENT KICK ǿ�ƶϿ��ͻ���
@!CLIENT NAME �����ӿͻ������� ����Ϊ�˺�����
@!CLIENT LOGIN �ͻ��˵�¼ �û���+����
@!CLIENT LOGIN_EX �û���+�Ự��
@!CLIENT SU Ȩ������

@!CLIENT PROXY �ͻ��˻��µ�Ŀ�� �������û�в��� ��ǰ���������Ŀ�꽫�ر�

*/

#include "myClient.h"
#include <string>
#include "XString/XString.h"

typedef int(*mycmd_proc)(CxMyClient* cli, const char* buf, int size,XTokenizer* tok);

struct sx_cmd_t
{
	const char* cmd;
	mycmd_proc proc;
	int state;
	int privilege;
	std::string help;
};

int mycmd_client_list(CxMyClient* cli, const char* buf, int size, XTokenizer* tok);

/**
�ÿͻ������ӵ�����һ��������
*/
int mycmd_client_proxy(CxMyClient* cli, const char* buf, int size, XTokenizer* tok);

int mycmd_system_helo(CxMyClient* cli, const char* buf, int size, XTokenizer* tok);
int mycmd_system_help(CxMyClient* cli, const char* buf, int size, XTokenizer* tok);

extern struct sx_cmd_t mycmds[];
extern unsigned int mycmds_count;

#endif // myCmd_h__
