#ifndef xnet_h__
#define xnet_h__


/*


支持阻塞和非阻塞

*/


#include <string>
#include <vector>

#define  X_INVALID_SOCKET 0

#ifndef XULONG
typedef unsigned long XULONG;
#endif

#define XNET_ENDL  "\r\n\r\n\r\n"
#define XNET_ENDL_R "&#13&#10&#13&#10&#13&#10"

#define XNET_AF_INET4 AF_INET
#define XNET_AF_INET6 AF_INET6

#if defined(_MSC_VER)

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <ws2def.h>


typedef SOCKET xnet_sock;

#define __WINDOWS__  1

#else


#include <sys/socket.h>
//#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/ioctl.h> 
#include <net/if.h>
#include <atomic>
#include <type_traits>
#include <utility>
#include <iostream>
#include <list>
#include <sys/select.h>
//#include <sys/epoll.h>
//#include <map>

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

typedef int xnet_sock;

#define INVALID_SOCKET  (int)(~0)
#define SOCKET_ERROR            (-1)

typedef fd_set FD_SET;
//#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)

#endif

typedef void(*xnet_recv_proc)(void* arg,xnet_sock s, char* _lpBuf,int _size);

xnet_sock xnet_connect(std::string ipAddr, int port, timeval _TimeOut = { 6,0 }, int af = XNET_AF_INET4,  bool _noblock = false);
void xnet_disconnect(xnet_sock s);
int  xnet_send(xnet_sock s,const char* lpBuf,int size);

/**
向服务器发送请求 并且阻塞 socket 等待服务器返回内容
@param encode 对收到的数据进行转码
*/
int xnet_request(xnet_sock s, std::string cmdStr, std::vector<char>& buf_cont, bool encode=true);

#define xnet_recv recv

/**
设置是否阻塞
*/
int xnet_noblock(xnet_sock s,bool v);

bool xnet_is_noblock(xnet_sock s); //未测试

int xnet_step(void* arg,xnet_sock s, xnet_recv_proc proc,char* recvBuf,int recvBufSize);

int xnet_init();
void xnet_cleanup();


void myIpAddrList(std::vector<std::string>& result);

int xnet_addr(const char* ip, int port, struct sockaddr_in* addr,int family= AF_INET);
//int xnet_bind();


#endif // xnet_h__
