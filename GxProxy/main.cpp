
/*

允许 2种 1=单独连接到目标服务器 2=通过共享连接到目标服务器



*/


#include <iostream>
#ifdef __X86__
#include <vld.h>
#endif
#include "GxProxy.h"

#include "myDefine.h"

#ifdef _MSC_VER
#include "XServer/ServiceWin32.h"
#define __WINDOWS__ 1
#else
#include "XServer/PosixDaemon.h"
#endif

/*
* -1 - not in service mode
*  0 - stopped
*  1 - running
*  2 - paused
*/
int App_ServiceStatus = -1;
std::string myPidFile = "/var/tmp/gproxy_s"; //这里注意有好多个不同的游戏服务器

int proc_configure(int argc, const char** argv)
{

	for (int i = 1; i < argc; i++)
	{
		if (0 == strcmp(argv[i], "-cxml") && argc>i ) {
			CxMyConfig::Instance()->LoadFromXmlFile(argv[i + 1]);
		}
	}

	return 0;
}

/**
为Windows Service 提供入口
*/
int main_server()
{
	CxMyService::OnStart();
	CxMyServer::Instance()->Run();
	CxMyService::OnStop();
	return 0;
}


int main(int argc,const char** argv)
{


//#ifdef _DEBUG
	main_server();
	return 0;
//#endif


#ifdef __WINDOWS__
	ServiceMainProc(argc, argv);
	return 0;
#else

	bool bDaemon = false;
	for (int i = 0; i < argc; i++)
	{
		if (0 == strcmp(argv[i], "-d")) {
			bDaemon = true;
		}

		if (0 == strcmp(argv[i], "-stop")) {
			stopDaemon(myPidFile.c_str());
			//detachDaemon();
			exit(0);
		}

	}


	if (bDaemon) {
		startDaemon(myPidFile.c_str());
	}


	//CreatePIDFile(myPidFile);
	main_server();

#endif


	return 0;
}


