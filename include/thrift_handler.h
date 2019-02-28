#ifndef _THRIFT_HANDLE_H
#define _THRIFT_HANDLE_H

// ÆÁ±Îµô±àÒë´íÎó
#ifdef WIN32
#pragma warning (disable:4786)
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nvrWebService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::com::nvr::thrift;

class nvrWebServiceHandler : virtual public nvrWebServiceIf 
{
public:
	nvrWebServiceHandler() ;

	int32_t notice(const std::string& cmd, const std::string& jsonObject) ;
};

#endif // 

