#ifndef myConfig_h__
#define myConfig_h__

#include "config.h"
#include "Singleton.h"

class CxMyConfig :public XsConfig,public Singleton<CxMyConfig>
{
public:
	CxMyConfig();

};



#endif // myConfig_h__
