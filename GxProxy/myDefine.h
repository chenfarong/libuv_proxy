#ifndef myDefine_h__
#define myDefine_h__

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

#define X_WIN_MY_SERVICE_INFO 1

#if (X_WIN_MY_SERVICE_INFO>0)
// Internal name of the service
#define SERVICE_NAME  L"GxProxy"
// Displayed name of the service
#define SERVICE_DISPLAY_NAME L"Game Proxy Server"
#define SERVICE_DESCRIPTION  L"XG Network Game Kit Server"
// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""
// The name of the account under which the service should run
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"
// The password to the service account name
#define SERVICE_PASSWORD         NULL
// Service start options.
#define SERVICE_START_TYPE       SERVICE_AUTO_START

#endif

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

#endif // myDefine_h__
