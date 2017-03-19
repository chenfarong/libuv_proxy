
/*

允许 2种 1=单独连接到目标服务器 2=通过共享连接到目标服务器



*/


#include <iostream>

#include "GxProxy.h"


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



int main(int argc,const char** argv)
{

	//std::cout << "Hello World" << std::endl;

	CxMyServer::Instance()->Listen("0.0.0.0", 8000);

	CxMyServer::Instance()->Run();

	return 0;
}


