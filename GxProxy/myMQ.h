#ifndef myServer_h__
#define myServer_h__

#include "uv.h"
#include "LibXs.h"

class CxMyServer
{
public:
	CxMyServer();
	virtual ~CxMyServer();


public:


protected:
	uv_loop_t*  loop;



};



#endif // myServer_h__
