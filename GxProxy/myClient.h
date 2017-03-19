#ifndef myClient_h__
#define myClient_h__

/*

�ͻ�������������ȴ���֤

*/

#include "Singleton.h"
#include "LibXs.h"
#include "uv.h"
#include "SafeContainer.h"
#include "XNetBuffer.h"

class CxTcpClient;
class CxMyClient;

typedef struct {
	uv_tcp_t handle;
	uv_shutdown_t shutdown_req;
	CxMyClient* client;
} conn_rec;

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
	CxMyClient* client;
} write_req_t;



class CxTcpRecvDelegate
{
public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size) {  };
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size) {  };
	virtual void OnTcpClose(CxTcpClient* sender) {};
	virtual void OnTcpOpen(CxTcpClient* sender) {};
};

class CxTcpClient
{
public:
	CxTcpClient();

	int Open(sockaddr_in _addr);
	void Close();

	virtual int Send(const char* buf, int size);
	int SendPto(const char* buf, int size);

	virtual int Recv(const char* buf, int size);

	bool IsOnline();
	virtual void SetFD(int64 fd);

	virtual int DoCmd(const char* buf, unsigned int size) { return 0; };

protected:
	CxTcpRecvDelegate* m_delegate;
	int64 m_socket;
	CxNetBuffer m_input;

	int pto_type;

public:

	std::string m_name;
	struct sockaddr_in peer;

	int64 m_tiBirth; //����������ʱ��
	int64 m_tiBreak; //���������ʱ�� ����������жϿͻ����Ƿ�����

//	char  recvBuffer[65535]; //��������������ʱ�򱣴���
private:
	
};


class CxMyClient : public CxTcpClient,public CxTcpRecvDelegate
{
public:
	enum CLI_STATE
	{
		CTT_NEW,
		CTT_TLS,	//��������
		CTT_AUTH,	//�Ѿ�ͨ����֤��
	};

public:
	CxMyClient();
	void Reset();

	void Accept();

	virtual int Send(const char* buf, int size);

	void OnRecvFromClient(const char* buf, int size);

	/**
	���Ӵ���һ��Ŀ��
	*/
	void SetProxyTarget(sockaddr_in _addr);

	void Decrypto(char* buf, int size);
	void Encrypto(char* buf, int size);

public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size);
	
	virtual int DoCmd(const char* buf, unsigned int size);

protected:

	

	int m_nState;

	sockaddr_in m_addr;

	bool m_bSSL;
	std::string m_sCryptoKey;



	CxTcpClient* m_cur_proxy;	//���ĸ���������

	int m_iPrivilege;  //Ȩ��

public:
	uv_tcp_t* handle;

};

#pragma mark - 

class CxMyClientPool : public Singleton<CxMyClientPool>
{
public:
	void Init(uint _client_num);


public:
	//CxSafeVector<CxMyClient*>	clients; //�����޸�

	CxSafeMap<int64, CxMyClient*> fd_clients;

	void CheckClientOnline();

	void Step();

	CxMyClient* findClientByFD(int64 fd,bool _create);



};


#endif // myClient_h__
