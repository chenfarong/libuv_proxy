
/*

���� 2�� 1=�������ӵ�Ŀ������� 2=ͨ���������ӵ�Ŀ�������



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


