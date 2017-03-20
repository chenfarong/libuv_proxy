#ifndef myConfig_h__
#define myConfig_h__

#include "config.h"
#include "Singleton.h"

class CxMyConfig :public XsConfig,public Singleton<CxMyConfig>
{
public:
	CxMyConfig();


public:
	static int proxy_type;
	static bool enable_ssl;

};

extern CxMyConfig theConfig;

#endif // myConfig_h__
